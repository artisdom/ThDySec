/**
* Copyright (C) 2009-2016, Ariel Vina-Rodriguez ( ariel.rodriguez@fli.bund.de , arielvina@yahoo.es )
*  https://www.fli.de/en/institutes/institut-fuer-neue-und-neuartige-tierseuchenerreger/wissenschaftlerinnen/prof-dr-m-h-groschup/
*  distributed under the GNU General Public License, see <http://www.gnu.org/licenses/>.
*
* @author Ariel Vina-Rodriguez (qPCR4vir)
* 2012-2016
*
* @file  ThDySec\include\ThDySec\sec.h
*
* @brief To manipulate DNA sequences for simple thermodynamic modeling of hybridization.
*
* Classes to manipulate DNA sequences. Adapted exclusively to DNA and specifically for thermodynamic calculations.
*
*
*/

#pragma unmanaged    

#ifndef _SEC_H
#define _SEC_H

#include <stdlib.h>
#include <fstream>
#include <cassert>
#include <string>
#include <memory>
#include <vector>
#include <filesystem>
namespace filesystem = std::experimental::filesystem;  


#include "sec_basic.h" 
#include "th_dy_param.h"   
#include "common.h" 
 
///\todo crear (adaptar) clase primer derivada de CSec, con pos y Tm en cada sec Target
 
class CMultSec    ;

                   // ---------------------------------------   CSec    ---------------------------------------------------

/// To manipulate DNA sequences (secuencias) for simple thermodynamic modeling of hybridization.

/// Fundamental class to manipulate DNA sequences. Adapted exclusively to DNA and specifically for thermodynamic calculations.
/// Have the sequence in "letter" or nt format, AND in "code" format to avoid millions of repeated conversions. 
/// Remember most characteristics for fast retrieval: length, CG%, degeneracy, etc.
/// Importantly have a sequence of the values of dH and dS from the beginning to each position,
/// which make trivial and fast to find accumulated dH and dS and Tm of any fragment by a simple difference calculation.
/// When degenerated it can have a NonDegSet with all the CSec with degeneracy 1 - each of the variants in the original
/// It track some group of sequences (a "parent" CMultSec) to which it belongs.
/// It can be filtred and can be selected. Normally only selected sequences are used in calculations,
/// and only if the parent group is also selected.
///
/// Some variables have index base [1] while others have [0] in sec. 
///
///      sec_begin                                                                             
///      0                                                                                     sec_end
///      |    1        exp_begin                        exp_end                                |       ---> sq
///      |    |        |                                |                                      |   
///     Z-----ATCGCGTAGCTAGCTAGCTAGCTGACTTGTCTGGTAGCT--GCTATCTAATGCTGATGCTAGTCGATCGTAGCTGC-x----ZX x?
///      |             |                                |
///      1             orig_beging                      orig_end                                       ---> aln
///                    |                                |
///                    1 -fltr_beging                   fltr_end - not counting internal gaps
///     
/// set sq.sq to some original "experimental" CSec if any
///
/// set aln.sq to the parent CMultSec*
class CSec : public CSecBasInfo    
{public:
        int                           x;            ///<  ????
        TemperatureRang               _Tm ;         // float        _Tm, _minTm, _maxTm ;            
        std::shared_ptr<CSaltCorrNN>  _NNpar ;
        float                   _Conc ;            ///< concentration of this molecule. If equal to all others is set to -1 and NNParam is used instead
        std::vector<Code>       _b=   std::vector<Code>   {n_basek-1};            ///< codified sequence, initially? basek
        std::vector<Entropy>    _SdS= std::vector<Entropy>{ _NNpar->GetInitialEntropy()};        ///< dS acumulada. Calcular Delta S sera solo restar la final menos la inicial    
        std::vector<Energy>     _SdH= std::vector<Energy> {  Energy{} };          // 
        CMultSec               *_parentMS{nullptr}    ;   //std::weak_ptr<CMultSec> _parentMS    ;

    /// Some variables have index base [1] while others have [0] in sec. We need to clean the sec, which can contain non bases letter, like tabs, end of line, blancs, etc, but we take into account "-".
    CSec (  const std::string&  sec, 
            int                 id,
            const std::string&  nam,     
            std::shared_ptr<CSaltCorrNN>  NNpar, 
            LonSecPos           lmax=0,    ///< limita la cant de bases originales a leer despues de las primeras secBeg-1 bases 
            LonSecPos           secBeg=1,  ///< base [1] in sec. The first letter in sec to be read. 
            const std::string&  clas="" , 
            float               conc=-1         );

    /// Dummy sequence without actual sequence, but with NNpar and reserved space
    CSec ( long l, std::shared_ptr<CSaltCorrNN>  NNpar) ;


    //long        Len            ()const        {return _SdS.size();} //
    void         CorrectSaltOwczarzy    () ;
    CMultSec    *CreateNonDegSet        () ;              ///< crea todo el set si no existia, solo si existen bases deg: _NDB>0
    CMultSec    *ForceNonDegSet         ();               ///< lo crea siempre, incluso para =1??
    CSec        *GenerateNonDegVariant  (CSec *s, long pos, Base ndb)   ; ///< recursiva
    CSec        *CopyFirstBases         (long pos)   ;    ///< copia parcialmente hasta la pos
    void         CorrectSalt            () { if ( _NNpar->UseOwczarzy () ) CorrectSaltOwczarzy();};
    CSec *Clone(DNAstrand strnd=DNAstrand::direct     ) const override; /// unique_ptr<ISec> crea una copia muy simple. \todo CUIDADO con copias de CSecBLASTHit y otros derivados
    CSec *Clone( long  InicBase,
                 long  EndBase, 
                 DNAstrand   strnd = DNAstrand::direct) const override;

    //virtual CSec*CreateCopy        (DNAstrand strnd=direct) override;//< crea una copia muy simple. CUIDADO con copias de CSecBLASTHit y otros derivados
    //const char    *Get_charSec            ()const{return (const char*)_c.c_str();}  ///   ???????????

    bool        Selected() const;                 //< User-editable    ???????????????????????????????????????????????????????????????????????????
    bool        Selected(bool select)    
                          { 
                              _selected = select; 
                              return Selected(); 
                           }             //< make protected: ?????????????????????????????????????????????????
    Base        operator()        (int i)const{return _b[i];}
    Temperature  Tm    (long pi, long pf   )const;                               ///< Tm de la sonda con sec desde pi hasta pf, inclusive ambas!! 
    Temperature  Tm    (long pi            )const    {return Tm(pi,Len())   ;}   ///< Tm de la sonda con sec desde pi hasta el final, inclusive ambos!!
    Energy        G    (long pi, long pf, float Ta)const;                        ///< G de la sonda con sec desde pi hasta pf, inclusive ambas!! 
    Energy        G    (long pi, float Ta  )const    {return G(pi,Len(), Ta);}   ///< G de la sonda con sec desde pi hasta el final, inclusive ambos!!
    Energy        G    (float Ta           )const    {return G(1 ,Len(), Ta);}   ///< G de la sonda con sec desde inicio hasta el final, inclusive ambos!!
    Energy        G    (long pi, long pf   )const;                               ///< G de la sonda con sec desde pi hasta pf, inclusive ambas!! 
    Energy        G    (long pi            )const    {return G(pi,Len())    ;}   ///< G de la sonda con sec desde pi hasta el final, inclusive ambos!!
    Energy        G    (                   )const    {return G(1,Len())     ;}   ///< G de la sonda con sec desde inicio hasta el final, inclusive ambos!!

     ~CSec() override  ;    
    virtual bool NotIdem(CSec *sec) {return false;}
};

      //<Hsp_num>1</Hsp_num>
      //<Hsp_bit-score>482.786</Hsp_bit-score>
      //<Hsp_score>534</Hsp_score>
      //<Hsp_evalue>3.71782e-133</Hsp_evalue>
      //<Hsp_query-from>1</Hsp_query-from>
      //<Hsp_query-to>267</Hsp_query-to>
      //<Hsp_hit-from>9043</Hsp_hit-from>
      //<Hsp_hit-to>9309</Hsp_hit-to>
      //<Hsp_query-frame>1</>
      //<Hsp_hit-frame>1</>
      //<Hsp_identity>267</>
      //<Hsp_positive>267</>
      //<Hsp_gaps>0</>
      //<Hsp_align-len>267</>
      //<Hsp_qseq>TACAACATGATGGGAAAGAGAGAGAAGAAGCCTGGAGAGTTCGGCAAGGCTAAAGGCAGCAGAGCCATCTGGTTCATGTGGCTGGGGGCTCGCTTCCTGGAGTTTGAAGCTCTCGGATTCCTCAATGAAGACCACTGGCTGGGTAGGAAGAACTCAGGAGGAGGAGTAGAAGGCTTAGGACTGCAGAAGCTTGGGTACATCTTGAAGGAAGTTGGGACAAAGCCTGGAGGAAAGATTTACGCTGATGATACCGCAGGCTGGGACACA</Hsp_qseq>
      //<Hsp_hseq>TACAACATGATGGGAAAGAGAGAGAAGAAGCCTGGAGAGTTCGGCAAGGCTAAAGGCAGCAGAGCCATCTGGTTCATGTGGCTGGGGGCTCGCTTCCTGGAGTTTGAAGCTCTCGGATTCCTCAATGAAGACCACTGGCTGGGTAGGAAGAACTCAGGAGGAGGAGTAGAAGGCTTAGGACTGCAGAAGCTTGGGTACATCTTGAAGGAAGTTGGGACAAAGCCTGGAGGAAAGATTTACGCTGATGATACCGCAGGCTGGGACACA</Hsp_hseq>
      //<>

class CSecBLASTHit : public CSec // ---------------------------------------   CSecBLASTHit    ------------------------------------------------
{public:
    CSecBLASTHit(    unsigned int    BlastOutput_query_len ,
                    // para cada hit
                    unsigned int    Hit_num ,
                    std::string&&   Hit_id ,                
                    std::string&&   Hit_def ,                
                    std::string&&   Hit_accession    ,
                    long            Hit_len ,                
                    float            Hsp_bit_score ,
                    unsigned int    Hsp_score ,
                    double            Hsp_evalue ,
                    unsigned int    Hsp_query_from ,
                    unsigned int    Hsp_query_to ,
                    unsigned int    Hsp_hit_from ,
                    unsigned int    Hsp_hit_to ,
                    unsigned int    Hsp_query_frame ,
                    unsigned int    Hsp_hit_frame ,
                    unsigned int    Hsp_identity ,
                    unsigned int    Hsp_positive ,
                    unsigned int    Hsp_gaps ,
                    unsigned int    Hsp_align_len ,
                    std::string&&   Hsp_midline ,
                    bool            FormatOK ,
                    std::string&&   sec    ,    
                    LonSecPos       lmax,                            //long    SecBeg,long    SecEnd,
                    LonSecPos       secBeg,                          //long    SecBeg,long    SecEnd,
                    int                id,                           //    Hit_num    char    *    nam,    Hit_def
                    std::shared_ptr<CSaltCorrNN>  NNpar,             //    long  l=0,    Hit_len ------> _Hsp_align_len
                    std::string        clas="", 
                    float            conc=-1
                )  :
                        CSec (  std::move(sec),             // sec
                                id,                         // 
                                std::move(Hit_accession),   // name 
                                NNpar,                      //  . maxlen . secBeg .
                                lmax, 
                                secBeg, 
                                clas,
                                conc ),
                            _BlastOutput_query_len( BlastOutput_query_len ) ,
                            // para cada hit
                            _Hit_num        ( Hit_num ) ,
                            _Hit_id         ( std::move(Hit_id) ) ,                
                            _Hit_def        ( std::move(Hit_def) ) ,                
                            _Hit_accession  ( std::move(Hit_accession) )    ,
                            _Hit_len        ( Hit_len ) ,                
                            _Hsp_bit_score  ( Hsp_bit_score ) ,
                            _Hsp_score      ( Hsp_score ) ,
                            _Hsp_evalue     ( Hsp_evalue ) ,
                            _Hsp_query_from ( Hsp_query_from ) ,
                            _Hsp_query_to   ( Hsp_query_to ) ,
                            _Hsp_hit_from   ( Hsp_hit_from ) ,
                            _Hsp_hit_to     ( Hsp_hit_to ) ,
                            _Hsp_query_frame( Hsp_query_frame ) ,
                            _Hsp_hit_frame  ( Hsp_hit_frame ) ,
                            _Hsp_identity   ( Hsp_identity ) ,
                            _Hsp_positive   ( Hsp_positive ) ,
                            _Hsp_gaps       ( Hsp_gaps ) ,
                            _Hsp_align_len  ( Hsp_align_len ) ,
                            _Hsp_midline    ( std::move(Hsp_midline) ) ,
                            _FormatOK       ( FormatOK ) /*,
                            _SecLim         ( SecLim )    *//*,_SecBeg    (SecBeg),     _SecEnd        (SecEnd)*/
                            {
                        }            


    unsigned int    _BlastOutput_query_len ;
    // para cada hit
    unsigned int    _Hit_num ;
    std::string     _Hit_id ;                
    std::string     _Hit_def ;                
    std::string     _Hit_accession    ;
    long            _Hit_len ;                
    float           _Hsp_bit_score ;
    unsigned int    _Hsp_score ;
    double          _Hsp_evalue ;
    unsigned int    _Hsp_query_from ;
    unsigned int    _Hsp_query_to ;
    unsigned int    _Hsp_hit_from ;
    unsigned int    _Hsp_hit_to ;
    unsigned int    _Hsp_query_frame ;
    unsigned int    _Hsp_hit_frame ;
    unsigned int    _Hsp_identity ;
    unsigned int    _Hsp_positive ;
    unsigned int    _Hsp_gaps ;
    unsigned int    _Hsp_align_len ;
    std::string     _Hsp_midline ;
    bool            _FormatOK ;
    //NumRang<long>    _SecLim;                                     //long    _SecBeg;    //long    _SecEnd;
    std::string    Description ()const    override {return _description.length() ? _description : _Hit_def ; }
};

class CSecGB : public CSec // ---------------------------------------   CSecGB    ------------------------------------------------
{public:
    std::string     _Textseq_id_accession ;    
    std::string     _Org_ref_taxname      ;
    std::string     _Seqdesc_title        ;
    long            _Seq_inst_length      ;        
    
    CSecGB(         std::string     Textseq_id_accession ,    
                    std::string     Org_ref_taxname     ,
                    std::string     Seqdesc_title    ,
                    long            Seq_inst_length     ,    
                    const char    * sec    ,    
                    int             id,                        //    char        *    nam,    Seqdesc_title    ,    
                    std::shared_ptr<CSaltCorrNN>  NNpar,       //    long            l=0,        Seq_inst_length
                    std::string     clas="", 
                    float           conc=-1
                ):
                CSec (sec, id, Textseq_id_accession, NNpar, Seq_inst_length,1, clas, conc ),// actualizar Beg-End
                _Textseq_id_accession   ( std::move(Textseq_id_accession) ) ,
                _Org_ref_taxname        ( std::move(Org_ref_taxname )) ,
                _Seqdesc_title          ( std::move(Seqdesc_title) ) ,                
                _Seq_inst_length        ( Seq_inst_length )                 {}            
    virtual std::string    Description ()const override    {return _description.length() ? _description : _Seqdesc_title ; }

    virtual ~CSecGB(){    }
};

class CSecGBtxt : public CSec // ---------------------------------------   CSecGBtxt    ------------------------------------------------
{public:
    std::string    _LOCUS         ;
    long         _Seq_inst_length ;    
    std::string  _DEFINITION      ;
    std::string  _ACCESSION       ;
    std::string  _ORGANISM        ;            // ORIGIN      
    CSecGBtxt(      std::string        LOCUS          ,
                    long               Seq_inst_length,    
                    std::string        DEFINITION     ,
                    std::string        ACCESSION      ,
                    std::string        ORGANISM       ,
                    const char    *    sec    ,    
                    int                id,                        //    char        *    nam,    DEFINITION    ,    
                    std::shared_ptr<CSaltCorrNN>  NNpar,                  //    long            l=0,        Seq_inst_length
                    std::string        clas="", 
                    float              conc=-1
                ):        CSec (sec, id, LOCUS, NNpar, 0,1, clas, conc ), // actualizar Beg-End
                            _LOCUS          ( std::move(LOCUS )) ,
                            _Seq_inst_length( Seq_inst_length ) ,
                            _DEFINITION     ( std::move(DEFINITION) ) ,                
                            _ACCESSION      ( std::move(ACCESSION) )     ,            
                            _ORGANISM       ( std::move(ORGANISM ))                 {}    
    virtual std::string    Description ()const override    {return _description.length() ? _description : _DEFINITION ; }
    
    virtual ~CSecGBtxt() {                }    
};

/// ahora estos seran solo para mantener "copias" (punteros) de sec o msec en otras listas. 
/// Cada sec o msec sabe en que lista esta
//class CSecLink     // NO es dueno de la sec, no la borra, no delete
//{    public:
//        CSecLink (CSec *s, CSecLink *p, CSecLink *n=nullptr) : _sec(s) {} ;
//        CSec *_sec ;
//};
        
#endif


//class CConsParam 
//{    public:
//        double _min ;
//};
//enum DNAstrand                        {plus    , minus, direct    , rev    , compl, rev_compl    } ;
//// extern char *DNAstrandName[]=    {""        , "(c)", ""        , "(r)"    , "(i)", "(c)"        } ;
//extern char *DNAstrandName[];
//enum fileFormat {fasta=1 , csv=1<<1, f2=1<<2, f3=1<<3} ; // se pueden "OR" los formatos : OUTPUT !!!!!!
//
//char *AttachToCharStr       (const char *CharStr, const char *Attach)    ;
//char *ChangeCharStrAttaching(char *&CharStrToChange, const char *Attach); // CharStrToChange : debe ser una cadena que se creo con new, 
//char *ChangeCharStrAttaching(char *&CharStrToChange, const int Attach);// y que sera borrada y vuelta a crear !!!

///\todo ?? anadir funcion de compactar cod (eliminar los gap y bases deg?). SdH y S se recalculan.
///\todo ?? anadir funcion para regenerar cod no compactado, recordar estado comp/no comp

/*************************************************************************
 *
 *  $RCSfile: expfld.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _EXPFLD_HXX
#define _EXPFLD_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _CELLFML_HXX
#include <cellfml.hxx>
#endif

class SfxPoolItem;
class SwTxtNode;
class SwCalc;
class SwFrm;
struct SwPosition;
class SwTxtFld;
class SwTableSortBoxes;
class SwDoc;
class SwTable;
class SwFmtFld;

// Vorwaertsdeklaration: besorge den "Body-TextNode", fuer Exp.Fld in Fly's
//                      Header/Footers/Footnodes
const SwTxtNode* GetBodyTxtNode( const SwDoc& pDoc, SwPosition& rPos,
                                 const SwFrm& rFrm );
// Wandlung Address -> Adressen
void ReplacePoint(String& sTmpName);

struct _SeqFldLstElem
{
    String sDlgEntry;
    USHORT nSeqNo;

    _SeqFldLstElem( const String& rStr, USHORT nNo )
        : sDlgEntry( rStr ), nSeqNo( nNo )
    {}
};
SV_DECL_PTRARR_DEL( _SwSeqFldList, _SeqFldLstElem*, 10, 10 )

class SwSeqFldList : public _SwSeqFldList
{
public:
    SwSeqFldList()  : _SwSeqFldList( 10, 10 ) {}

    BOOL InsertSort( _SeqFldLstElem* );
    BOOL SeekEntry( const _SeqFldLstElem& , USHORT* pPos = 0 );
};

/*--------------------------------------------------------------------
    Beschreibung: Ausdruck
 --------------------------------------------------------------------*/

class SwGetExpFieldType : public SwValueFieldType
{
public:
        SwGetExpFieldType(SwDoc* pDoc);
        virtual SwFieldType*    Copy() const;

        // ueberlagert, weil das Get-Field nicht veraendert werden kann
        // und dann auch nicht aktualisiert werden muss. Aktualisierung
        // erfolgt beim Aendern von Set-Werten !

        virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );
};

/*--------------------------------------------------------------------
    Beschreibung: GetExperession
 --------------------------------------------------------------------*/

class SwGetExpField : public SwFormulaField
{
    String          sExpand;
    BOOL            bIsInBodyTxt;
    USHORT          nSubType;

public:
    SwGetExpField( SwGetExpFieldType*, const String& rFormel,
                   USHORT nSubType = GSE_EXPR, ULONG nFmt = 0);

    virtual void                SetValue( const double& rVal );
    virtual void                SetLanguage(USHORT nLng);

    virtual String              Expand() const;
    virtual SwField*            Copy() const;

    inline const String&        GetExpStr() const;
    inline void                 ChgExpStr(const String& rExpand);

    // wird von der Formatierung abgefragt
    inline BOOL                 IsInBodyTxt() const;
    // wird von UpdateExpFlds gesetzt (dort ist die Node-Position bekannt)
    inline void                 ChgBodyTxtFlag( BOOL bIsInBody );
    // fuer Felder in Header/Footer/Footnotes/Flys:
    // (wird nur von der Formatierung aufgerufen!!)
    void                        ChangeExpansion( const SwFrm&, const SwTxtFld& );

    virtual String              GetCntnt(BOOL bName = FALSE) const;

    // Die Formel aendern
    virtual String              GetPar2() const;
    virtual void                SetPar2(const String& rStr);

    virtual USHORT              GetSubType() const;
    virtual void                SetSubType(USHORT nType);
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );

    static USHORT       GetReferenceTextPos( const SwFmtFld& rFmt, SwDoc& rDoc);
};

inline void SwGetExpField::ChgExpStr(const String& rExpand)
    { sExpand = rExpand;}

inline const String& SwGetExpField::GetExpStr() const
    { return sExpand;   }

// wird von der Formatierung abgefragt
inline BOOL SwGetExpField::IsInBodyTxt() const
    { return bIsInBodyTxt; }

// wird von UpdateExpFlds gesetzt (dort ist die Node-Position bekannt)
inline void SwGetExpField::ChgBodyTxtFlag( BOOL bIsInBody )
    { bIsInBodyTxt = bIsInBody; }


/*--------------------------------------------------------------------
    Beschreibung: Ausdruck setzen
 --------------------------------------------------------------------*/

class SwSetExpField;

class SwSetExpFieldType : public SwValueFieldType
{
    String      sName;
    const SwNode* pOutlChgNd;
    sal_Unicode cDelim;
    USHORT      nType;
    BYTE        nLevel;
    BOOL        bDeleted;

public:
    SwSetExpFieldType( SwDoc* pDoc, const String& rName,
                        USHORT nType = GSE_EXPR );
    virtual SwFieldType*    Copy() const;
    virtual const String&   GetName() const;

    inline void             SetType(USHORT nTyp);
    inline USHORT           GetType() const;

    void                    SetSeqFormat(ULONG nFormat);
    ULONG                   GetSeqFormat();

    BOOL                    IsDeleted() const       { return bDeleted; }
    void                    SetDeleted( BOOL b )    { bDeleted = b; }

    // ueberlagert, weil das Set-Field selbst dafuer sorgt, das
    // es aktualisiert wird.
    virtual void            Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );
    inline const String&    GetSetRefName() const;

    USHORT SetSeqRefNo( SwSetExpField& rFld );

    USHORT GetSeqFldList( SwSeqFldList& rList );
    String MakeSeqName( USHORT nSeqNo );

    // Seqencefelder ggfs. Kapitelweise numerieren
    sal_Unicode GetDelimiter() const        { return cDelim; }
    void SetDelimiter( sal_Unicode c )      { cDelim = c; }
    BYTE GetOutlineLvl() const              { return nLevel; }
    void SetOutlineLvl( BYTE n )            { nLevel = n; }
    void SetChapter( SwSetExpField& rFld, const SwNode& rNd );
    // Member nur fuers SwDoc::UpdateExpFld - wird nur waehrend der Laufzeit
    // von SequencefeldTypen benoetigt!!!
    const SwNode* GetOutlineChgNd() const   { return pOutlChgNd; }
    void SetOutlineChgNd( const SwNode* p ) { pOutlChgNd = p; }

    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

inline void SwSetExpFieldType::SetType( USHORT nTyp )
{
        nType = nTyp;
        EnableFormat( !(nType & (GSE_SEQ|GSE_STRING)));
}

inline USHORT SwSetExpFieldType::GetType() const
    { return nType;   }

inline const String& SwSetExpFieldType::GetSetRefName() const
    { return sName; }


/*--------------------------------------------------------------------
    Beschreibung: Ausdruck
 --------------------------------------------------------------------*/

class SwSetExpField : public SwFormulaField
{
    String          sExpand;
    String          aPText;
    String          aSeqText;
    BOOL            bInput;
    USHORT          nSeqNo;
    USHORT          nSubType;

public:
    SwSetExpField(SwSetExpFieldType*, const String& rFormel, ULONG nFmt = 0);

    virtual void                SetValue( const double& rVal );

    virtual String              Expand() const;
    virtual SwField*            Copy() const;

    inline const String&        GetExpStr() const;

    inline void                 ChgExpStr( const String& rExpand );

    inline void                 SetPromptText(const String& rStr);
    inline const                String& GetPromptText() const;

    inline void                 SetInputFlag(BOOL bInp);
    inline BOOL                 GetInputFlag() const;

    inline void                 SetSeqText(const String& rStr);
    inline const                String& GetSeqText() const;

    virtual String              GetCntnt(BOOL bName = FALSE) const;
    virtual USHORT              GetSubType() const;
    virtual void                SetSubType(USHORT nType);

    inline BOOL                 IsSequenceFld() const;

    // fuer SequenceFelder - logische Nummer
    inline void                 SetSeqNumber( USHORT n )    { nSeqNo = n; }
    inline USHORT               GetSeqNumber() const        { return nSeqNo; }

    // Der Name nur erfragen
    virtual const String&       GetPar1()   const;

    // Die Formel
    virtual String              GetPar2()   const;
    virtual void                SetPar2(const String& rStr);
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

inline const String& SwSetExpField::GetExpStr() const
    { return sExpand;       }

inline void SwSetExpField::ChgExpStr( const String& rExpand )
    { sExpand = rExpand;    }

inline void  SwSetExpField::SetPromptText(const String& rStr)
    { aPText = rStr;        }

inline const String& SwSetExpField::GetPromptText() const
    { return aPText;        }

inline void  SwSetExpField::SetSeqText(const String& rStr)
    { aSeqText = rStr;          }

inline const String& SwSetExpField::GetSeqText() const
    { return aSeqText;          }

inline void SwSetExpField::SetInputFlag(BOOL bInp)
    { bInput = bInp; }

inline BOOL SwSetExpField::GetInputFlag() const
    { return bInput; }

inline BOOL SwSetExpField::IsSequenceFld() const
    { return 0 != (GSE_SEQ & ((SwSetExpFieldType*)GetTyp())->GetType()); }

/*--------------------------------------------------------------------
    Beschreibung: Eingabe im Text/Variable setzen
 --------------------------------------------------------------------*/

class SwInputFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwInputFieldType( SwDoc* pDoc );

    virtual SwFieldType* Copy() const;

    SwDoc* GetDoc() const { return pDoc; }
};

/*--------------------------------------------------------------------
    Beschreibung: Eingabefeld
 --------------------------------------------------------------------*/

class SwInputField : public SwField
{
    String  aContent;
    String  aPText;
    USHORT  nSubType;
public:
    // Direkte Eingabe ueber Dialog alten Wert loeschen
    SwInputField(SwInputFieldType*, const String& rContent ,
                 const String& rPrompt, USHORT nSubType = 0,
                 ULONG nFmt = 0);

    virtual String          GetCntnt(BOOL bName = FALSE) const;
    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    // Content
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);

    // aPromptText
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);

    virtual USHORT          GetSubType() const;
    virtual void            SetSubType(USHORT nSub);
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

/*--------------------------------------------------------------------
    Beschreibung: Sortieren der Eingabefelder
 --------------------------------------------------------------------*/

class SwInputFieldList
{
public:
    SwInputFieldList( SwEditShell* pShell, FASTBOOL bBuildTmpLst = FALSE );
    ~SwInputFieldList();

    USHORT      Count() const;
    SwField*    GetField(USHORT nId);

    void        GotoFieldPos(USHORT nId);
    void        PushCrsr();
    void        PopCrsr();

    // vergleiche TmpLst mit akt Feldern. Alle neue kommen in die SortLst
    // damit sie geupdatet werden koennen. Returnt die Anzahl.
    // (Fuer Textbausteine: nur seine Input-Felder aktualisieren)
    USHORT      BuildSortLst();

    // Alle unselektierten Felder aus Liste entfernen
    void        RemoveUnselectedFlds();

private:
    SwEditShell*    pSh;
    _SetGetExpFlds* pSrtLst;
    SvPtrarr        aTmpLst;
};

/*--------------------------------------------------------------------
    Beschreibung: Tabellen-Formelfeld
                  (Implementierung steht in tblcalc.cxx)
 --------------------------------------------------------------------*/

class SwTblFieldType : public SwValueFieldType
{
public:
    SwTblFieldType(SwDoc* pDocPtr);
    virtual SwFieldType* Copy() const;
};


// MSC will den hier nicht
//typedef void (SwField:: *FnScanFormel)( const SwTable&, String&,
//                                   String&, String* = 0, void* = 0 );


class SwTblField : public SwValueField, public SwTableFormula
{
    String      sExpand;
    USHORT      nSubType;

    // suche den TextNode, in dem das Feld steht
    virtual const SwNode* GetNodeOfFormula() const;

public:
    SwTblField( SwTblFieldType*, const String& rFormel,
                USHORT nSubType = 0, ULONG nFmt = 0);

    virtual void        SetValue( const double& rVal );
    virtual USHORT      GetSubType() const;
    virtual void        SetSubType(USHORT nType);
    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    const String&       GetExpStr() const               { return sExpand; }
    void                ChgExpStr(const String& rStr)   { sExpand = rStr; }

    // berechne sich selbst
    void                CalcField( SwTblCalcPara& rCalcPara );

    virtual String      GetCntnt(BOOL bName = FALSE) const;

    // Die Formel
    virtual String      GetPar2()   const;
    virtual void        SetPar2(const String& rStr);
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};


#endif // _EXPFLD_HXX

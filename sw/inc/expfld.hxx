/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SW_EXPFLD_HXX
#define SW_EXPFLD_HXX

#include <svl/svarray.hxx>
#include "swdllapi.h"
#include <fldbas.hxx>
#include <cellfml.hxx>

class SfxPoolItem;
class SwTxtNode;
class SwFrm;
struct SwPosition;
class SwTxtFld;
class SwDoc;
class SwFmtFld;
class _SetGetExpFlds;
class SwEditShell;

// Vorwaertsdeklaration: besorge den "Body-TextNode", fuer Exp.Fld in Fly's
//                      Header/Footers/Footnodes
const SwTxtNode* GetBodyTxtNode( const SwDoc& pDoc, SwPosition& rPos,
                                 const SwFrm& rFrm );
// Wandlung Address -> Adressen
void ReplacePoint(String& sTmpName, sal_Bool bWithCommandType = sal_False);

struct _SeqFldLstElem
{
    String sDlgEntry;
    sal_uInt16 nSeqNo;

    _SeqFldLstElem( const String& rStr, sal_uInt16 nNo )
        : sDlgEntry( rStr ), nSeqNo( nNo )
    {}
};
SV_DECL_PTRARR_DEL( _SwSeqFldList, _SeqFldLstElem*, 10, 10 )

class SW_DLLPUBLIC SwSeqFldList : public _SwSeqFldList
{
public:
    SwSeqFldList()  : _SwSeqFldList( 10, 10 ) {}

    sal_Bool InsertSort( _SeqFldLstElem* );
    sal_Bool SeekEntry( const _SeqFldLstElem& , sal_uInt16* pPos = 0 );
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
protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );
};

/*--------------------------------------------------------------------
    Beschreibung: GetExperession
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwGetExpField : public SwFormulaField
{
    String          sExpand;
    sal_Bool            bIsInBodyTxt;
    sal_uInt16          nSubType;

    bool            bLateInitialization; // #i82544#

    virtual String              Expand() const;
    virtual SwField*            Copy() const;

public:
    SwGetExpField( SwGetExpFieldType*, const String& rFormel,
                   sal_uInt16 nSubType = nsSwGetSetExpType::GSE_EXPR, sal_uLong nFmt = 0);

    virtual void                SetValue( const double& rVal );
    virtual void                SetLanguage(sal_uInt16 nLng);

    inline const String&        GetExpStr() const;
    inline void                 ChgExpStr(const String& rExpand);

    // wird von der Formatierung abgefragt
    inline sal_Bool                 IsInBodyTxt() const;
    // wird von UpdateExpFlds gesetzt (dort ist die Node-Position bekannt)
    inline void                 ChgBodyTxtFlag( sal_Bool bIsInBody );
    // fuer Felder in Header/Footer/Footnotes/Flys:
    // (wird nur von der Formatierung aufgerufen!!)
    void                        ChangeExpansion( const SwFrm&, const SwTxtFld& );

    virtual String      GetFieldName() const;

    // Die Formel aendern
    virtual String              GetPar2() const;
    virtual void                SetPar2(const String& rStr);

    virtual sal_uInt16              GetSubType() const;
    virtual void                SetSubType(sal_uInt16 nType);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

    static sal_uInt16       GetReferenceTextPos( const SwFmtFld& rFmt, SwDoc& rDoc);
    // #i82544#
    void                SetLateInitialization() { bLateInitialization = true;}
};

inline void SwGetExpField::ChgExpStr(const String& rExpand)
    { sExpand = rExpand;}

inline const String& SwGetExpField::GetExpStr() const
    { return sExpand;   }

// wird von der Formatierung abgefragt
inline sal_Bool SwGetExpField::IsInBodyTxt() const
    { return bIsInBodyTxt; }

// wird von UpdateExpFlds gesetzt (dort ist die Node-Position bekannt)
inline void SwGetExpField::ChgBodyTxtFlag( sal_Bool bIsInBody )
    { bIsInBodyTxt = bIsInBody; }


/*--------------------------------------------------------------------
    Beschreibung: Ausdruck setzen
 --------------------------------------------------------------------*/

class SwSetExpField;

class SW_DLLPUBLIC SwSetExpFieldType : public SwValueFieldType
{
    String      sName;
    const SwNode* pOutlChgNd;
    String      sDelim;
    sal_uInt16      nType;
    sal_uInt8       nLevel;
    sal_Bool        bDeleted;

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );

public:
    SwSetExpFieldType( SwDoc* pDoc, const String& rName,
                        sal_uInt16 nType = nsSwGetSetExpType::GSE_EXPR );
    virtual SwFieldType*    Copy() const;
    virtual const String&   GetName() const;

    inline void             SetType(sal_uInt16 nTyp);
    inline sal_uInt16           GetType() const;

    void                    SetSeqFormat(sal_uLong nFormat);
    sal_uLong                   GetSeqFormat();

    sal_Bool                    IsDeleted() const       { return bDeleted; }
    void                    SetDeleted( sal_Bool b )    { bDeleted = b; }

    // ueberlagert, weil das Set-Field selbst dafuer sorgt, das
    // es aktualisiert wird.
    inline const String&    GetSetRefName() const;

    sal_uInt16 SetSeqRefNo( SwSetExpField& rFld );

    sal_uInt16 GetSeqFldList( SwSeqFldList& rList );
    String MakeSeqName( sal_uInt16 nSeqNo );

    // Seqencefelder ggfs. Kapitelweise numerieren
//  sal_Unicode GetDelimiter() const        { return cDelim; }
//  void SetDelimiter( sal_Unicode c )      { cDelim = c; }
    const String& GetDelimiter() const      { return sDelim; }
    void SetDelimiter( const String& s )    { sDelim = s; }
    sal_uInt8 GetOutlineLvl() const                 { return nLevel; }
    void SetOutlineLvl( sal_uInt8 n )           { nLevel = n; }
    void SetChapter( SwSetExpField& rFld, const SwNode& rNd );
    // Member nur fuers SwDoc::UpdateExpFld - wird nur waehrend der Laufzeit
    // von SequencefeldTypen benoetigt!!!
    const SwNode* GetOutlineChgNd() const   { return pOutlChgNd; }
    void SetOutlineChgNd( const SwNode* p ) { pOutlChgNd = p; }

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline void SwSetExpFieldType::SetType( sal_uInt16 nTyp )
{
        nType = nTyp;
        EnableFormat( !(nType & (nsSwGetSetExpType::GSE_SEQ|nsSwGetSetExpType::GSE_STRING)));
}

inline sal_uInt16 SwSetExpFieldType::GetType() const
    { return nType;   }

inline const String& SwSetExpFieldType::GetSetRefName() const
    { return sName; }


/*--------------------------------------------------------------------
    Beschreibung: Ausdruck
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwSetExpField : public SwFormulaField
{
    String          sExpand;
    String          aPText;
    String          aSeqText;
    sal_Bool            bInput;
    sal_uInt16          nSeqNo;
    sal_uInt16          nSubType;

    virtual String              Expand() const;
    virtual SwField*            Copy() const;

public:
    SwSetExpField(SwSetExpFieldType*, const String& rFormel, sal_uLong nFmt = 0);

    virtual void                SetValue( const double& rVal );

    inline const String&        GetExpStr() const;

    inline void                 ChgExpStr( const String& rExpand );

    inline void                 SetPromptText(const String& rStr);
    inline const                String& GetPromptText() const;

    inline void                 SetInputFlag(sal_Bool bInp);
    inline sal_Bool                 GetInputFlag() const;

    virtual String              GetFieldName() const;

    virtual sal_uInt16              GetSubType() const;
    virtual void                SetSubType(sal_uInt16 nType);

    inline sal_Bool                 IsSequenceFld() const;

    // fuer SequenceFelder - logische Nummer
    inline void                 SetSeqNumber( sal_uInt16 n )    { nSeqNo = n; }
    inline sal_uInt16               GetSeqNumber() const        { return nSeqNo; }

    // Der Name nur erfragen
    virtual const String&       GetPar1()   const;

    // Die Formel
    virtual String              GetPar2()   const;
    virtual void                SetPar2(const String& rStr);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline const String& SwSetExpField::GetExpStr() const
    { return sExpand;       }

inline void SwSetExpField::ChgExpStr( const String& rExpand )
    { sExpand = rExpand;    }

inline void  SwSetExpField::SetPromptText(const String& rStr)
    { aPText = rStr;        }

inline const String& SwSetExpField::GetPromptText() const
    { return aPText;        }

inline void SwSetExpField::SetInputFlag(sal_Bool bInp)
    { bInput = bInp; }

inline sal_Bool SwSetExpField::GetInputFlag() const
    { return bInput; }

inline sal_Bool SwSetExpField::IsSequenceFld() const
    { return 0 != (nsSwGetSetExpType::GSE_SEQ & ((SwSetExpFieldType*)GetTyp())->GetType()); }

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

class SW_DLLPUBLIC SwInputField : public SwField
{
    String  aContent;
    String  aPText;
    String  aHelp;
    String  aToolTip;
    sal_uInt16  nSubType;

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

public:
    // Direkte Eingabe ueber Dialog alten Wert loeschen
    SwInputField(SwInputFieldType*, const String& rContent ,
                 const String& rPrompt, sal_uInt16 nSubType = 0,
                 sal_uLong nFmt = 0);

    virtual String          GetFieldName() const;

    // Content
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);

    // aPromptText
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);

    virtual String          GetHelp() const;
    virtual void            SetHelp(const String & rStr);

    virtual String          GetToolTip() const;
    virtual void            SetToolTip(const String & rStr);

    virtual sal_Bool            isFormField() const;

    virtual sal_uInt16          GetSubType() const;
    virtual void            SetSubType(sal_uInt16 nSub);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Description: Sorted list of input fields and DropDown fields
 --------------------------------------------------------------------*/

class SwInputFieldList
{
public:
    SwInputFieldList( SwEditShell* pShell, sal_Bool bBuildTmpLst = sal_False );
    ~SwInputFieldList();

    sal_uInt16      Count() const;
    SwField*    GetField(sal_uInt16 nId);

    void        GotoFieldPos(sal_uInt16 nId);
    void        PushCrsr();
    void        PopCrsr();

    // vergleiche TmpLst mit akt Feldern. Alle neue kommen in die SortLst
    // damit sie geupdatet werden koennen. Returnt die Anzahl.
    // (Fuer Textbausteine: nur seine Input-Felder aktualisieren)
    sal_uInt16      BuildSortLst();

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
    sal_uInt16      nSubType;

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    // suche den TextNode, in dem das Feld steht
    virtual const SwNode* GetNodeOfFormula() const;

    String GetCommand();

public:
    SwTblField( SwTblFieldType*, const String& rFormel,
                sal_uInt16 nSubType = 0, sal_uLong nFmt = 0);

    virtual void        SetValue( const double& rVal );
    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType(sal_uInt16 nType);

    const String&       GetExpStr() const               { return sExpand; }
    void                ChgExpStr(const String& rStr)   { sExpand = rStr; }

    // berechne sich selbst
    void                CalcField( SwTblCalcPara& rCalcPara );

    virtual String      GetFieldName() const;

    // Die Formel
    virtual String      GetPar2()   const;
    virtual void        SetPar2(const String& rStr);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};


#endif // SW_EXPFLD_HXX

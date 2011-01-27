/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _EXPFLD_HXX
#define _EXPFLD_HXX

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

// Forward declaration: get "BodyTxtNode" for exp.fld in Fly's headers/footers/footnotes.
const SwTxtNode* GetBodyTxtNode( const SwDoc& pDoc, SwPosition& rPos,
                                 const SwFrm& rFrm );

void ReplacePoint(String& sTmpName, BOOL bWithCommandType = FALSE);

struct _SeqFldLstElem
{
    String sDlgEntry;
    USHORT nSeqNo;

    _SeqFldLstElem( const String& rStr, USHORT nNo )
        : sDlgEntry( rStr ), nSeqNo( nNo )
    {}
};
SV_DECL_PTRARR_DEL( _SwSeqFldList, _SeqFldLstElem*, 10, 10 )

class SW_DLLPUBLIC SwSeqFldList : public _SwSeqFldList
{
public:
    SwSeqFldList()  : _SwSeqFldList( 10, 10 ) {}

    BOOL InsertSort( _SeqFldLstElem* );
    BOOL SeekEntry( const _SeqFldLstElem& , USHORT* pPos = 0 );
};

class SwGetExpFieldType : public SwValueFieldType
{
public:
        SwGetExpFieldType(SwDoc* pDoc);
        virtual SwFieldType*    Copy() const;

        // Overlay, because get-field cannot be changed and therefore
        // does not need to be updated. Update at changing of set-values!

        virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );
};

class SW_DLLPUBLIC SwGetExpField : public SwFormulaField
{
    String          sExpand;
    BOOL            bIsInBodyTxt;
    USHORT          nSubType;

    bool            bLateInitialization; // #i82544#

public:
    SwGetExpField( SwGetExpFieldType*, const String& rFormel,
                   USHORT nSubType = nsSwGetSetExpType::GSE_EXPR, ULONG nFmt = 0);

    virtual void                SetValue( const double& rVal );
    virtual void                SetLanguage(USHORT nLng);

    virtual String              Expand() const;
    virtual SwField*            Copy() const;

    inline const String&        GetExpStr() const;
    inline void                 ChgExpStr(const String& rExpand);

    // Called by formating.
    inline BOOL                 IsInBodyTxt() const;

    // Set by UpdateExpFlds where node position is known.
    inline void                 ChgBodyTxtFlag( BOOL bIsInBody );

    // For fields in header/footer/footnotes/flys:
    // Only called by formating!!
    void                        ChangeExpansion( const SwFrm&, const SwTxtFld& );

    virtual String              GetCntnt(BOOL bName = FALSE) const;

    // Change formula.
    virtual String              GetPar2() const;
    virtual void                SetPar2(const String& rStr);

    virtual USHORT              GetSubType() const;
    virtual void                SetSubType(USHORT nType);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhich );

    static USHORT       GetReferenceTextPos( const SwFmtFld& rFmt, SwDoc& rDoc);
    // #i82544#
    void                SetLateInitialization() { bLateInitialization = true;}
};

inline void SwGetExpField::ChgExpStr(const String& rExpand)
    { sExpand = rExpand;}

inline const String& SwGetExpField::GetExpStr() const
    { return sExpand;   }

// Called by formating.
inline BOOL SwGetExpField::IsInBodyTxt() const
    { return bIsInBodyTxt; }

// Set by UpdateExpFlds where node position is known.
inline void SwGetExpField::ChgBodyTxtFlag( BOOL bIsInBody )
    { bIsInBodyTxt = bIsInBody; }

class SwSetExpField;

class SW_DLLPUBLIC SwSetExpFieldType : public SwValueFieldType
{
    String      sName;
    const SwNode* pOutlChgNd;
    String      sDelim;
    USHORT      nType;
    BYTE        nLevel;
    BOOL        bDeleted;

public:
    SwSetExpFieldType( SwDoc* pDoc, const String& rName,
                        USHORT nType = nsSwGetSetExpType::GSE_EXPR );
    virtual SwFieldType*    Copy() const;
    virtual const String&   GetName() const;

    inline void             SetType(USHORT nTyp);
    inline USHORT           GetType() const;

    void                    SetSeqFormat(ULONG nFormat);
    ULONG                   GetSeqFormat();

    BOOL                    IsDeleted() const       { return bDeleted; }
    void                    SetDeleted( BOOL b )    { bDeleted = b; }

    // Overlay, because set-field takes care for its being updated by itself.
    virtual void            Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );
    inline const String&    GetSetRefName() const;

    USHORT SetSeqRefNo( SwSetExpField& rFld );

    USHORT GetSeqFldList( SwSeqFldList& rList );
    String MakeSeqName( USHORT nSeqNo );

    // Number sequence fields chapterwise if required.
    const String& GetDelimiter() const      { return sDelim; }
    void SetDelimiter( const String& s )    { sDelim = s; }
    BYTE GetOutlineLvl() const              { return nLevel; }
    void SetOutlineLvl( BYTE n )            { nLevel = n; }
    void SetChapter( SwSetExpField& rFld, const SwNode& rNd );

    // Member only for SwDoc::UpdateExpFld.
    // It is needed only at runtime of sequence field types!
    const SwNode* GetOutlineChgNd() const   { return pOutlChgNd; }
    void SetOutlineChgNd( const SwNode* p ) { pOutlChgNd = p; }

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhich );
};

inline void SwSetExpFieldType::SetType( USHORT nTyp )
{
        nType = nTyp;
        EnableFormat( !(nType & (nsSwGetSetExpType::GSE_SEQ|nsSwGetSetExpType::GSE_STRING)));
}

inline USHORT SwSetExpFieldType::GetType() const
    { return nType;   }

inline const String& SwSetExpFieldType::GetSetRefName() const
    { return sName; }


class SW_DLLPUBLIC SwSetExpField : public SwFormulaField
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

    virtual String              GetCntnt(BOOL bName = FALSE) const;
    virtual USHORT              GetSubType() const;
    virtual void                SetSubType(USHORT nType);

    inline BOOL                 IsSequenceFld() const;

    // Logical number, sequence fields.
    inline void                 SetSeqNumber( USHORT n )    { nSeqNo = n; }
    inline USHORT               GetSeqNumber() const        { return nSeqNo; }

    // Query name only.
    virtual const String&       GetPar1()   const;

    // Query formula.
    virtual String              GetPar2()   const;
    virtual void                SetPar2(const String& rStr);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhich );
};

inline const String& SwSetExpField::GetExpStr() const
    { return sExpand;       }

inline void SwSetExpField::ChgExpStr( const String& rExpand )
    { sExpand = rExpand;    }

inline void  SwSetExpField::SetPromptText(const String& rStr)
    { aPText = rStr;        }

inline const String& SwSetExpField::GetPromptText() const
    { return aPText;        }

inline void SwSetExpField::SetInputFlag(BOOL bInp)
    { bInput = bInp; }

inline BOOL SwSetExpField::GetInputFlag() const
    { return bInput; }

inline BOOL SwSetExpField::IsSequenceFld() const
    { return 0 != (nsSwGetSetExpType::GSE_SEQ & ((SwSetExpFieldType*)GetTyp())->GetType()); }


class SwInputFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwInputFieldType( SwDoc* pDoc );

    virtual SwFieldType* Copy() const;

    SwDoc* GetDoc() const { return pDoc; }
};


class SW_DLLPUBLIC SwInputField : public SwField
{
    String  aContent;
    String  aPText;
    String  aHelp;
    String  aToolTip;
    USHORT  nSubType;
public:
    // Direct input via dialog; delete old value.
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

    virtual String          GetHelp() const;
    virtual void            SetHelp(const String & rStr);

    virtual String          GetToolTip() const;
    virtual void            SetToolTip(const String & rStr);

    virtual BOOL            isFormField() const;

    virtual USHORT          GetSubType() const;
    virtual void            SetSubType(USHORT nSub);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhich );
};

/*--------------------------------------------------------------------
    Description: Sorted list of input fields and DropDown fields
 --------------------------------------------------------------------*/

class SwInputFieldList
{
public:
    SwInputFieldList( SwEditShell* pShell, BOOL bBuildTmpLst = FALSE );
    ~SwInputFieldList();

    USHORT      Count() const;
    SwField*    GetField(USHORT nId);

    void        GotoFieldPos(USHORT nId);
    void        PushCrsr();
    void        PopCrsr();

    // Put all that are new into SortLst for updating. Returns count.
    // (For Glossary: only update its input-fields).
    // Compare TmpLst with current fields.
    USHORT      BuildSortLst();

    void        RemoveUnselectedFlds();

private:
    SwEditShell*    pSh;
    _SetGetExpFlds* pSrtLst;
    SvPtrarr        aTmpLst;
};

// Implementation in tblcalc.cxx.
class SwTblFieldType : public SwValueFieldType
{
public:
    SwTblFieldType(SwDoc* pDocPtr);
    virtual SwFieldType* Copy() const;
};


class SwTblField : public SwValueField, public SwTableFormula
{
    String      sExpand;
    USHORT      nSubType;

    // Search TextNode containing the field.
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

    void                CalcField( SwTblCalcPara& rCalcPara );

    virtual String      GetCntnt(BOOL bName = FALSE) const;

    // The formula.
    virtual String      GetPar2()   const;
    virtual void        SetPar2(const String& rStr);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhich );
};


#endif // _EXPFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

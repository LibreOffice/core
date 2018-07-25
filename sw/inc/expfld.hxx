/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_EXPFLD_HXX
#define INCLUDED_SW_INC_EXPFLD_HXX

#include "swdllapi.h"
#include "fldbas.hxx"
#include "cellfml.hxx"
#include <set>
#include <memory>
#include <vector>

class SfxPoolItem;
class SwTextNode;
class SwFrame;
struct SwPosition;
class SwTextField;
class SwDoc;
class SwFormatField;
class SetGetExpFields;
class SwEditShell;

/// Forward declaration: get "BodyTextNode" for exp.fld in Fly's headers/footers/footnotes.
const SwTextNode* GetBodyTextNode( const SwDoc& pDoc, SwPosition& rPos,
                                 const SwFrame& rFrame );

OUString ReplacePoint(const OUString& sTmpName, bool bWithCommandType = false);

struct SeqFieldLstElem
{
    OUString sDlgEntry;
    sal_uInt16 nSeqNo;

    SeqFieldLstElem( const OUString& rStr, sal_uInt16 nNo )
        : sDlgEntry( rStr ), nSeqNo( nNo )
    {}
};

class SW_DLLPUBLIC SwSeqFieldList
{
    std::vector<SeqFieldLstElem*> maData;
public:
    ~SwSeqFieldList()
    {
        for( std::vector<SeqFieldLstElem*>::const_iterator it = maData.begin(); it != maData.end(); ++it )
            delete *it;
    }

    bool InsertSort(SeqFieldLstElem* pNew);
    bool SeekEntry(const SeqFieldLstElem& rNew, size_t* pPos) const;

    size_t Count() { return maData.size(); }
    SeqFieldLstElem* operator[](size_t nIndex) { return maData[nIndex]; }
    const SeqFieldLstElem* operator[](size_t nIndex) const { return maData[nIndex]; }
    void Clear() { maData.clear(); }
};

class SwGetExpFieldType : public SwValueFieldType
{
public:
    SwGetExpFieldType(SwDoc* pDoc);
    virtual SwFieldType*    Copy() const override;

    /** Overlay, because get-field cannot be changed and therefore
     does not need to be updated. Update at changing of set-values! */
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;
};

class SW_DLLPUBLIC SwGetExpField : public SwFormulaField
{
    OUString        m_sExpand;
    bool            m_bIsInBodyText;
    sal_uInt16          m_nSubType;

    bool            m_bLateInitialization; // #i82544#

    virtual OUString            Expand() const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwGetExpField( SwGetExpFieldType*, const OUString& rFormel,
                   sal_uInt16 nSubType, sal_uLong nFormat);

    virtual void                SetValue( const double& rVal ) override;
    virtual void                SetLanguage(LanguageType nLng) override;

    inline void                 ChgExpStr(const OUString& rExpand);

    /// Called by formatting.
    inline bool                 IsInBodyText() const;

    /// Set by UpdateExpFields where node position is known.
    inline void                 ChgBodyTextFlag( bool bIsInBody );

    /** For fields in header/footer/footnotes/flys:
     Only called by formatting!! */
    void                        ChangeExpansion( const SwFrame&, const SwTextField& );

    virtual OUString    GetFieldName() const override;

    /// Change formula.
    virtual OUString GetPar2() const override;
    virtual void        SetPar2(const OUString& rStr) override;

    virtual sal_uInt16  GetSubType() const override;
    virtual void        SetSubType(sal_uInt16 nType) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

    static sal_Int32    GetReferenceTextPos( const SwFormatField& rFormat, SwDoc& rDoc, sal_Int32 nHint = 0);
    // #i82544#
    void                SetLateInitialization() { m_bLateInitialization = true;}
};

inline void SwGetExpField::ChgExpStr(const OUString& rExpand)
    { m_sExpand = rExpand;}

 /// Called by formatting.
inline bool SwGetExpField::IsInBodyText() const
    { return m_bIsInBodyText; }

 /// Set by UpdateExpFields where node position is known.
inline void SwGetExpField::ChgBodyTextFlag( bool bIsInBody )
    { m_bIsInBodyText = bIsInBody; }

class SwSetExpField;

class SW_DLLPUBLIC SwSetExpFieldType : public SwValueFieldType
{
    OUString sName;
    const SwNode* pOutlChgNd;
    OUString      sDelim;
    sal_uInt16      nType;
    sal_uInt8       nLevel;
    bool        bDeleted;

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

public:
    SwSetExpFieldType( SwDoc* pDoc, const OUString& rName,
                        sal_uInt16 nType = nsSwGetSetExpType::GSE_EXPR );
    virtual SwFieldType*    Copy() const override;
    virtual OUString        GetName() const override;

    inline void             SetType(sal_uInt16 nTyp);
    inline sal_uInt16       GetType() const;

    void                    SetSeqFormat(sal_uLong nFormat);
    sal_uLong               GetSeqFormat();

    bool                IsDeleted() const       { return bDeleted; }
    void                    SetDeleted( bool b )    { bDeleted = b; }

    /// Overlay, because set-field takes care for its being updated by itself.
    inline const OUString&  GetSetRefName() const;

    void   SetSeqRefNo( SwSetExpField& rField );

    size_t GetSeqFieldList( SwSeqFieldList& rList );

    /// Number sequence fields chapterwise if required.
    const OUString& GetDelimiter() const      { return sDelim; }
    void SetDelimiter( const OUString& s )    { sDelim = s; }
    sal_uInt8 GetOutlineLvl() const             { return nLevel; }
    void SetOutlineLvl( sal_uInt8 n )           { nLevel = n; }
    void SetChapter( SwSetExpField& rField, const SwNode& rNd );

    /** Member only for SwDoc::UpdateExpField.
     It is needed only at runtime of sequence field types! */
    const SwNode* GetOutlineChgNd() const   { return pOutlChgNd; }
    void SetOutlineChgNd( const SwNode* p ) { pOutlChgNd = p; }

    virtual void        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual void        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline void SwSetExpFieldType::SetType( sal_uInt16 nTyp )
{
        nType = nTyp;
        EnableFormat( !(nType & (nsSwGetSetExpType::GSE_SEQ|nsSwGetSetExpType::GSE_STRING)));
}

inline sal_uInt16 SwSetExpFieldType::GetType() const
    { return nType;   }

inline const OUString& SwSetExpFieldType::GetSetRefName() const
    { return sName; }

class SW_DLLPUBLIC SwSetExpField : public SwFormulaField
{
    OUString        sExpand;
    OUString        aPText;
    bool            bInput;
    sal_uInt16          nSeqNo;
    sal_uInt16          nSubType;
    SwFormatField * mpFormatField; /// pool item to which the SwSetExpField belongs

    virtual OUString            Expand() const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwSetExpField(SwSetExpFieldType*, const OUString& rFormel, sal_uLong nFormat = 0);

    void SetFormatField(SwFormatField & rFormatField);
    SwFormatField* GetFormatField() { return mpFormatField;}

    virtual void                SetValue( const double& rVal ) override;

    inline const OUString&      GetExpStr() const;

    inline void                 ChgExpStr( const OUString& rExpand );

    inline void                 SetPromptText(const OUString& rStr);
    inline const OUString&      GetPromptText() const;

    inline void                 SetInputFlag(bool bInp);
    inline bool                 GetInputFlag() const;

    virtual OUString            GetFieldName() const override;

    virtual sal_uInt16              GetSubType() const override;
    virtual void                SetSubType(sal_uInt16 nType) override;

    inline bool                 IsSequenceField() const;

    /// Logical number, sequence fields.
    void                 SetSeqNumber( sal_uInt16 n )    { nSeqNo = n; }
    sal_uInt16           GetSeqNumber() const        { return nSeqNo; }

    /// Query name only.
    virtual OUString       GetPar1()   const override;

    /// Query formula.
    virtual OUString       GetPar2()   const override;
    virtual void                SetPar2(const OUString& rStr) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline const OUString& SwSetExpField::GetExpStr() const
    { return sExpand;       }

inline void SwSetExpField::ChgExpStr( const OUString& rExpand )
    { sExpand = rExpand;    }

inline void  SwSetExpField::SetPromptText(const OUString& rStr)
    { aPText = rStr;        }

inline const OUString& SwSetExpField::GetPromptText() const
    { return aPText;        }

inline void SwSetExpField::SetInputFlag(bool bInp)
    { bInput = bInp; }

inline bool SwSetExpField::GetInputFlag() const
    { return bInput; }

inline bool SwSetExpField::IsSequenceField() const
    { return 0 != (nsSwGetSetExpType::GSE_SEQ & static_cast<SwSetExpFieldType*>(GetTyp())->GetType()); }

class SwInputFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwInputFieldType( SwDoc* pDoc );

    virtual SwFieldType* Copy() const override;

    SwDoc* GetDoc() const { return pDoc; }
};

class SW_DLLPUBLIC SwInputField : public SwField
{
    mutable OUString aContent;
    OUString aPText;
    OUString aHelp;
    OUString aToolTip;
    sal_uInt16 nSubType;
    bool mbIsFormField;

    SwFormatField* mpFormatField; // attribute to which the <SwInputField> belongs to

    virtual OUString        Expand() const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    // Accessing Input Field's content
    const OUString& getContent() const { return aContent;}

public:
    /// Direct input via dialog; delete old value.
    SwInputField(
        SwInputFieldType* pFieldType,
        const OUString& rContent,
        const OUString& rPrompt,
        sal_uInt16 nSubType,
        sal_uLong nFormat = 0,
        bool bIsFormField = true );
    virtual ~SwInputField() override;

    void SetFormatField( SwFormatField& rFormatField );
    SwFormatField* GetFormatField() { return mpFormatField;}

    // Providing new Input Field's content:
    // Fill Input Field's content depending on <nSupType>.
    void applyFieldContent( const OUString& rNewFieldContent );

    bool isFormField() const;

    virtual OUString        GetFieldName() const override;

    /// Content
    virtual OUString        GetPar1() const override;
    virtual void            SetPar1(const OUString& rStr) override;

    /// aPromptText
    virtual OUString        GetPar2() const override;
    virtual void            SetPar2(const OUString& rStr) override;

    const OUString&         GetHelp() const;
    void                    SetHelp(const OUString & rStr);

    const OUString&         GetToolTip() const;
    void                    SetToolTip(const OUString & rStr);

    virtual sal_uInt16      GetSubType() const override;
    virtual void            SetSubType(sal_uInt16 nSub) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Sorted list of input fields and DropDown fields
class SwInputFieldList
{
public:
    SwInputFieldList( SwEditShell* pShell, bool bBuildTmpLst = false );
    ~SwInputFieldList();

    size_t      Count() const;
    SwField*    GetField(size_t nId);

    void        GotoFieldPos(size_t nId);
    void        PushCursor();
    void        PopCursor();

    /** Put all that are new into SortList for updating. @return true if not empty.
     (For Glossary: only update its input-fields).
     Compare TmpLst with current fields. */
    bool        BuildSortLst();

private:
    SwEditShell*                      pSh;
    std::unique_ptr<SetGetExpFields>  pSrtLst;
    std::set<const SwTextField*>      aTmpLst;
};

 /// Implementation in tblcalc.cxx.
class SwTableFieldType : public SwValueFieldType
{
public:
    SwTableFieldType(SwDoc* pDocPtr);
    virtual SwFieldType* Copy() const override;
};

class SwTableField : public SwValueField, public SwTableFormula
{
    OUString      sExpand;
    sal_uInt16      nSubType;

    virtual OUString    Expand() const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    /// Search TextNode containing the field.
    virtual const SwNode* GetNodeOfFormula() const override;

    OUString GetCommand();

public:
    SwTableField( SwTableFieldType*, const OUString& rFormel,
                sal_uInt16 nSubType, sal_uLong nFormat);

    virtual void        SetValue( const double& rVal ) override;
    virtual sal_uInt16  GetSubType() const override;
    virtual void        SetSubType(sal_uInt16 nType) override;

    void                ChgExpStr(const OUString& rStr) { sExpand = rStr; }

    void                CalcField( SwTableCalcPara& rCalcPara );

    virtual OUString    GetFieldName() const override;

    /// The formula.
    virtual OUString GetPar2()   const override;
    virtual void        SetPar2(const OUString& rStr) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

#endif // INCLUDED_SW_INC_EXPFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

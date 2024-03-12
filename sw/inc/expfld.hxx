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
#include <memory>
#include <utility>
#include <vector>
#include <tools/solar.h>
#include <o3tl/sorted_vector.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

class SfxPoolItem;
class SwTextNode;
class SwFrame;
class SwRootFrame;
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

    SeqFieldLstElem( OUString aStr, sal_uInt16 nNo )
        : sDlgEntry(std::move( aStr )), nSeqNo( nNo )
    {}
};

class SW_DLLPUBLIC SwSeqFieldList
{
    std::vector<SeqFieldLstElem> maData;
public:
    bool InsertSort(SeqFieldLstElem aNew);
    bool SeekEntry(const SeqFieldLstElem& rNew, size_t* pPos) const;

    size_t Count() { return maData.size(); }
    SeqFieldLstElem& operator[](size_t nIndex) { return maData[nIndex]; }
    const SeqFieldLstElem& operator[](size_t nIndex) const { return maData[nIndex]; }
    void Clear() { maData.clear(); }
};

class SAL_DLLPUBLIC_RTTI SwGetExpFieldType final : public SwValueFieldType
{
public:
    SwGetExpFieldType(SwDoc* pDoc);
    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual void UpdateFields() override {};

    /** Overlay, because get-field cannot be changed and therefore
     does not need to be updated. Update at changing of set-values! */
private:
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;
};

class SAL_DLLPUBLIC_RTTI SwGetExpField final : public SwFormulaField
{
    double          m_fValueRLHidden; ///< SwValueField; hidden redlines
    OUString        m_sExpand;
    OUString        m_sExpandRLHidden; ///< hidden redlines
    bool            m_bIsInBodyText;
    sal_uInt16          m_nSubType;

    bool            m_bLateInitialization; // #i82544#

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;
    using SwFormulaField::GetValue; // hide it, don't use
    virtual void        SetValue(const double& rVal) override; // hide it

public:
    SW_DLLPUBLIC SwGetExpField( SwGetExpFieldType*, const OUString& rFormel,
                   sal_uInt16 nSubType, sal_uLong nFormat);

    double      GetValue(SwRootFrame const* pLayout) const;
    void        SetValue(const double& rVal, SwRootFrame const* pLayout);

    virtual void                SetLanguage(LanguageType nLng) override;

    void                 ChgExpStr(const OUString& rExpand, SwRootFrame const* pLayout);

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

    SW_DLLPUBLIC static sal_Int32 GetReferenceTextPos( const SwFormatField& rFormat, SwDoc& rDoc, sal_Int32 nHint = 0);
    // #i82544#
    void                SetLateInitialization() { m_bLateInitialization = true;}
};

 /// Called by formatting.
inline bool SwGetExpField::IsInBodyText() const
    { return m_bIsInBodyText; }

 /// Set by UpdateExpFields where node position is known.
inline void SwGetExpField::ChgBodyTextFlag( bool bIsInBody )
    { m_bIsInBodyText = bIsInBody; }

class SwSetExpField;

class SW_DLLPUBLIC SwSetExpFieldType final : public SwValueFieldType
{
    OUString       m_sName;
    OUString      m_sDelim;
    sal_uInt16      m_nType;
    sal_uInt8       m_nLevel;
    bool        m_bDeleted;

    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:
    SwSetExpFieldType( SwDoc* pDoc, OUString aName,
                        sal_uInt16 nType = nsSwGetSetExpType::GSE_EXPR );
    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual OUString        GetName() const override;

    inline void             SetType(sal_uInt16 nTyp);
    inline sal_uInt16       GetType() const;

    void                    SetSeqFormat(sal_uLong nFormat);
    sal_uLong               GetSeqFormat() const;

    bool                IsDeleted() const       { return m_bDeleted; }
    void                    SetDeleted( bool b )    { m_bDeleted = b; }

    /// Overlay, because set-field takes care for its being updated by itself.
    inline const OUString&  GetSetRefName() const;

    void   SetSeqRefNo( SwSetExpField& rField );

    size_t GetSeqFieldList(SwSeqFieldList& rList, SwRootFrame const* pLayout);

    /// Number sequence fields chapterwise if required.
    const OUString& GetDelimiter() const      { return m_sDelim; }
    void SetDelimiter( const OUString& s )    { m_sDelim = s; }
    sal_uInt8 GetOutlineLvl() const             { return m_nLevel; }
    void SetOutlineLvl( sal_uInt8 n )           { m_nLevel = n; }
    void SetChapter(SwSetExpField& rField, const SwNode& rNd, SwRootFrame const* pLayout);

    virtual void QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual void PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
    virtual void UpdateFields() override {};
};

inline void SwSetExpFieldType::SetType( sal_uInt16 nTyp )
{
        m_nType = nTyp;
        EnableFormat( !(m_nType & (nsSwGetSetExpType::GSE_SEQ|nsSwGetSetExpType::GSE_STRING)));
}

inline sal_uInt16 SwSetExpFieldType::GetType() const
    { return m_nType;   }

inline const OUString& SwSetExpFieldType::GetSetRefName() const
    { return m_sName; }

class SW_DLLPUBLIC SwSetExpField final : public SwFormulaField
{
    double          m_fValueRLHidden; ///< SwValueField; hidden redlines
    OUString        msExpand;
    OUString        msExpandRLHidden; ///< hidden redlines
    OUString        maPText;
    bool            mbInput;
    sal_uInt16          mnSeqNo;
    sal_uInt16          mnSubType;
    SwFormatField * mpFormatField; /// pool item to which the SwSetExpField belongs

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;
    using SwFormulaField::GetValue; // hide it, don't use
    virtual void        SetValue(const double& rVal) override; // hide it

public:
    SwSetExpField(SwSetExpFieldType*, const OUString& rFormel, sal_uLong nFormat = 0);

    void SetFormatField(SwFormatField & rFormatField);
    SwFormatField* GetFormatField() { return mpFormatField;}

    double      GetValue(SwRootFrame const* pLayout) const;
    void        SetValue(const double& rVal, SwRootFrame const* pLayout);

    const OUString&      GetExpStr(SwRootFrame const* pLayout) const;

    void                 ChgExpStr(const OUString& rExpand, SwRootFrame const* pLayout);

    inline void                 SetPromptText(const OUString& rStr);
    inline const OUString&      GetPromptText() const;

    inline void                 SetInputFlag(bool bInp);
    inline bool                 GetInputFlag() const;

    virtual OUString            GetFieldName() const override;

    virtual sal_uInt16              GetSubType() const override;
    virtual void                SetSubType(sal_uInt16 nType) override;

    inline bool                 IsSequenceField() const;

    /// Logical number, sequence fields.
    void                 SetSeqNumber( sal_uInt16 n )    { mnSeqNo = n; }
    sal_uInt16           GetSeqNumber() const        { return mnSeqNo; }

    /// Query name only.
    virtual OUString       GetPar1()   const override;

    /// Query formula.
    virtual OUString       GetPar2()   const override;
    virtual void                SetPar2(const OUString& rStr) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline void  SwSetExpField::SetPromptText(const OUString& rStr)
    { maPText = rStr;        }

inline const OUString& SwSetExpField::GetPromptText() const
    { return maPText;        }

inline void SwSetExpField::SetInputFlag(bool bInp)
    { mbInput = bInp; }

inline bool SwSetExpField::GetInputFlag() const
    { return mbInput; }

inline bool SwSetExpField::IsSequenceField() const
    { return 0 != (nsSwGetSetExpType::GSE_SEQ & static_cast<SwSetExpFieldType*>(GetTyp())->GetType()); }

class SAL_DLLPUBLIC_RTTI SwInputFieldType final : public SwFieldType
{
    SwDoc* mpDoc;
public:
    SwInputFieldType( SwDoc* pDoc );

    virtual std::unique_ptr<SwFieldType> Copy() const override;

    SwDoc* GetDoc() const { return mpDoc; }
};

class SW_DLLPUBLIC SwInputField final : public SwField
{
    mutable OUString maContent;
    OUString maPText;
    OUString maHelp;
    OUString maToolTip;
    sal_uInt16 mnSubType;
    bool mbIsFormField;
    css::uno::Sequence<css::beans::PropertyValue> maGrabBag;

    SwFormatField* mpFormatField; // attribute to which the <SwInputField> belongs to

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    // Accessing Input Field's content
    const OUString& getContent() const { return maContent;}

public:
    /// Direct input via dialog; delete old value.
    SwInputField(
        SwInputFieldType* pFieldType,
        OUString aContent,
        OUString aPrompt,
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
    const css::uno::Sequence<css::beans::PropertyValue> & getGrabBagParams() const { return maGrabBag; }

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
    SwEditShell*                      mpSh;
    std::unique_ptr<SetGetExpFields>  mpSrtLst;
    o3tl::sorted_vector<const SwTextField*> maTmpLst;
};

 /// Implementation in tblcalc.cxx.
class SwTableFieldType final : public SwValueFieldType
{
public:
    SwTableFieldType(SwDoc* pDocPtr);
    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual void UpdateFields() override {};
};

class SwTableField final : public SwValueField, public SwTableFormula
{
    OUString      m_sExpand;
    sal_uInt16      m_nSubType;

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
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

    void                ChgExpStr(const OUString& rStr) { m_sExpand = rStr; }

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

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

#ifndef INCLUDED_SW_INC_SECTION_HXX
#define INCLUDED_SW_INC_SECTION_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/text/XTextSection.hpp>

#include <tools/ref.hxx>
#include <svl/hint.hxx>
#include <svl/listener.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/Metadatable.hxx>
#include <unotools/weakref.hxx>

#include "frmfmt.hxx"
#include <vector>

class SwSectionFormat;
class SwDoc;
class SwSection;
class SwSectionNode;
class SwTOXBase;
class SwServerObject;
class SwXTextSection;

typedef std::vector<SwSection*> SwSections;

enum class SectionType { Content,
                    ToxHeader,
                    ToxContent,
                    DdeLink    = static_cast<int>(sfx2::SvBaseLinkObjectType::ClientDde),
                    FileLink   = static_cast<int>(sfx2::SvBaseLinkObjectType::ClientFile)
                    };

enum class LinkCreateType
{
    NONE,            // Do nothing.
    Connect,         // Connect created link.
    Update           // Connect created link and update it.
};

class SW_DLLPUBLIC SwSectionData
{
private:
    SectionType m_eType;

    UIName m_sSectionName;
    OUString m_sCondition; ///< Hide condition
    OUString m_sLinkFileName;
    OUString m_sLinkFilePassword; // Must be changed to Sequence.
    css::uno::Sequence <sal_Int8> m_Password;
    sal_uInt16 m_nPage; // loaded meta page count for page anchored flys in master document

    /// It seems this flag caches the current final "hidden" state.
    bool m_bHiddenFlag          : 1;
    /// Flags that correspond to attributes in the format:
    /// may have different value than format attribute:
    /// format attr has value for this section, while flag is
    /// effectively ORed with parent sections!
    bool m_bProtectFlag         : 1; ///< protect flag is no longer inherited
    // Edit in readonly sections.
    bool m_bEditInReadonlyFlag  : 1;

    bool m_bHidden              : 1; ///< Section is hidden, unless condition evaluates `false'
    bool m_bCondHiddenFlag      : 1; ///< Hide condition evaluated `true'
    bool m_bConnectFlag         : 1; // Connected to server?

public:

    SwSectionData(SectionType const eType, UIName aName);
    explicit SwSectionData(SwSection const&);
    SwSectionData(SwSectionData const&);
    SwSectionData & operator=(SwSectionData const&);
    bool operator==(SwSectionData const&) const;

    const UIName& GetSectionName() const         { return m_sSectionName; }
    void SetSectionName(UIName const& rName){ m_sSectionName = rName; }
    SectionType GetType() const             { return m_eType; }
    void SetType(SectionType const eNew)    { m_eType = eNew; }

    bool IsHidden() const { return m_bHidden; }
    void SetHidden(bool const bFlag) { m_bHidden = bFlag; }

    bool IsHiddenFlag() const { return m_bHiddenFlag; }
    SAL_DLLPRIVATE void
        SetHiddenFlag(bool const bFlag) { m_bHiddenFlag = bFlag; }
    bool IsProtectFlag() const { return m_bProtectFlag; }
    SAL_DLLPRIVATE void
        SetProtectFlag(bool const bFlag) { m_bProtectFlag = bFlag; }
    bool IsEditInReadonlyFlag() const { return m_bEditInReadonlyFlag; }
    void SetEditInReadonlyFlag(bool const bFlag)
        { m_bEditInReadonlyFlag = bFlag; }

    void SetCondHidden(bool const bFlag) { m_bCondHiddenFlag = bFlag; }
    bool IsCondHidden() const { return m_bCondHiddenFlag; }

    const OUString& GetCondition() const           { return m_sCondition; }
    void SetCondition(OUString const& rNew) { m_sCondition = rNew; }

    const OUString& GetLinkFileName() const        { return m_sLinkFileName; }
    void SetLinkFileName(OUString const& rNew)
    {
        m_sLinkFileName = rNew;
    }

    const OUString& GetLinkFilePassword() const        { return m_sLinkFilePassword; }
    void SetLinkFilePassword(OUString const& rS){ m_sLinkFilePassword = rS; }

    css::uno::Sequence<sal_Int8> const& GetPassword() const
                                            { return m_Password; }
    void SetPassword(css::uno::Sequence<sal_Int8> const& rNew)
                                            { m_Password = rNew; }
    bool IsLinkType() const
    { return (SectionType::DdeLink == m_eType) || (SectionType::FileLink == m_eType); }

    bool IsConnectFlag() const                  { return m_bConnectFlag; }
    void SetConnectFlag(bool const bFlag){ m_bConnectFlag = bFlag; }

    sal_uInt16 GetPageNum() const         { return m_nPage; }
    void SetPageNum(sal_uInt16 nPageNum){ m_nPage = nPageNum; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SW_DLLPUBLIC SwSection
    : public SwClient
    , public SvtListener // needed for SwClientNotify to be called from SwSectionFormat
{
    // In order to correctly maintain the flag when creating/deleting frames.
    friend class SwSectionNode;
    // The "read CTOR" of SwSectionFrame have to change the Hiddenflag.
    friend class SwSectionFrame;

private:
    mutable SwSectionData m_Data;

    tools::SvRef<SwServerObject> m_RefObj; // Set if DataServer.
    tools::SvRef<sfx2::SvBaseLink> m_RefLink;

    SAL_DLLPRIVATE void ImplSetHiddenFlag(
            bool const bHidden, bool const bCondition);

protected:
    virtual void Notify(SfxHint const& rHint) override;
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:

    SwSection(SectionType const eType, UIName const& rName,
                SwSectionFormat & rFormat);
    virtual ~SwSection() override;

    bool DataEquals(SwSectionData const& rCmp) const;

    void SetSectionData(SwSectionData const& rData);

    const UIName& GetSectionName() const         { return m_Data.GetSectionName(); }
    void SetSectionName(UIName const& rName){ m_Data.SetSectionName(rName); }
    SectionType GetType() const             { return m_Data.GetType(); }
    void SetType(SectionType const eType)   { return m_Data.SetType(eType); }

    inline SwSectionFormat* GetFormat();
    inline SwSectionFormat const * GetFormat() const;

    // Set hidden/protected -> update the whole tree!
    // (Attributes/flags are set/get.)
    bool IsHidden()  const { return m_Data.IsHidden(); }
    void SetHidden (bool const bFlag = true);
    bool IsProtect() const;
    void SetProtect(bool const bFlag = true);
    bool IsEditInReadonly() const;
    void SetEditInReadonly(bool const bFlag = true);

    // Get internal flags (state including parents, not what is
    // currently set at section!).
    bool IsHiddenFlag()  const { return m_Data.IsHiddenFlag(); }
    bool IsProtectFlag() const { return m_Data.IsProtectFlag(); }
    bool IsEditInReadonlyFlag() const { return m_Data.IsEditInReadonlyFlag(); }

    void SetCondHidden(bool const bFlag);
    bool IsCondHidden() const { return m_Data.IsCondHidden(); }
    // Query (also for parents) if this section is to be hidden.
    bool CalcHiddenFlag() const;

    inline SwSection* GetParent() const;

    OUString const & GetCondition() const           { return m_Data.GetCondition(); }
    void SetCondition(OUString const& rNew) { m_Data.SetCondition(rNew); }

    OUString const & GetLinkFileName() const;
    void SetLinkFileName(OUString const& rNew);
    // Password of linked file (only valid during runtime!)
    OUString const & GetLinkFilePassword() const
        { return m_Data.GetLinkFilePassword(); }
    void SetLinkFilePassword(OUString const& rS)
        { m_Data.SetLinkFilePassword(rS); }

    // Get / set password of this section
    css::uno::Sequence<sal_Int8> const& GetPassword() const
                                            { return m_Data.GetPassword(); }

    sal_uInt16 GetPageNum() const
        { return m_Data.GetPageNum(); }
    void SetPageNum(sal_uInt16 nPage)
        { m_Data.SetPageNum(nPage); }

    // Data server methods.
    void SetRefObject( SwServerObject* pObj );
    const SwServerObject* GetObject() const {  return m_RefObj.get(); }
          SwServerObject* GetObject()       {  return m_RefObj.get(); }
    bool IsServer() const                   {  return m_RefObj.is(); }

    // Methods for linked ranges.
    SfxLinkUpdateMode GetUpdateType() const    { return m_RefLink->GetUpdateMode(); }
    void SetUpdateType(SfxLinkUpdateMode nType )
        { m_RefLink->SetUpdateMode(nType); }

    bool IsConnected() const        { return m_RefLink.is(); }
    void UpdateNow()                { m_RefLink->Update(); }
    void Disconnect()               { m_RefLink->Disconnect(); }

    const ::sfx2::SvBaseLink& GetBaseLink() const    { return *m_RefLink; }
          ::sfx2::SvBaseLink& GetBaseLink()          { return *m_RefLink; }

    void CreateLink( LinkCreateType eType );

    static void MakeChildLinksVisible( const SwSectionNode& rSectNd );

    bool IsLinkType() const { return m_Data.IsLinkType(); }

    // Flags for UI. Did connection work?
    bool IsConnectFlag() const      { return m_Data.IsConnectFlag(); }
    void SetConnectFlag(bool const bFlag = true)
                                    { m_Data.SetConnectFlag(bFlag); }

    // Return the TOX base class if the section is a TOX section
    const SwTOXBase* GetTOXBase() const;

    void BreakLink();

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

// #i117863#
class SwSectionFrameMoveAndDeleteHint final : public SfxHint
{
    public:
        SwSectionFrameMoveAndDeleteHint( const bool bSaveContent )
            : SfxHint( SfxHintId::SwSectionFrameMoveAndDelete )
            , mbSaveContent( bSaveContent )
        {}

        bool IsSaveContent() const
        {
            return mbSaveContent;
        }

    private:
        const bool mbSaveContent;
};

enum class SectionSort { Not, Pos };

class SW_DLLPUBLIC SwSectionFormat final
    : public SwFrameFormat
    , public ::sfx2::Metadatable
{
    friend class SwDoc;

    /** Why does this exist in addition to the m_wXObject in SwFrameFormat?
        in case of an index, both a SwXDocumentIndex and a SwXTextSection
        register at this SwSectionFormat, so we need to have two refs.
     */
    unotools::WeakReference<SwXTextSection> m_wXTextSection;

    SAL_DLLPRIVATE void UpdateParent();      // Parent has been changed.

    SwSectionFormat( SwFrameFormat* pDrvdFrame, SwDoc& rDoc );
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:
    virtual ~SwSectionFormat() override;

    // Deletes all Frames in aDepend (Frames are recognized via dynamic_cast).
    virtual void DelFrames() override;

    // Creates views.
    virtual void MakeFrames() override;

    // Get information from Format.
    virtual bool GetInfo( SwFindNearestNode& ) const override;

    virtual bool IsVisible() const override;

    SwSection* GetSection() const;
    inline SwSectionFormat* GetParent() const;
    inline SwSection* GetParentSection() const;

    //  All sections that are derived from this one:
    //  - sorted according to name or position or unsorted
    //  - all of them or only those that are in the normal Nodes-array.
    void GetChildSections( SwSections& rArr,
                            SectionSort eSort = SectionSort::Not,
                            bool bAllSections = true ) const;

    // Query whether section is in Nodes-array or in UndoNodes-array.
    bool IsInNodesArr() const;

          SwSectionNode* GetSectionNode();
    const SwSectionNode* GetSectionNode() const
        { return const_cast<SwSectionFormat *>(this)
                ->GetSectionNode(); }

    // Is section a valid one for global document?
    const SwSection* GetGlobalDocSection() const;

    SAL_DLLPRIVATE unotools::WeakReference<SwXTextSection> const& GetXTextSection() const
            { return m_wXTextSection; }
    SAL_DLLPRIVATE void SetXTextSection(rtl::Reference<SwXTextSection> const& xTextSection);

    // sfx2::Metadatable
    virtual ::sfx2::IXmlIdRegistry& GetRegistry() override;
    virtual bool IsInClipboard() const override;
    virtual bool IsInUndo() const override;
    virtual bool IsInContent() const override;
    virtual css::uno::Reference< css::rdf::XMetadatable > MakeUnoObject() override;
    virtual bool supportsFullDrawingLayerFillAttributeSet() const override;
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

};

SwSectionFormat* SwSection::GetFormat()
{
    return static_cast<SwSectionFormat*>(GetRegisteredIn());
}

SwSectionFormat const * SwSection::GetFormat() const
{
    return static_cast<SwSectionFormat const *>(GetRegisteredIn());
}

inline SwSection* SwSection::GetParent() const
{
    SwSectionFormat const * pFormat = GetFormat();
    SwSection* pRet = nullptr;
    if( pFormat )
        pRet = pFormat->GetParentSection();
    return pRet;
}

inline SwSectionFormat* SwSectionFormat::GetParent() const
{
    SwSectionFormat* pRet = nullptr;
    if( GetRegisteredIn() )
        pRet = const_cast<SwSectionFormat*>(dynamic_cast< const SwSectionFormat* >( GetRegisteredIn() ));
    return pRet;
}

inline SwSection* SwSectionFormat::GetParentSection() const
{
    SwSectionFormat* pParent = GetParent();
    SwSection* pRet = nullptr;
    if( pParent )
    {
        pRet = pParent->GetSection();
    }
    return pRet;
}

#endif /* _ INCLUDED_SW_INC_SECTION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

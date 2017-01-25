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

#include <tools/ref.hxx>
#include <svl/hint.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/Metadatable.hxx>

#include <frmfmt.hxx>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace text { class XTextSection; }
} } }

class SwSectionFormat;
class SwDoc;
class SwSection;
class SwSectionNode;
class SwTOXBase;
class SwServerObject;

typedef std::vector<SwSection*> SwSections;

enum SectionType { CONTENT_SECTION,
                    TOX_HEADER_SECTION,
                    TOX_CONTENT_SECTION,
                    DDE_LINK_SECTION    = OBJECT_CLIENT_DDE,
                    FILE_LINK_SECTION   = OBJECT_CLIENT_FILE
                    };

enum LinkCreateType
{
    CREATE_NONE,            // Do nothing.
    CREATE_CONNECT,         // Connect created link.
    CREATE_UPDATE           // Connect created link and update it.
};

class SW_DLLPUBLIC SwSectionData
{
private:
    SectionType m_eType;

    OUString m_sSectionName;
    OUString m_sCondition;
    OUString m_sLinkFileName;
    OUString m_sLinkFilePassword; // Must be changed to Sequence.
    css::uno::Sequence <sal_Int8> m_Password;

    /// It seems this flag caches the current final "hidden" state.
    bool m_bHiddenFlag          : 1;
    /// Flags that correspond to attributes in the format:
    /// may have different value than format attribute:
    /// format attr has value for this section, while flag is
    /// effectively ORed with parent sections!
    bool m_bProtectFlag         : 1;
    // Edit in readonly sections.
    bool m_bEditInReadonlyFlag  : 1;

    bool m_bHidden              : 1; // All paragraphs hidden?
    bool m_bCondHiddenFlag      : 1; // Hiddenflag for condition.
    bool m_bConnectFlag         : 1; // Connected to server?

public:

    SwSectionData(SectionType const eType, OUString const& rName);
    explicit SwSectionData(SwSection const&);
    SwSectionData(SwSectionData const&);
    SwSectionData & operator=(SwSectionData const&);
    bool operator==(SwSectionData const&) const;

    const OUString& GetSectionName() const         { return m_sSectionName; }
    void SetSectionName(OUString const& rName){ m_sSectionName = rName; }
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
    { return (DDE_LINK_SECTION == m_eType) || (FILE_LINK_SECTION == m_eType); }

    bool IsConnectFlag() const                  { return m_bConnectFlag; }
    void SetConnectFlag(bool const bFlag){ m_bConnectFlag = bFlag; }

    static OUString CollapseWhiteSpaces(const OUString& sName);
};

class SW_DLLPUBLIC SwSection
    : public SwClient
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
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew ) override;

public:

    SwSection(SectionType const eType, OUString const& rName,
                SwSectionFormat & rFormat);
    virtual ~SwSection() override;

    bool DataEquals(SwSectionData const& rCmp) const;

    void SetSectionData(SwSectionData const& rData);

    OUString GetSectionName() const         { return m_Data.GetSectionName(); }
    void SetSectionName(OUString const& rName){ m_Data.SetSectionName(rName); }
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

    OUString GetCondition() const           { return m_Data.GetCondition(); }
    void SetCondition(OUString const& rNew) { m_Data.SetCondition(rNew); }

    OUString GetLinkFileName() const;
    void SetLinkFileName(OUString const& rNew);
    // Password of linked file (only valid during runtime!)
    OUString GetLinkFilePassword() const
        { return m_Data.GetLinkFilePassword(); }
    void SetLinkFilePassword(OUString const& rS)
        { m_Data.SetLinkFilePassword(rS); }

    // Get / set password of this section
    css::uno::Sequence<sal_Int8> const& GetPassword() const
                                            { return m_Data.GetPassword(); }

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

};

// #i117863#
class SwSectionFrameMoveAndDeleteHint : public SfxHint
{
    public:
        SwSectionFrameMoveAndDeleteHint( const bool bSaveContent )
            : SfxHint( SfxHintId::Dying )
            , mbSaveContent( bSaveContent )
        {}

        bool IsSaveContent() const
        {
            return mbSaveContent;
        }

    private:
        const bool mbSaveContent;
};

enum SectionSort { SORTSECT_NOT, SORTSECT_NAME, SORTSECT_POS };

class SW_DLLPUBLIC SwSectionFormat
    : public SwFrameFormat
    , public ::sfx2::Metadatable
{
    friend class SwDoc;

    /** Why does this exist in addition to the m_wXObject in SwFrameFormat?
        in case of an index, both a SwXDocumentIndex and a SwXTextSection
        register at this SwSectionFormat, so we need to have two refs.
     */
    css::uno::WeakReference<css::text::XTextSection> m_wXTextSection;

    SAL_DLLPRIVATE void UpdateParent();      // Parent has been changed.

protected:
    SwSectionFormat( SwFrameFormat* pDrvdFrame, SwDoc *pDoc );
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew ) override;

public:
    virtual ~SwSectionFormat() override;

    // Deletes all Frames in aDepend (Frames are recognized via dynamic_cast).
    virtual void DelFrames() override;

    // Creates views.
    virtual void MakeFrames() override;

    // Get information from Format.
    virtual bool GetInfo( SfxPoolItem& ) const override;

    SwSection* GetSection() const;
    inline SwSectionFormat* GetParent() const;
    inline SwSection* GetParentSection() const;

    //  All sections that are derived from this one:
    //  - sorted according to name or position or unsorted
    //  - all of them or only those that are in the normal Nodes-array.
    void GetChildSections( SwSections& rArr,
                            SectionSort eSort = SORTSECT_NOT,
                            bool bAllSections = true ) const;

    // Query whether section is in Nodes-array or in UndoNodes-array.
    bool IsInNodesArr() const;

          SwSectionNode* GetSectionNode();
    const SwSectionNode* GetSectionNode() const
        { return const_cast<SwSectionFormat *>(this)
                ->GetSectionNode(); }

    // Is section a valid one for global document?
    const SwSection* GetGlobalDocSection() const;

    SAL_DLLPRIVATE css::uno::WeakReference<css::text::XTextSection> const& GetXTextSection() const
            { return m_wXTextSection; }
    SAL_DLLPRIVATE void SetXTextSection(css::uno::Reference<css::text::XTextSection> const& xTextSection)
            { m_wXTextSection = xTextSection; }

    // sfx2::Metadatable
    virtual ::sfx2::IXmlIdRegistry& GetRegistry() override;
    virtual bool IsInClipboard() const override;
    virtual bool IsInUndo() const override;
    virtual bool IsInContent() const override;
    virtual css::uno::Reference< css::rdf::XMetadatable > MakeUnoObject() override;
    void dumpAsXml(struct _xmlTextWriter* pWriter) const;

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

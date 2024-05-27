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

#ifndef INCLUDED_SW_INC_FMTFLD_HXX
#define INCLUDED_SW_INC_FMTFLD_HXX

#include <cppuhelper/weakref.hxx>
#include <svl/poolitem.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <com/sun/star/text/XTextField.hpp>
#include <unotools/weakref.hxx>

#include "swdllapi.h"
#include "swtypes.hxx"
#include "calbck.hxx"
#include "reffld.hxx"
#include "nodeoffset.hxx"

class SwField;
class SwTextField;
class SwView;
class SwFieldType;
class SwDDETable;
class SwFormatField;
class SwXTextField;
class IDocumentRedlineAccess;

namespace sw {
    struct FindFormatForFieldHint final : SfxHint {
        const SwField* m_pField;
        SwFormatField*& m_rpFormat;
        FindFormatForFieldHint(const SwField* pField, SwFormatField*& rpFormat)
            : SfxHint(SfxHintId::SwFindFormatForField), m_pField(pField), m_rpFormat(rpFormat) {};
    };
    struct FindFormatForPostItIdHint final : SfxHint {
        const sal_uInt32 m_nPostItId;
        SwFormatField*& m_rpFormat;
        FindFormatForPostItIdHint(const sal_uInt32 nPostItId, SwFormatField*& rpFormat)
            : SfxHint(SfxHintId::SwFindFormatForPostItId), m_nPostItId(nPostItId), m_rpFormat(rpFormat) {};
    };
    struct CollectPostItsHint final : SfxHint {
        std::vector<SwFormatField*>& m_rvFormatFields;
        IDocumentRedlineAccess const& m_rIDRA;
        const bool m_bHideRedlines;
        CollectPostItsHint(std::vector<SwFormatField*>& rvFormatFields, IDocumentRedlineAccess const& rIDRA, bool bHideRedlines)
            : SfxHint(SfxHintId::SwCollectPostIts),
              m_rvFormatFields(rvFormatFields), m_rIDRA(rIDRA), m_bHideRedlines(bHideRedlines) {};
    };
    struct HasHiddenInformationNotesHint final : SfxHint {
        bool& m_rbHasHiddenInformationNotes;
        HasHiddenInformationNotesHint(bool& rbHasHiddenInformationNotes)
            : SfxHint(SfxHintId::SwHasHiddenInformationNotes),
              m_rbHasHiddenInformationNotes(rbHasHiddenInformationNotes) {};
    };
    struct GatherNodeIndexHint final : SfxHint {
        std::vector<SwNodeOffset>& m_rvNodeIndex;
        GatherNodeIndexHint(std::vector<SwNodeOffset>& rvNodeIndex)
            : SfxHint(SfxHintId::SwGatherNodeIndex),
              m_rvNodeIndex(rvNodeIndex) {};
    };
    struct GatherRefFieldsHint final : SfxHint {
        std::vector<SwGetRefField*>& m_rvRFields;
        const sal_uInt16 m_nType;
        GatherRefFieldsHint(std::vector<SwGetRefField*>& rvRFields, const sal_uInt16 nType)
            : SfxHint(SfxHintId::SwGatherRefFields),
              m_rvRFields(rvRFields), m_nType(nType) {};
    };
    struct GatherFieldsHint final : SfxHint {
        const bool m_bCollectOnlyInDocNodes;
        std::vector<SwFormatField*>& m_rvFields;
        GatherFieldsHint(std::vector<SwFormatField*>& rvFields, bool bCollectOnlyInDocNodes = true)
            : SfxHint(SfxHintId::SwGatherFields),
              m_bCollectOnlyInDocNodes(bCollectOnlyInDocNodes), m_rvFields(rvFields) {};
    };
    struct GatherDdeTablesHint final : SfxHint {
        std::vector<SwDDETable*>& m_rvTables;
        GatherDdeTablesHint(std::vector<SwDDETable*>& rvTables) : m_rvTables(rvTables) {};
    };
}


/// SfxPoolItem subclass that is a wrapper around an SwField, i.e. one inserted field into paragraph
/// text. Typically owned by an SwTextField.
class SW_DLLPUBLIC SwFormatField final
    : public SfxPoolItem
    , public sw::BroadcastingModify
    , public SfxBroadcaster
{
    friend SwFormatField* createSwFormatFieldForItemInfoPackage(sal_uInt16);
    // friend class ItemInfoPackageSwAttributes;
    friend void InitCore();
    SwFormatField( sal_uInt16 nWhich ); // for default-Attribute

    unotools::WeakReference<SwXTextField> m_wXTextField;

    std::unique_ptr<SwField> mpField;
    SwTextField* mpTextField; // the TextAttribute

    virtual void SwClientNotify( const SwModify& rModify, const SfxHint& rHint ) override;

public:

    /// Single argument constructors shall be explicit.
    explicit SwFormatField( const SwField &rField );

    SwFormatField( const SwFormatField& rAttr );

    virtual ~SwFormatField() override;

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SwFormatField*  Clone( SfxItemPool* pPool = nullptr ) const override;

    void InvalidateField();

    const SwField* GetField() const
    {
        return mpField.get();
    }
    SwField* GetField()
    {
        return mpField.get();
    }

    /**
       Sets current field.

       @param pField          new field

       @attention The current field will be destroyed before setting the new field.
     */
    void SetField( std::unique_ptr<SwField> pField );

    const SwTextField* GetTextField() const
    {
        return mpTextField;
    }
    SwTextField* GetTextField()
    {
        return mpTextField;
    }
    void SetTextField( SwTextField& rTextField );
    void ClearTextField();

    void RegisterToFieldType( SwFieldType& );
    bool IsFieldInDoc() const;
    bool IsProtect() const;

    SAL_DLLPRIVATE unotools::WeakReference<SwXTextField> const& GetXTextField() const
            { return m_wXTextField; }
    SAL_DLLPRIVATE void SetXTextField(rtl::Reference<SwXTextField> const& xTextField);

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    void ForceUpdateTextNode();
    void UpdateTextNode(const SfxHint& rHint);
    void UpdateDocPos(const SwTwips nDocPos);
};

enum class SwFormatFieldHintWhich
{
    INSERTED   = 1,
    REMOVED    = 2,
    FOCUS      = 3,
    CHANGED    = 4,
    RESOLVED   = 5,
    REDLINED_DELETION = 6
};

/// This SfxHint subclass is produced by code that changes the doc model (e.g. SwNodes::ChgNode()),
/// the typical which id is SwFormatFieldHintWhich::INSERTED or SwFormatFieldHintWhich::REMOVED.
///
/// The consumer of this hint is SwPostItMgr::Notify(), i.e. the sidebar on the right hand side of
/// the Writer edit window that hosts the comments as post-its.
class SW_DLLPUBLIC SwFormatFieldHint final : public SfxHint
{
    const SwFormatField*   m_pField;
    SwFormatFieldHintWhich m_nWhich;
    const SwView*     m_pView;

public:
    SwFormatFieldHint( const SwFormatField* pField, SwFormatFieldHintWhich nWhich, const SwView* pView = nullptr)
        : SfxHint(SfxHintId::SwFormatField)
        , m_pField(pField)
        , m_nWhich(nWhich)
        , m_pView(pView)
    {}

    const SwFormatField* GetField() const { return m_pField; }
    SwFormatFieldHintWhich Which() const { return m_nWhich; }
    const SwView* GetView() const { return m_pView; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

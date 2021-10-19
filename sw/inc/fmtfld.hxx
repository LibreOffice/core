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

#include "swdllapi.h"
#include "calbck.hxx"
#include "reffld.hxx"
#include "nodeoffset.hxx"

class SwField;
class SwTextField;
class SwView;
class SwFieldType;
class SwFormatField;
class IDocumentRedlineAccess;
namespace com::sun::star::text { class XTextField; }

namespace sw {
    struct FindFormatForFieldHint final : SfxHint {
        const SwField* m_pField;
        SwFormatField*& m_rpFormat;
        FindFormatForFieldHint(const SwField* pField, SwFormatField*& rpFormat) : m_pField(pField), m_rpFormat(rpFormat) {};
    };
    struct FindFormatForPostItIdHint final : SfxHint {
        const sal_uInt32 m_nPostItId;
        SwFormatField*& m_rpFormat;
        FindFormatForPostItIdHint(const sal_uInt32 nPostItId, SwFormatField*& rpFormat) : m_nPostItId(nPostItId), m_rpFormat(rpFormat) {};
    };
    struct CollectPostItsHint final : SfxHint {
        std::vector<SwFormatField*>& m_rvFormatFields;
        IDocumentRedlineAccess const& m_rIDRA;
        const bool m_bHideRedlines;
        CollectPostItsHint(std::vector<SwFormatField*>& rvFormatFields, IDocumentRedlineAccess const& rIDRA, bool bHideRedlines) : m_rvFormatFields(rvFormatFields), m_rIDRA(rIDRA), m_bHideRedlines(bHideRedlines) {};
    };
    struct HasHiddenInformationNotesHint final : SfxHint {
        bool& m_rbHasHiddenInformationNotes;
        HasHiddenInformationNotesHint(bool& rbHasHiddenInformationNotes) : m_rbHasHiddenInformationNotes(rbHasHiddenInformationNotes) {};
    };
    struct GatherNodeIndexHint final : SfxHint {
        std::vector<SwNodeOffset>& m_rvNodeIndex;
        GatherNodeIndexHint(std::vector<SwNodeOffset>& rvNodeIndex) : m_rvNodeIndex(rvNodeIndex) {};
    };
    struct GatherRefFieldsHint final : SfxHint {
        std::vector<SwGetRefField*>& m_rvRFields;
        const sal_uInt16 m_nType;
        GatherRefFieldsHint(std::vector<SwGetRefField*>& rvRFields, const sal_uInt16 nType) : m_rvRFields(rvRFields), m_nType(nType) {};
    };
    struct GatherFieldsHint final : SfxHint {
        const bool m_bCollectOnlyInDocNodes;
        std::vector<SwFormatField*>& m_rvFields;
        GatherFieldsHint(std::vector<SwFormatField*>& rvFields, bool bCollectOnlyInDocNodes = true) : m_bCollectOnlyInDocNodes(bCollectOnlyInDocNodes), m_rvFields(rvFields) {};
    };
}


// ATT_FLD
class SW_DLLPUBLIC SwFormatField final
    : public SfxPoolItem
    , public sw::BroadcastingModify
    , public SfxBroadcaster
{
    friend void InitCore();
    SwFormatField( sal_uInt16 nWhich ); // for default-Attribute

    css::uno::WeakReference<css::text::XTextField> m_wXTextField;

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

    virtual bool GetInfo( SfxPoolItem& rInfo ) const override;

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

    SAL_DLLPRIVATE css::uno::WeakReference<css::text::XTextField> const& GetXTextField() const
            { return m_wXTextField; }
    SAL_DLLPRIVATE void SetXTextField(css::uno::Reference<css::text::XTextField> const& xTextField)
            { m_wXTextField = xTextField; }
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    void UpdateTextNode(const SfxPoolItem* pOld, const SfxPoolItem* pNew);
};

enum class SwFormatFieldHintWhich
{
    INSERTED   = 1,
    REMOVED    = 2,
    FOCUS      = 3,
    CHANGED    = 4,
    LANGUAGE   = 5,
    RESOLVED   = 6
};

class SW_DLLPUBLIC SwFormatFieldHint final : public SfxHint
{
    const SwFormatField*   m_pField;
    SwFormatFieldHintWhich m_nWhich;
    const SwView*     m_pView;

public:
    SwFormatFieldHint( const SwFormatField* pField, SwFormatFieldHintWhich nWhich, const SwView* pView = nullptr)
        : m_pField(pField)
        , m_nWhich(nWhich)
        , m_pView(pView)
    {}

    const SwFormatField* GetField() const { return m_pField; }
    SwFormatFieldHintWhich Which() const { return m_nWhich; }
    const SwView* GetView() const { return m_pView; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

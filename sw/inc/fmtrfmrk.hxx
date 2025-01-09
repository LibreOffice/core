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
#ifndef INCLUDED_SW_INC_FMTRFMRK_HXX
#define INCLUDED_SW_INC_FMTRFMRK_HXX

#include <rtl/ustring.hxx>
#include <cppuhelper/weakref.hxx>
#include <svl/poolitem.hxx>
#include <unotools/weakref.hxx>
#include <com/sun/star/text/XTextContent.hpp>

#include "calbck.hxx"

class SwTextRefMark;
class SwXReferenceMark;

/// SfxPoolItem subclass that represents a reference mark (a position in the document with a name,
/// which has a start and may or may not have an end).
///
/// This can be used as a character format in the hints array of a text node, it's owned by an
/// SwTextRefMark.
///
/// It's Insert -> Cross-reference -> Cross-references -> set reference on the UI.
class SAL_DLLPUBLIC_RTTI SwFormatRefMark final : public SfxPoolItem
{
    friend class SwTextRefMark;
    SwTextRefMark* m_pTextAttr;

    SwFormatRefMark& operator=(const SwFormatRefMark& rRefMark) = delete;
    OUString m_aRefName;

    unotools::WeakReference<SwXReferenceMark> m_wXReferenceMark;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatRefMark)
    SwFormatRefMark( OUString aText );
    SwFormatRefMark( const SwFormatRefMark& rRefMark );
    virtual ~SwFormatRefMark( ) override;

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SwFormatRefMark* Clone( SfxItemPool* pPool = nullptr ) const override;

    void InvalidateRefMark();

    const SwTextRefMark *GetTextRefMark() const   { return m_pTextAttr; }

    OUString &GetRefName()       { return m_aRefName; }
    const OUString &GetRefName() const { return m_aRefName; }

    unotools::WeakReference<SwXReferenceMark> const& GetXRefMark() const
        { return m_wXReferenceMark; }
    void SetXRefMark(rtl::Reference<SwXReferenceMark> const& xMark);
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

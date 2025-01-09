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
#ifndef INCLUDED_SFX2_INC_SORGITM_HXX
#define INCLUDED_SFX2_INC_SORGITM_HXX

#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>

// class SfxScriptOrganizerItem ---------------------------------------------

class SfxScriptOrganizerItem final : public SfxPoolItem
{
private:
    OUString aLanguage;

public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SfxScriptOrganizerItem)
    SfxScriptOrganizerItem();

    virtual SfxScriptOrganizerItem* Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool          operator==( const SfxPoolItem& ) const override;
    virtual bool supportsHashCode() const override { return true; }
    virtual size_t hashCode() const override { return aLanguage.hashCode(); }
    virtual bool          QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool          PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual bool GetPresentation(SfxItemPresentation,
                                 MapUnit, MapUnit,
                                 OUString & rText,
                                 const IntlWrapper&) const override;
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    const OUString&       getLanguage() const { return aLanguage; };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

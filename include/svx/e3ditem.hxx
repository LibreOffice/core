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

#ifndef INCLUDED_SVX_E3DITEM_HXX
#define INCLUDED_SVX_E3DITEM_HXX

#include <config_options.h>
#include <svl/poolitem.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <svx/svxdllapi.h>

class SAL_WARN_UNUSED UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SvxB3DVectorItem final : public SfxPoolItem
{
    basegfx::B3DVector  m_aVal;

public:
                            DECLARE_ITEM_TYPE_FUNCTION(SvxB3DVectorItem)
                            SvxB3DVectorItem( TypedWhichId<SvxB3DVectorItem> nWhich, const basegfx::B3DVector& rVal );
                            SvxB3DVectorItem( const SvxB3DVectorItem& );
                            virtual ~SvxB3DVectorItem() override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SvxB3DVectorItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const basegfx::B3DVector&           GetValue() const { return m_aVal; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

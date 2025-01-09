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

#ifndef INCLUDED_SVX_XCOLIT_HXX
#define INCLUDED_SVX_XCOLIT_HXX

#include <tools/color.hxx>
#include <docmodel/color/ComplexColor.hxx>

#include <svx/svxdllapi.h>
#include <svx/xit.hxx>

class SVXCORE_DLLPUBLIC XColorItem : public NameOrIndex
{
    Color   m_aColor;
    model::ComplexColor maComplexColor;

public:
            static SfxPoolItem* CreateDefault();
            DECLARE_ITEM_TYPE_FUNCTION(XColorItem)
            XColorItem() : NameOrIndex() {}
            XColorItem(TypedWhichId<XColorItem> nWhich, sal_Int32 nIndex, const Color& rTheColor);
            XColorItem(TypedWhichId<XColorItem> nWhich, const Color& rTheColor);
            XColorItem(TypedWhichId<XColorItem> nWhich, const OUString& rName, const Color& rTheColor);
            XColorItem(const XColorItem& rItem);

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual bool            operator==(const SfxPoolItem& rItem) const override;
    virtual XColorItem* Clone(SfxItemPool* pPool = nullptr) const override;

    const Color&    GetColorValue() const;
    void            SetColorValue(const Color& rNew) { m_aColor = rNew; Detach(); }

    void setComplexColor(model::ComplexColor const& rComplexColor) { maComplexColor = rComplexColor; }
    const model::ComplexColor& getComplexColor() const { return maComplexColor; }
    model::ComplexColor& getComplexColor() { return maComplexColor; }

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <com/sun/star/awt/Gradient2.hpp>
#include <svx/xflftrit.hxx>

#include <svx/svdmodel.hxx>
#include <svx/unofill.hxx>
#include <svx/unomid.hxx>
#include "UnoNameItemTable.hxx"

using namespace ::com::sun::star;
using namespace ::cppu;

namespace
{
class SvxUnoTransGradientTable : public SvxUnoNameItemTable
{
public:
    explicit SvxUnoTransGradientTable(SdrModel* pModel) noexcept;

    virtual NameOrIndex* createItem() const override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
};
}

SvxUnoTransGradientTable::SvxUnoTransGradientTable(SdrModel* pModel) noexcept
    : SvxUnoNameItemTable(pModel, XATTR_FILLFLOATTRANSPARENCE, MID_FILLGRADIENT)
{
}

OUString SAL_CALL SvxUnoTransGradientTable::getImplementationName()
{
    return u"SvxUnoTransGradientTable"_ustr;
}

uno::Sequence<OUString> SAL_CALL SvxUnoTransGradientTable::getSupportedServiceNames()
{
    return { u"com.sun.star.drawing.TransparencyGradientTable"_ustr };
}

NameOrIndex* SvxUnoTransGradientTable::createItem() const
{
    XFillFloatTransparenceItem* pNewItem = new XFillFloatTransparenceItem();
    pNewItem->SetEnabled(true);
    return pNewItem;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoTransGradientTable::getElementType()
{
    // tdf#158421 use newer extended type for the list
    return cppu::UnoType<awt::Gradient2>::get();
}

/**
 * Create a hatchtable
 */
uno::Reference<uno::XInterface> SvxUnoTransGradientTable_createInstance(SdrModel* pModel)
{
    return *new SvxUnoTransGradientTable(pModel);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

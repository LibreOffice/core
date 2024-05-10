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

#include <com/sun/star/drawing/LineDash.hpp>

#include "UnoNameItemTable.hxx"
#include <svx/xlndsit.hxx>
#include <svx/unomid.hxx>

#include <svx/svdmodel.hxx>
#include <svx/unofill.hxx>

using namespace ::com::sun::star;
using namespace ::cppu;

namespace
{
class SvxUnoDashTable : public SvxUnoNameItemTable
{
public:
    explicit SvxUnoDashTable(SdrModel* pModel) noexcept;

    virtual NameOrIndex* createItem() const override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
};
}

SvxUnoDashTable::SvxUnoDashTable(SdrModel* pModel) noexcept
    : SvxUnoNameItemTable(pModel, XATTR_LINEDASH, MID_LINEDASH)
{
}

OUString SAL_CALL SvxUnoDashTable::getImplementationName() { return u"SvxUnoDashTable"_ustr; }

uno::Sequence<OUString> SAL_CALL SvxUnoDashTable::getSupportedServiceNames()
{
    return { u"com.sun.star.drawing.DashTable"_ustr };
}

NameOrIndex* SvxUnoDashTable::createItem() const
{
    XLineDashItem* pNewItem = new XLineDashItem();
    pNewItem->SetWhich(XATTR_LINEDASH); // set which id for pooling
    return pNewItem;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoDashTable::getElementType()
{
    return cppu::UnoType<drawing::LineDash>::get();
}

/**
 * Create a gradienttable
 */
uno::Reference<uno::XInterface> SvxUnoDashTable_createInstance(SdrModel* pModel)
{
    return *new SvxUnoDashTable(pModel);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

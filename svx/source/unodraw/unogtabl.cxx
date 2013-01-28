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

#include <com/sun/star/awt/Gradient.hpp>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include "UnoNameItemTable.hxx"

#include <svx/xgrad.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xdef.hxx>
#include <svx/xflgrit.hxx>
#include "svx/unofill.hxx"
#include <svx/unomid.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoGradientTable : public SvxUnoNameItemTable
{
    bool m_bTextWhich;
public:
    SvxUnoGradientTable( SdrModel* pModel ) throw();
    SvxUnoGradientTable( SdrModel* pModel, sal_uInt16 nWhich ) throw();
    virtual ~SvxUnoGradientTable() throw();

    virtual NameOrIndex* createItem() const throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
};

SvxUnoGradientTable::SvxUnoGradientTable( SdrModel* pModel ) throw()
    : SvxUnoNameItemTable( pModel, XATTR_FILLGRADIENT, MID_FILLGRADIENT ), m_bTextWhich(false)
{
}

SvxUnoGradientTable::SvxUnoGradientTable( SdrModel* pModel, sal_uInt16 nWhich ) throw()
    : SvxUnoNameItemTable( pModel, nWhich, MID_FILLGRADIENT ), m_bTextWhich(true)
{
}

SvxUnoGradientTable::~SvxUnoGradientTable() throw()
{
}

OUString SAL_CALL SvxUnoGradientTable::getImplementationName() throw( uno::RuntimeException )
{
    if (m_bTextWhich)
        return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoTextGradientTable") );
    else
        return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoGradientTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoGradientTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    if (m_bTextWhich)
        aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.GradientTable" ));
    else
        aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GradientTable" ));
    return aSNS;
}


// XNameContainer
NameOrIndex* SvxUnoGradientTable::createItem() const throw()
{
    return new XFillGradientItem();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoGradientTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const struct awt::Gradient*)0);
}

/**
 * Create a gradienttable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoGradientTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoGradientTable(pModel);
}

uno::Reference< uno::XInterface > SAL_CALL SvxUnoTextGradientTable_createInstance( SdrModel* pModel, sal_uInt16 nWhich )
{
    return *new SvxUnoGradientTable(pModel, nWhich);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

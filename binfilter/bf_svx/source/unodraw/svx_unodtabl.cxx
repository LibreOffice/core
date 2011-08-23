/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/drawing/LineDash.hpp>



#include <vector>

#include "UnoNameItemTable.hxx"

#include "xlndsit.hxx"


#include "xdef.hxx"

namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoDashTable : public SvxUnoNameItemTable
{
public:
    SvxUnoDashTable( SdrModel* pModel ) throw();
    virtual	~SvxUnoDashTable() throw();

    virtual NameOrIndex* createItem() const throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
};

SvxUnoDashTable::SvxUnoDashTable( SdrModel* pModel ) throw()
: SvxUnoNameItemTable( pModel, XATTR_LINEDASH )
{
}

SvxUnoDashTable::~SvxUnoDashTable() throw()
{
}

OUString SAL_CALL SvxUnoDashTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoDashTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoDashTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DashTable" ));
    return aSNS;
}

NameOrIndex* SvxUnoDashTable::createItem() const throw()
{
    XLineDashItem* pNewItem = new XLineDashItem();
    pNewItem->SetWhich( XATTR_LINEDASH ); // set which id for pooling
    return pNewItem;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoDashTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const struct drawing::LineDash*)0);
}

/**
 * Create a gradienttable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoDashTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoDashTable(pModel);
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif



#ifndef _SVX_UNONAMEITEMTABLE_HXX_
#include "UnoNameItemTable.hxx"
#endif

#include "xdef.hxx"
#include "xflhtit.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoHatchTable : public SvxUnoNameItemTable
{
public:
    SvxUnoHatchTable( SdrModel* pModel ) throw();
    virtual	~SvxUnoHatchTable() throw();

    virtual NameOrIndex* createItem() const throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
};

SvxUnoHatchTable::SvxUnoHatchTable( SdrModel* pModel ) throw()
: SvxUnoNameItemTable( pModel, XATTR_FILLHATCH )
{
}

SvxUnoHatchTable::~SvxUnoHatchTable() throw()
{
}

OUString SAL_CALL SvxUnoHatchTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoHatchTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoHatchTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.HatchTable" ));
    return aSNS;
}

NameOrIndex* SvxUnoHatchTable::createItem() const throw()
{
    return new XFillHatchItem();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoHatchTable::getElementType(  )
    throw( uno::RuntimeException )
{
    return ::getCppuType((const struct drawing::Hatch*)0);
}

/**
 * Create a hatchtable
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoHatchTable_createInstance( SdrModel* pModel )
{
    return *new SvxUnoHatchTable(pModel);
}



}

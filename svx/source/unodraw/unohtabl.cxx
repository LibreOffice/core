/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <com/sun/star/drawing/Hatch.hpp>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include "UnoNameItemTable.hxx"

#include <svx/xhatch.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xdef.hxx>
#include <svx/xflhtit.hxx>
#include <svx/unomid.hxx>
#include "svx/unofill.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoHatchTable : public SvxUnoNameItemTable
{
public:
    SvxUnoHatchTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoHatchTable() throw();

    virtual NameOrIndex* createItem() const throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
};

SvxUnoHatchTable::SvxUnoHatchTable( SdrModel* pModel ) throw()
: SvxUnoNameItemTable( pModel, XATTR_FILLHATCH, MID_FILLHATCH )
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




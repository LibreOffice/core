/*************************************************************************
 *
 *  $RCSfile: unodtabl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 09:45:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef _SVX_UNONAMEITEMTABLE_HXX_
#include "UnoNameItemTable.hxx"
#endif

#ifndef _SVX_XLNDSIT_HXX
#include "xlndsit.hxx"
#endif

#include "xdash.hxx"
#include "svdmodel.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoDashTable : public SvxUnoNameItemTable
{
public:
    SvxUnoDashTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoDashTable() throw();

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




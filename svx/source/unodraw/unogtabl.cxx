/*************************************************************************
 *
 *  $RCSfile: unogtabl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-26 15:09:58 $
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

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SVX_UNONAMEITEMTABLE_HXX_
#include "UnoNameItemTable.hxx"
#endif

#include "xgrad.hxx"
#include "svdmodel.hxx"
#include "xdef.hxx"
#include "xflgrit.hxx"
#include "unomid.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoGradientTable : public SvxUnoNameItemTable
{
public:
    SvxUnoGradientTable( SdrModel* pModel ) throw();
    virtual ~SvxUnoGradientTable() throw();

    virtual NameOrIndex* createItem() const throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw( uno::RuntimeException );
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames(  ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
};

SvxUnoGradientTable::SvxUnoGradientTable( SdrModel* pModel ) throw()
    : SvxUnoNameItemTable( pModel, XATTR_FILLGRADIENT, MID_FILLGRADIENT )
{
}

SvxUnoGradientTable::~SvxUnoGradientTable() throw()
{
}

OUString SAL_CALL SvxUnoGradientTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxUnoGradientTable") );
}

uno::Sequence< OUString > SAL_CALL SvxUnoGradientTable::getSupportedServiceNames(  )
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
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




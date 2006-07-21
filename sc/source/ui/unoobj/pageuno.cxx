/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pageuno.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:44:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "pageuno.hxx"
#include "shapeuno.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------------------------

ScPageObj::ScPageObj( SdrPage* pPage ) :
    SvxFmDrawPage( pPage )
{
}

ScPageObj::~ScPageObj() throw()
{
}

uno::Reference<drawing::XShape > ScPageObj::_CreateShape( SdrObject *pObj ) const throw()
{
    uno::Reference<drawing::XShape> xShape(SvxFmDrawPage::_CreateShape( pObj ));

    new ScShapeObj( xShape );       // aggregates object and modifies xShape

    return xShape;
}

::rtl::OUString SAL_CALL ScPageObj::getImplementationName()
                                throw(uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScPageObj"));
}

sal_Bool SAL_CALL ScPageObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr(rServiceName);
    return aServiceStr.EqualsAscii( "com.sun.star.sheet.SpreadsheetDrawPage" );
}

uno::Sequence<rtl::OUString> SAL_CALL ScPageObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDrawPage"));
    return aRet;
}

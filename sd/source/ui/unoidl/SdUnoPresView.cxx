/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdUnoPresView.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:46:33 $
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

#include "SdUnoPresView.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {


SdUnoPresView::SdUnoPresView (
    ViewShellBase& rBase,
    ViewShell& rViewShell,
    View& rView) throw()
    : SdUnoDrawView (rBase, rViewShell, rView)
{
}




SdUnoPresView::~SdUnoPresView() throw()
{
}




// XTypeProvider

IMPLEMENT_GET_IMPLEMENTATION_ID(SdUnoPresView);




// XServiceInfo


OUString SAL_CALL SdUnoPresView::getImplementationName (void)
    throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdUnoPresView" ) );
}




//----------------------------------------------------------------------
//------ The Properties of this implementation -------------------------
//----------------------------------------------------------------------

// Id must be the index into the array
enum properties
{
    PROPERTY_CURRENTPAGE = 0,

    PROPERTY_COUNT
};

/**
 * All Properties of this implementation. Must be sorted by name.
 */
static beans::Property * getBasicProps()
{
    static beans::Property *pTable = 0;

    if( ! pTable )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pTable )
        {

            static beans::Property aBasicProps[PROPERTY_COUNT] =
            {
                beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("CurrentPage") ),        PROPERTY_CURRENTPAGE,   ::getCppuType((const Reference< drawing::XDrawPage > *)0), beans::PropertyAttribute::BOUND ),
            };
            pTable = aBasicProps;
        }
    }
    return pTable;
}

//----------------------------------------------------------------------
//------ XPropertySet & OPropertySetHelper -----------------------------
//----------------------------------------------------------------------

/**
 * Create a table that map names to index values.
 */
IPropertyArrayHelper & SdUnoPresView::getInfoHelper()
{
    OGuard aGuard( Application::GetSolarMutex() );

    static OPropertyArrayHelper aInfo( getBasicProps(), PROPERTY_COUNT );
    return aInfo;
}

//----------------------------------------------------------------------

Reference < beans::XPropertySetInfo >  SdUnoPresView::getPropertySetInfo() throw ( ::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    static Reference < beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

} // end of namespace sd

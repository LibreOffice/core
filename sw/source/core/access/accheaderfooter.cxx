 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accheaderfooter.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:14:32 $
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
#include "precompiled_sw.hxx"


#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _HFFRM_HXX
#include <hffrm.hxx>
#endif

#ifndef _ACCHEADERFOOTER_HXX
#include "accheaderfooter.hxx"
#endif
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

const sal_Char sServiceNameHeader[] = "com.sun.star.text.AccessibleHeaderView";
const sal_Char sServiceNameFooter[] = "com.sun.star.text.AccessibleFooterView";
const sal_Char sImplementationNameHeader[] = "com.sun.star.comp.Writer.SwAccessibleHeaderView";
const sal_Char sImplementationNameFooter[] = "com.sun.star.comp.Writer.SwAccessibleFooterView";

SwAccessibleHeaderFooter::SwAccessibleHeaderFooter(
        SwAccessibleMap* pInitMap,
        const SwHeaderFrm* pHdFrm    ) :
    SwAccessibleContext( pInitMap, AccessibleRole::HEADER, pHdFrm )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    OUString sArg( OUString::valueOf( (sal_Int32)pHdFrm->GetPhyPageNum() ) );
    SetName( GetResource( STR_ACCESS_HEADER_NAME, &sArg ) );
}

SwAccessibleHeaderFooter::SwAccessibleHeaderFooter(
        SwAccessibleMap* pInitMap,
        const SwFooterFrm* pFtFrm    ) :
    SwAccessibleContext( pInitMap, AccessibleRole::FOOTER, pFtFrm )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    OUString sArg( OUString::valueOf( (sal_Int32)pFtFrm->GetPhyPageNum() ) );
    SetName( GetResource( STR_ACCESS_FOOTER_NAME, &sArg ) );
}

SwAccessibleHeaderFooter::~SwAccessibleHeaderFooter()
{
}

OUString SAL_CALL SwAccessibleHeaderFooter::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    sal_uInt16 nResId = AccessibleRole::HEADER == GetRole()
        ? STR_ACCESS_HEADER_DESC
        : STR_ACCESS_FOOTER_DESC ;

    OUString sArg( GetFormattedPageNumber() );

    return GetResource( nResId, &sArg );
}

OUString SAL_CALL SwAccessibleHeaderFooter::getImplementationName()
        throw( RuntimeException )
{
    if( AccessibleRole::HEADER == GetRole() )
        return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationNameHeader));
    else
        return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationNameFooter));
}

sal_Bool SAL_CALL SwAccessibleHeaderFooter::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    if( sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                       sizeof(sAccessibleServiceName)-1 ) )
        return sal_True;
    else if( AccessibleRole::HEADER == GetRole() )
        return sTestServiceName.equalsAsciiL( sServiceNameHeader, sizeof(sServiceNameHeader)-1 );
    else
        return sTestServiceName.equalsAsciiL( sServiceNameFooter, sizeof(sServiceNameFooter)-1 );

}

Sequence< OUString > SAL_CALL SwAccessibleHeaderFooter::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    if( AccessibleRole::HEADER == GetRole() )
        pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceNameHeader) );
    else
        pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceNameFooter) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleHeaderFooter::getImplementationId()
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

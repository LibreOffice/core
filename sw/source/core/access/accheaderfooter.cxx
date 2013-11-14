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
#include "precompiled_sw.hxx"


#include <vos/mutex.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <rtl/uuid.h>
#include <vcl/svapp.hxx>
#include <hffrm.hxx>
#include "accheaderfooter.hxx"
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

//IAccessibility2 Implementation 2009-----
sal_Int32 SAL_CALL SwAccessibleHeaderFooter::getBackground()
        throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XAccessible > xParent =  getAccessibleParent();
    if (xParent.is())
    {
        Reference< XAccessibleComponent > xAccContext (xParent,UNO_QUERY);
        if(xAccContext.is())
        {
            return xAccContext->getBackground();
        }
    }
    return SwAccessibleContext::getBackground();
}
//-----IAccessibility2 Implementation 2009

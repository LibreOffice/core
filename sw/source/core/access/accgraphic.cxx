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


#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <rtl/uuid.h>
#include <flyfrm.hxx>
#include "accgraphic.hxx"

using namespace ::com::sun::star;
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextGraphicObject";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleGraphic";

SwAccessibleGraphic::SwAccessibleGraphic(
        SwAccessibleMap* pInitMap,
        const SwFlyFrm* pFlyFrm  ) :
    SwAccessibleNoTextFrame( pInitMap, AccessibleRole::GRAPHIC, pFlyFrm )
{
}

SwAccessibleGraphic::~SwAccessibleGraphic()
{
}

OUString SAL_CALL SwAccessibleGraphic::getImplementationName()
        throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleGraphic::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

Sequence< OUString > SAL_CALL SwAccessibleGraphic::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleGraphic::getImplementationId()
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
//  Return this object's role.
sal_Int16 SAL_CALL SwAccessibleGraphic::getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException)
{
        SwFmtURL aURL( ((SwLayoutFrm*)GetFrm())->GetFmt()->GetURL() );

        if(aURL.GetMap() )
            return AccessibleRole::IMAGE_MAP ;
        return AccessibleRole::GRAPHIC ;
}

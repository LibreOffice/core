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
#include "accembedded.hxx"
#include "cntfrm.hxx"
#include "ndole.hxx"
#include <doc.hxx>
#include <docsh.hxx>
#include <../../ui/inc/wrtsh.hxx>
#include <../../ui/inc/view.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextEmbeddedObject";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleEmbeddedObject";

SwAccessibleEmbeddedObject::SwAccessibleEmbeddedObject(
        SwAccessibleMap* pInitMap,
        const SwFlyFrm* pFlyFrm  ) :
    SwAccessibleNoTextFrame( pInitMap, AccessibleRole::EMBEDDED_OBJECT, pFlyFrm )
{
}

SwAccessibleEmbeddedObject::~SwAccessibleEmbeddedObject()
{
}

//=====  XInterface  ==========================================================
com::sun::star::uno::Any SAL_CALL
    SwAccessibleEmbeddedObject::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = SwAccessibleNoTextFrame::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
         static_cast< ::com::sun::star::accessibility::XAccessibleExtendedAttributes* >(this) );
    return aReturn;
}

void SAL_CALL
    SwAccessibleEmbeddedObject::acquire (void)
    throw ()
{
    SwAccessibleNoTextFrame::acquire ();
}

void SAL_CALL
    SwAccessibleEmbeddedObject::release (void)
    throw ()
{
    SwAccessibleNoTextFrame::release ();
}

OUString SAL_CALL SwAccessibleEmbeddedObject::getImplementationName()
        throw( uno::RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleEmbeddedObject::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleEmbeddedObject::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}


uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleEmbeddedObject::getImplementationId()
        throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}
//=====  XAccessibleExtendedAttributes  ========================================================
::com::sun::star::uno::Any SAL_CALL SwAccessibleEmbeddedObject::getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any strRet;
    ::rtl::OUString style;
    SwFlyFrm* pFFrm = getFlyFrm();

    if( pFFrm )
    {
        style = ::rtl::OUString::createFromAscii("style:");
        SwCntntFrm* pCFrm;
        pCFrm = pFFrm->ContainsCntnt();
        if( pCFrm )
        {
            SwCntntNode* pCNode = pCFrm->GetNode();
            if( pCNode )
            {
                style += ((SwOLENode*)pCNode)->GetOLEObj().GetStyleString();
            }
        }
        style += ::rtl::OUString::createFromAscii(";");
    }
    strRet <<= style;
    return strRet;
}

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
#include <rtl/uuid.h>
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif
#include <accpreview.hxx>

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextDocumentPageView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleDocumentPageView";


// using namespace accessibility;

using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;




//
// SwAccessiblePreview
//

SwAccessiblePreview::SwAccessiblePreview( SwAccessibleMap *pMp ) :
    SwAccessibleDocumentBase( pMp )
{
    SetName( GetResource( STR_ACCESS_PREVIEW_DOC_NAME ) );
}

SwAccessiblePreview::~SwAccessiblePreview()
{
}

OUString SwAccessiblePreview::getImplementationName( )
    throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( sImplementationName ) );
}

sal_Bool SwAccessiblePreview::supportsService( const OUString& rServiceName )
    throw( RuntimeException )
{
    return rServiceName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( sServiceName) ) ||
        rServiceName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( sAccessibleServiceName ) );
}

Sequence<OUString> SwAccessiblePreview::getSupportedServiceNames( )
    throw( RuntimeException )
{
    Sequence<OUString> aSeq( 2 );
    aSeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( sServiceName ) );
    aSeq[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( sAccessibleServiceName ) );
    return aSeq;
}

Sequence< sal_Int8 > SAL_CALL SwAccessiblePreview::getImplementationId()
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
OUString SAL_CALL SwAccessiblePreview::getAccessibleDescription (void) throw (com::sun::star::uno::RuntimeException)
{
    return GetResource( STR_ACCESS_PREVIEW_DOC_NAME );
}

OUString SAL_CALL SwAccessiblePreview::getAccessibleName (void) throw (::com::sun::star::uno::RuntimeException)
{
    OUString sName = SwAccessibleDocumentBase::getAccessibleName();
    sName += OUString::createFromAscii(" ");
    sName += GetResource( STR_ACCESS_PREVIEW_DOC_SUFFIX );
    return sName;
}
void SwAccessiblePreview::_InvalidateFocus()
{
    FireStateChangedEvent( ::com::sun::star::accessibility::AccessibleStateType::FOCUSED, sal_True );
}

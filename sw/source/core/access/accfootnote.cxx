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
#include <ftnfrm.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <viewsh.hxx>
#include <accmap.hxx>
#include "accfootnote.hxx"
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

const sal_Char sServiceNameFootnote[] = "com.sun.star.text.AccessibleFootnoteView";
const sal_Char sServiceNameEndnote[] = "com.sun.star.text.AccessibleEndnoteView";
const sal_Char sImplementationNameFootnote[] = "com.sun.star.comp.Writer.SwAccessibleFootnoteView";
const sal_Char sImplementationNameEndnote[] = "com.sun.star.comp.Writer.SwAccessibleEndnoteView";

SwAccessibleFootnote::SwAccessibleFootnote(
        SwAccessibleMap* pInitMap,
        sal_Bool bIsEndnote,
        const SwFtnFrm *pFtnFrm ) :
    SwAccessibleContext( pInitMap,
        bIsEndnote ? AccessibleRole::END_NOTE : AccessibleRole::FOOTNOTE,
        pFtnFrm )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    sal_uInt16 nResId = bIsEndnote ? STR_ACCESS_ENDNOTE_NAME
                                   : STR_ACCESS_FOOTNOTE_NAME;
    //IAccessibility2 Implementation 2009-----
    //OUString sArg( OUString::valueOf( nFootEndNote ) );
    //old codes end
    OUString sArg;
    const SwTxtFtn *pTxtFtn =
        static_cast< const SwFtnFrm *>( GetFrm() )->GetAttr();
    if( pTxtFtn )
    {
        const SwDoc *pDoc = GetShell()->GetDoc();
        sArg = pTxtFtn->GetFtn().GetViewNumStr( *pDoc );
    }
    //-----IAccessibility2 Implementation 2009

    SetName( GetResource( nResId, &sArg ) );
}

SwAccessibleFootnote::~SwAccessibleFootnote()
{
}

OUString SAL_CALL SwAccessibleFootnote::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    sal_uInt16 nResId = AccessibleRole::END_NOTE == GetRole()
        ? STR_ACCESS_ENDNOTE_DESC
        : STR_ACCESS_FOOTNOTE_DESC ;

    OUString sArg;
    const SwTxtFtn *pTxtFtn =
        static_cast< const SwFtnFrm *>( GetFrm() )->GetAttr();
    if( pTxtFtn )
    {
        const SwDoc *pDoc = GetMap()->GetShell()->GetDoc();
        sArg = pTxtFtn->GetFtn().GetViewNumStr( *pDoc );
    }

    return GetResource( nResId, &sArg );
}

OUString SAL_CALL SwAccessibleFootnote::getImplementationName()
        throw( RuntimeException )
{
    if( AccessibleRole::END_NOTE == GetRole() )
        return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationNameEndnote));
    else
        return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationNameFootnote));
}

sal_Bool SAL_CALL SwAccessibleFootnote::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    if( sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                       sizeof(sAccessibleServiceName)-1 ) )
        return sal_True;
    else if( AccessibleRole::END_NOTE == GetRole() )
        return sTestServiceName.equalsAsciiL( sServiceNameEndnote, sizeof(sServiceNameEndnote)-1 );
    else
        return sTestServiceName.equalsAsciiL( sServiceNameFootnote, sizeof(sServiceNameFootnote)-1 );

}

Sequence< OUString > SAL_CALL SwAccessibleFootnote::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    if( AccessibleRole::END_NOTE == GetRole() )
        pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceNameEndnote) );
    else
        pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceNameFootnote) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleFootnote::getImplementationId()
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

sal_Bool SwAccessibleFootnote::IsEndnote( const SwFtnFrm *pFtnFrm )
{
    const SwTxtFtn *pTxtFtn = pFtnFrm ->GetAttr();
    return pTxtFtn && pTxtFtn->GetFtn().IsEndNote() ;
}

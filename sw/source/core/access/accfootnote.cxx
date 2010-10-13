/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include "access.hrc"

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
        sal_Int32 nFootEndNote,
        const SwFtnFrm *pFtnFrm ) :
    SwAccessibleContext( pInitMap,
        bIsEndnote ? AccessibleRole::END_NOTE : AccessibleRole::FOOTNOTE,
        pFtnFrm )
{
    SolarMutexGuard aGuard;

    sal_uInt16 nResId = bIsEndnote ? STR_ACCESS_ENDNOTE_NAME
                                   : STR_ACCESS_FOOTNOTE_NAME;
    OUString sArg( OUString::valueOf( nFootEndNote ) );
    SetName( GetResource( nResId, &sArg ) );
}

SwAccessibleFootnote::~SwAccessibleFootnote()
{
}

OUString SAL_CALL SwAccessibleFootnote::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

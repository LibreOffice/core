/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/mutex.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/servicehelper.hxx>
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
    OUString sArg( OUString::number( nFootEndNote ) );
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
        return OUString(sImplementationNameEndnote);
    else
        return OUString(sImplementationNameFootnote);
}

sal_Bool SAL_CALL SwAccessibleFootnote::supportsService(
        const OUString& sTestServiceName)
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
        pArray[0] = OUString( sServiceNameEndnote );
    else
        pArray[0] = OUString( sServiceNameFootnote );
    pArray[1] = OUString( sAccessibleServiceName );
    return aRet;
}

namespace
{
    class theSwAccessibleFootnoteImplementationId : public rtl::Static< UnoTunnelIdInit, theSwAccessibleFootnoteImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleFootnote::getImplementationId()
        throw(RuntimeException)
{
    return theSwAccessibleFootnoteImplementationId::get().getSeq();
}

sal_Bool SwAccessibleFootnote::IsEndnote( const SwFtnFrm *pFtnFrm )
{
    const SwTxtFtn *pTxtFtn = pFtnFrm ->GetAttr();
    return pTxtFtn && pTxtFtn->GetFtn().IsEndNote() ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

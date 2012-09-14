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

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/string.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/fstathelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

sal_Bool FStatHelper::GetModifiedDateTimeOfFile( const rtl::OUString& rURL,
                                        Date* pDate, Time* pTime )
{
    sal_Bool bRet = sal_False;
    try
    {
        ::ucbhelper::Content aTestContent( rURL,
                                uno::Reference< XCommandEnvironment > (),
                                comphelper::getProcessComponentContext());
        uno::Any aAny = aTestContent.getPropertyValue(
            OUString("DateModified") );
        if( aAny.hasValue() )
        {
            bRet = sal_True;
            const util::DateTime* pDT = (util::DateTime*)aAny.getValue();
            if( pDate )
                *pDate = Date( pDT->Day, pDT->Month, pDT->Year );
            if( pTime )
                *pTime = Time( pDT->Hours, pDT->Minutes,
                               pDT->Seconds, pDT->HundredthSeconds );
        }
    }
    catch(...)
    {
    }

    return bRet;
}

sal_Bool FStatHelper::IsDocument( const rtl::OUString& rURL )
{
    sal_Bool bExist = sal_False;
    try
    {
        ::ucbhelper::Content aTestContent( rURL,
                                uno::Reference< XCommandEnvironment > (),
                                comphelper::getProcessComponentContext());
        bExist = aTestContent.isDocument();
    }
    catch(...)
    {
    }
    return bExist;
}

sal_Bool FStatHelper::IsFolder( const rtl::OUString& rURL )
{
    sal_Bool bExist = sal_False;
    try
    {
        ::ucbhelper::Content aTestContent( rURL,
                                uno::Reference< XCommandEnvironment > (),
                                comphelper::getProcessComponentContext());
        bExist = aTestContent.isFolder();
    }
    catch(...)
    {
    }
    return bExist;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

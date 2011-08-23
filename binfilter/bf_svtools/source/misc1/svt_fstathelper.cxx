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


#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/string.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/util/DateTime.hpp>

#include <fstathelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

namespace binfilter
{


sal_Bool GetModifiedDateTimeOfFile( const UniString& rURL,
                                        Date* pDate, Time* pTime )
{
    sal_Bool bRet = FALSE;
    try
    {
        ::ucbhelper::Content aTestContent( rURL,
                                uno::Reference< XCommandEnvironment > ());
        uno::Any aAny = aTestContent.getPropertyValue(
            OUString::createFromAscii(  "DateModified" ) );
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

sal_Bool IsDocument( const UniString& rURL )
{
    BOOL bExist = FALSE;
    try
    {
        ::ucbhelper::Content aTestContent( rURL,
                                uno::Reference< XCommandEnvironment > ());
        bExist = aTestContent.isDocument();
    }
    catch(...)
    {
    }
    return bExist;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

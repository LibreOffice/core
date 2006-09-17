/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fstathelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:15:36 $
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
#include "precompiled_svtools.hxx"

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TIME_HXX
#include <tools/time.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#include <fstathelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

sal_Bool FStatHelper::GetModifiedDateTimeOfFile( const UniString& rURL,
                                        Date* pDate, Time* pTime )
{
    sal_Bool bRet = FALSE;
    try
    {
        ::ucb::Content aTestContent( rURL,
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

sal_Bool FStatHelper::IsDocument( const UniString& rURL )
{
    BOOL bExist = FALSE;
    try
    {
        ::ucb::Content aTestContent( rURL,
                                uno::Reference< XCommandEnvironment > ());
        bExist = aTestContent.isDocument();
    }
    catch(...)
    {
    }
    return bExist;
}

sal_Bool FStatHelper::IsFolder( const UniString& rURL )
{
    BOOL bExist = FALSE;
    try
    {
        ::ucb::Content aTestContent( rURL,
                                uno::Reference< XCommandEnvironment > ());
        bExist = aTestContent.isFolder();
    }
    catch(...)
    {
    }
    return bExist;
}


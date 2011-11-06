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
#include "precompiled_svl.hxx"
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/string.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <svl/fstathelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;

sal_Bool FStatHelper::GetModifiedDateTimeOfFile( const UniString& rURL,
                                        Date* pDate, Time* pTime )
{
    sal_Bool bRet = sal_False;
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

sal_Bool FStatHelper::IsDocument( const UniString& rURL )
{
    sal_Bool bExist = sal_False;
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

sal_Bool FStatHelper::IsFolder( const UniString& rURL )
{
    sal_Bool bExist = sal_False;
    try
    {
        ::ucbhelper::Content aTestContent( rURL,
                                uno::Reference< XCommandEnvironment > ());
        bExist = aTestContent.isFolder();
    }
    catch(...)
    {
    }
    return bExist;
}


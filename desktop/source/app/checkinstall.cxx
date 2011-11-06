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
#include "precompiled_desktop.hxx"

#include "checkinstall.hxx"
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/msgbox.hxx>
#include <tools/date.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace desktop
{

sal_Bool CheckInstallation( OUString& rTitle )
{
    try
    {
        Reference< XMultiServiceFactory > xSMgr = ::comphelper::getProcessServiceFactory();
        Reference< XExactName > xExactName( xSMgr->createInstance(
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                        "com.sun.star.comp.desktop.Evaluation" ))),
                                    UNO_QUERY );
        if ( xExactName.is() )
        {
            try
            {
                rTitle = xExactName->getExactName( rTitle );
                Reference< XMaterialHolder > xMaterialHolder( xExactName, UNO_QUERY );
                if ( xMaterialHolder.is() )
                {
                    com::sun::star::util::Date aExpirationDate;
                    Any a = xMaterialHolder->getMaterial();
                    if ( a >>= aExpirationDate )
                    {
                        Date aToday;
                        Date aTimeBombDate( aExpirationDate.Day, aExpirationDate.Month, aExpirationDate.Year );
                        if ( aToday > aTimeBombDate )
                        {
                            InfoBox aInfoBox( NULL, String::CreateFromAscii( "This version has expired" ) );
                            aInfoBox.Execute();
                            return sal_False;
                        }
                    }

                    return sal_True;
                }
                else
                {
                    InfoBox aInfoBox( NULL, rTitle );
                    aInfoBox.Execute();
                    return sal_False;
                }
            }
            catch ( RuntimeException& )
            {
                // Evaluation version expired!
                return sal_False;
            }
        }
        else
        {
            Reference< com::sun::star::container::XContentEnumerationAccess > rContent( xSMgr , UNO_QUERY );
            if( rContent.is() )
            {
                OUString sEvalService = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.Evaluation" ) );
                Reference < com::sun::star::container::XEnumeration > rEnum = rContent->createContentEnumeration( sEvalService );
                if ( rEnum.is() )
                {
                    InfoBox aInfoBox( NULL, rTitle );
                    aInfoBox.Execute();
                    return sal_False;
                }
            }
        }
    }
    catch(Exception)
    {
    }

    return sal_True;
}

} // namespace desktop

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

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

using ::rtl::OUString;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

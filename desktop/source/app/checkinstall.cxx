/*************************************************************************
 *
 *  $RCSfile: checkinstall.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2002-11-01 09:47:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "checkinstall.hxx"

#ifndef _COM_SUN_STAR_BEANS_XEXACTNAME_HPP_
#include <com/sun/star/beans/XExactName.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMATERIALHOLDER_HPP_
#include <com/sun/star/beans/XMaterialHolder.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace desktop
{

sal_Bool CheckInstallation( OUString& rTitle )
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

    return sal_True;
}

} // namespace desktop

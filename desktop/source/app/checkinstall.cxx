/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: checkinstall.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:34:57 $
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
#include "precompiled_desktop.hxx"

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
#ifndef _DATE_HXX
#include <tools/date.hxx>
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

/*************************************************************************
 *
 *  $RCSfile: fileidentifierconverter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sb $ $Date: 2001-06-06 07:32:36 $
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

#ifndef _UCBHELPER_FILEIDENTIFIERCONVERTER_HXX_
#include <ucbhelper/fileidentifierconverter.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_CONTENTPROVIDERINFO_HPP_
#include <com/sun/star/ucb/ContentProviderInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XFILEIDENTIFIERCONVERTER_HPP_
#include <com/sun/star/ucb/XFileIdentifierConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

using namespace com::sun;
using namespace com::sun::star;

namespace ucb {

//============================================================================
//
//  getLocalFileURL
//
//============================================================================

rtl::OUString
getLocalFileURL(
    uno::Reference< star::ucb::XContentProviderManager > const & rManager)
    SAL_THROW((com::sun::star::uno::RuntimeException))
{
    OSL_ASSERT(rManager.is());

    static sal_Char const * const aBaseURLs[]
        = { "file:///", "vnd.sun.star.wfs:///" };
    sal_Int32 nMaxLocality = -1;
    rtl::OUString aMaxBaseURL;
    for (int i = 0; i < sizeof aBaseURLs / sizeof (sal_Char const *); ++i)
    {
        rtl::OUString aBaseURL(rtl::OUString::createFromAscii(aBaseURLs[i]));
        uno::Reference< star::ucb::XFileIdentifierConverter >
            xConverter(rManager->queryContentProvider(aBaseURL),
                       uno::UNO_QUERY);
        if (xConverter.is())
        {
            sal_Int32 nLocality
                = xConverter->getFileProviderLocality(aBaseURL);
            if (nLocality > nMaxLocality)
            {
                nMaxLocality = nLocality;
                aMaxBaseURL = aBaseURL;
            }
        }
    }
    return aMaxBaseURL;
}

//============================================================================
//
//  getFileURLFromSystemPath
//
//============================================================================

rtl::OUString
getFileURLFromSystemPath(
    uno::Reference< star::ucb::XContentProviderManager > const & rManager,
    rtl::OUString const & rBaseURL,
    rtl::OUString const & rSystemPath)
    SAL_THROW((com::sun::star::uno::RuntimeException))
{
    OSL_ASSERT(rManager.is());

    uno::Reference< star::ucb::XFileIdentifierConverter >
        xConverter(rManager->queryContentProvider(rBaseURL), uno::UNO_QUERY);
    if (xConverter.is())
        return xConverter->getFileURLFromSystemPath(rBaseURL, rSystemPath);
    else
        return rtl::OUString();
}

//============================================================================
//
//  getSystemPathFromFileURL
//
//============================================================================

rtl::OUString
getSystemPathFromFileURL(
    uno::Reference< star::ucb::XContentProviderManager > const & rManager,
    rtl::OUString const & rURL)
    SAL_THROW((com::sun::star::uno::RuntimeException))
{
    OSL_ASSERT(rManager.is());

    uno::Reference< star::ucb::XFileIdentifierConverter >
        xConverter(rManager->queryContentProvider(rURL), uno::UNO_QUERY);
    if (xConverter.is())
        return xConverter->getSystemPathFromFileURL(rURL);
    else
        return rtl::OUString();
}

}

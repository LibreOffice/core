/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2002-01-14 09:28:45 $
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

#define I18N_CHARACTERCLASSIFICATION_USES_CLASS_INTERNATIONAL

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _I18N_INDEXENTRYSUPPLIER_HXX_
#include <indexentrysupplier.hxx>
#endif

using namespace ::com::sun::star::i18n;

#define STR_I18N_PATH       "com.sun.star.i18n."
#define STR_I18N_UNICODE    "Unicode"
#define STR_I18N_INSTANCE   "_IndexEntrySupplier"

::rtl::OUString SAL_CALL IndexEntrySupplier::getIndexCharacter(
                            const ::rtl::OUString& rIndexEntry,
                            const ::com::sun::star::lang::Locale& rLocale,
                            const ::rtl::OUString& rSortAlgorithm )
                                throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aRet;
    // take the max count of all length
    ::rtl::OUStringBuffer aBuf( sizeof( STR_I18N_PATH ) +
                                ( rLocale.Country.getLength() +
                                  rLocale.Variant.getLength() +
                                  rSortAlgorithm.getLength() + 3 ) +
                                sizeof( STR_I18N_UNICODE ) +
                                sizeof( STR_I18N_INSTANCE ) );

    aBuf.appendAscii( STR_I18N_PATH, sizeof( STR_I18N_PATH ) - 1  );

    if( rLocale.Language.getLength() )
    {
        aBuf.append( rLocale.Language );
        if ( rLocale.Country.getLength() )
            aBuf.append( (sal_Unicode)'_' ).append( rLocale.Country );
        if ( rLocale.Variant.getLength() )
            aBuf.append( (sal_Unicode)'_' ).append( rLocale.Variant );
        if ( rSortAlgorithm.getLength() )
            aBuf.append( (sal_Unicode)'_' ).append( rSortAlgorithm );
    }
    else // if not locale specified, use default Unicode service.
        aBuf.appendAscii( STR_I18N_UNICODE, sizeof( STR_I18N_UNICODE ) - 1  );
    aBuf.appendAscii( STR_I18N_INSTANCE, sizeof( STR_I18N_INSTANCE ) - 1  );

    ::rtl::OUString rServiceName( aBuf );

    if ( (!rServiceName.equals(aServiceName) || (!xIES.is())) && xMSF.is() ) {

        aServiceName = rServiceName;

        ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >
            xI = xMSF->createInstance( aServiceName );
        ::com::sun::star::uno::Any x;
        if( xI.is() ) {
        x = xI->queryInterface( ::getCppuType((const ::com::sun::star::uno::Reference<
                        ::com::sun::star::i18n::XIndexEntrySupplier>*)0) );
            x >>= xIES;
        } else {
        // For the locale the service does not exist, call default Unicode service,
        // if Unicode service does not exist also, throw an error.
        xI = xMSF->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.i18n.Unicode_IndexEntrySupplier" ));
        if ( xI.is() ) {
            x = xI->queryInterface( ::getCppuType((const ::com::sun::star::uno::Reference<
                        ::com::sun::star::i18n::XIndexEntrySupplier>*)0) );
            x >>= xIES;
        } else
            throw ::com::sun::star::uno::RuntimeException();

        }
    }

    if( xIES.is() )
        aRet = xIES->getIndexCharacter( rIndexEntry, rLocale, rSortAlgorithm );
    else if( rIndexEntry.getLength() )
        aRet = ::rtl::OUString( rIndexEntry.getStr(), 1 );
    return aRet;
}

::rtl::OUString SAL_CALL IndexEntrySupplier::getIndexFollowPageWord(
                            sal_Bool bMorePages,
                            const ::com::sun::star::lang::Locale& rLocale )
                                throw (::com::sun::star::uno::RuntimeException)
{
    static const struct {
        const sal_Char *pLang, *pFollowPage, *pFollowPages;
    } aFollowPageArr[] = {
        { "en",     "p.",       "pp" },
        { "de",     "f.",       "ff." },
        { "es",     "seg.",     "segs." },
        { "it",     "seg.",     "segg." },
        { "fr",     "suivante", "suivantes" },
        { "zh",     "",         "" },
        { "ja",     "",         "" },
        { "ko",     "",         "" },
        { 0, 0, 0 }
    };

    int n;
    for( n = 0; aFollowPageArr[ n ].pLang; ++n )
        if( 0 == rLocale.Language.compareToAscii( aFollowPageArr[ n ].pLang ))
            break;

    if( !aFollowPageArr[ n ].pLang )
        n = 0;      //the default for unknow languages

    return ::rtl::OUString::createFromAscii( bMorePages
                                ? aFollowPageArr[ n ].pFollowPages
                                : aFollowPageArr[ n ].pFollowPage );
}

::rtl::OUString SAL_CALL
IndexEntrySupplier::getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException )
{
    return ::rtl::OUString::createFromAscii( implementationName );
}

sal_Bool SAL_CALL
IndexEntrySupplier::supportsService(const rtl::OUString& rServiceName)
                throw( ::com::sun::star::uno::RuntimeException )
{
    return rServiceName.compareToAscii(implementationName) == 0;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
IndexEntrySupplier::getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString::createFromAscii( implementationName );
    return aRet;
}


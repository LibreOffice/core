/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 12:52:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#include <rtl/ustrbuf.hxx>
#include <tools/string.hxx>
#include <tools/intn.hxx>
#include <tools/isolang.hxx>
#include <indexentrysupplier.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier::IndexEntrySupplier( const Reference < XMultiServiceFactory >& rxMSF ) : xMSF( rxMSF )
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier";
}

OUString SAL_CALL IndexEntrySupplier::getIndexCharacter( const OUString& rIndexEntry,
    const Locale& rLocale, const OUString& rSortAlgorithm )
    throw (RuntimeException)
{
    return getLocaleSpecificIndexEntrySupplier(rLocale, rSortAlgorithm)->
            getIndexCharacter( rIndexEntry, rLocale, rSortAlgorithm );
}

static inline sal_Bool operator == (const Locale& l1, const Locale& l2) {
    return l1.Language == l2.Language && l1.Country == l2.Country && l1.Variant == l2.Variant;
}

sal_Bool SAL_CALL IndexEntrySupplier::createLocaleSpecificIndexEntrySupplier(const OUString& name) throw( RuntimeException )
{
    Reference < XInterface > xI = xMSF->createInstance(
        OUString::createFromAscii("com.sun.star.i18n.IndexEntrySupplier_") + name);

    if ( xI.is() ) {
        xI->queryInterface( ::getCppuType((const Reference< XIndexEntrySupplier>*)0) ) >>= xIES;
        return xIES.is();
    }
    return sal_False;
}

Reference < XIndexEntrySupplier > SAL_CALL
IndexEntrySupplier::getLocaleSpecificIndexEntrySupplier(const Locale& rLocale, const OUString& rSortAlgorithm) throw (RuntimeException)
{
    if (xIES.is() && rLocale == aLocale && rSortAlgorithm == aSortAlgorithm)
        return xIES;
    else if (xMSF.is()) {
        aLocale = rLocale;
        aSortAlgorithm = rSortAlgorithm;

        static sal_Unicode under = (sal_Unicode)'_';
        static OUString tw(OUString::createFromAscii("TW"));
        static OUString unicode(OUString::createFromAscii("Unicode"));

        sal_Int32 l = rLocale.Language.getLength();
        sal_Int32 c = rLocale.Country.getLength();
        sal_Int32 v = rLocale.Variant.getLength();
        sal_Int32 a = rSortAlgorithm.getLength();
        OUStringBuffer aBuf(l+c+v+a+4);

        if ((l > 0 && c > 0 && v > 0 && a > 0 &&
            // load service with name <base>_<lang>_<country>_<varian>_<algorithm>
            createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                    rLocale.Country).append(under).append(rLocale.Variant).append(under).append(
                    rSortAlgorithm).makeStringAndClear())) ||
        (l > 0 && c > 0 && a > 0 &&
            // load service with name <base>_<lang>_<country>_<algorithm>
            createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                    rLocale.Country).append(under).append(rSortAlgorithm).makeStringAndClear())) ||
        (l > 0 && c > 0 && a > 0 && rLocale.Language.compareToAscii("zh") == 0 &&
                        (rLocale.Country.compareToAscii("HK") == 0 ||
                        rLocale.Country.compareToAscii("MO") == 0) &&
            // if the country code is HK or MO, one more step to try TW.
            createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                    tw).append(under).append(rSortAlgorithm).makeStringAndClear())) ||
        (l > 0 && a > 0 &&
            // load service with name <base>_<lang>_<algorithm>
            createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                    rSortAlgorithm).makeStringAndClear())) ||
            // load service with name <algorithm>
        (a > 0 &&
            createLocaleSpecificIndexEntrySupplier(rSortAlgorithm)) ||
            // load default service with name <base>_Unicode
            createLocaleSpecificIndexEntrySupplier(unicode)) {
        return xIES;
        }
    }
    throw RuntimeException();
}

OUString SAL_CALL IndexEntrySupplier::getIndexFollowPageWord( sal_Bool bMorePages,
    const Locale& rLocale ) throw (RuntimeException)
{
    static const struct {
        const sal_Char *pLang, *pFollowPage, *pFollowPages;
    } aFollowPageArr[] = {
        { "en", "p.", "pp." },
        { "de", "f.", "ff." },
        { "es", " s.", " ss." },
        { "it", " e seg.", " e segg." },
        { "fr", " sv", " sv" },
        { "sv", "f.", "ff." },
        { "zh", "", "" },
        { "ja", "p.", "pp." },
        { "ko", "", "" },
        { 0, 0, 0 }
    };

    int n;
    for( n = 0; aFollowPageArr[ n ].pLang; ++n )
        if( 0 == rLocale.Language.compareToAscii( aFollowPageArr[ n ].pLang ))
        break;

    if( !aFollowPageArr[ n ].pLang )
        n = 0;      //the default for unknow languages

    return OUString::createFromAscii( bMorePages ? aFollowPageArr[ n ].pFollowPages
                            : aFollowPageArr[ n ].pFollowPage );
}

OUString SAL_CALL
IndexEntrySupplier::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii( implementationName );
}

sal_Bool SAL_CALL
IndexEntrySupplier::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName.compareToAscii(implementationName) == 0;
}

Sequence< OUString > SAL_CALL
IndexEntrySupplier::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii( implementationName );
    return aRet;
}

} } } }

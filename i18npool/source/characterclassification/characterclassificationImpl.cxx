/*************************************************************************
 *
 *  $RCSfile: characterclassificationImpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2002-03-26 16:59:03 $
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

#include <characterclassificationImpl.hxx>
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

CharacterClassificationImpl::CharacterClassificationImpl(
    const Reference < lang::XMultiServiceFactory >& rxMSF ) : xMSF( rxMSF )
{
    if (createLocaleSpecificCharacterClassification(OUString::createFromAscii("Unicode"), Locale()))
        xUCI = cachedItem->xCI;
}

CharacterClassificationImpl::~CharacterClassificationImpl() {
    // Clear lookuptable
    for (cachedItem = (lookupTableItem*)lookupTable.First();
        cachedItem; cachedItem = (lookupTableItem*)lookupTable.Next())
        delete cachedItem;
    lookupTable.Clear();
}


OUString SAL_CALL
CharacterClassificationImpl::toUpper( const OUString& Text, sal_Int32 nPos,
    sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->toUpper(Text, nPos, nCount, rLocale);
}

OUString SAL_CALL
CharacterClassificationImpl::toLower( const OUString& Text, sal_Int32 nPos,
    sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->toLower(Text, nPos, nCount, rLocale);
}

OUString SAL_CALL
CharacterClassificationImpl::toTitle( const OUString& Text, sal_Int32 nPos,
    sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->toTitle(Text, nPos, nCount, rLocale);
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getType( const OUString& Text, sal_Int32 nPos )
    throw(RuntimeException)
{
    if (xUCI.is())
        return xUCI->getType(Text, nPos);
    throw RuntimeException();
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getCharacterDirection( const OUString& Text, sal_Int32 nPos )
    throw(RuntimeException)
{
    if (xUCI.is())
        return xUCI->getCharacterDirection(Text, nPos);
    throw RuntimeException();
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getScript( const OUString& Text, sal_Int32 nPos )
    throw(RuntimeException)
{
    if (xUCI.is())
        return xUCI->getScript(Text, nPos);
    throw RuntimeException();
}

sal_Int32 SAL_CALL
CharacterClassificationImpl::getCharacterType( const OUString& Text, sal_Int32 nPos,
    const Locale& rLocale ) throw(RuntimeException)
{
    return getLocaleSpecificCharacterClassification(rLocale)->getCharacterType(Text, nPos, rLocale);
}

sal_Int32 SAL_CALL
CharacterClassificationImpl::getStringType( const OUString& Text, sal_Int32 nPos,
    sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException)
{
    return getLocaleSpecificCharacterClassification(rLocale)->getStringType(Text, nPos, nCount, rLocale);
}

ParseResult SAL_CALL CharacterClassificationImpl::parseAnyToken(
    const OUString& Text, sal_Int32 nPos, const Locale& rLocale,
    sal_Int32 startCharTokenType, const OUString& userDefinedCharactersStart,
    sal_Int32 contCharTokenType, const OUString& userDefinedCharactersCont )
    throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->parseAnyToken(Text, nPos, rLocale,
        startCharTokenType,userDefinedCharactersStart,
        contCharTokenType, userDefinedCharactersCont);
}


ParseResult SAL_CALL CharacterClassificationImpl::parsePredefinedToken(
    sal_Int32 nTokenType, const OUString& Text, sal_Int32 nPos,
    const Locale& rLocale, sal_Int32 startCharTokenType,
    const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
    const OUString& userDefinedCharactersCont ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->parsePredefinedToken(
        nTokenType, Text, nPos, rLocale, startCharTokenType, userDefinedCharactersStart,
        contCharTokenType, userDefinedCharactersCont);
}

sal_Bool SAL_CALL CharacterClassificationImpl::createLocaleSpecificCharacterClassification(const OUString& serviceName, const Locale& rLocale)
{
    // to share service between same Language but different Country code, like zh_CN and zh_SG
    for (cachedItem = (lookupTableItem*)lookupTable.First();
            cachedItem; cachedItem = (lookupTableItem*)lookupTable.Next()) {
        if (serviceName == cachedItem->aName) {
        lookupTable.Insert( cachedItem = new lookupTableItem(rLocale, serviceName, cachedItem->xCI) );
        return sal_True;
        }
    }

    Reference < XInterface > xI = xMSF->createInstance(
        OUString::createFromAscii("com.sun.star.i18n.CharacterClassification_") + serviceName);

    Reference < XCharacterClassification > xCI;
    if ( xI.is() ) {
        xI->queryInterface(::getCppuType((const Reference< XCharacterClassification>*)0) ) >>= xCI;
        if (xCI.is()) {
        lookupTable.Insert( cachedItem =  new lookupTableItem(rLocale, serviceName, xCI) );
        return sal_True;
        }
    }
    return sal_False;
}

Reference < XCharacterClassification > SAL_CALL
CharacterClassificationImpl::getLocaleSpecificCharacterClassification(const Locale& rLocale)
    throw(RuntimeException)
{
    // reuse instance if locale didn't change
    if (cachedItem && cachedItem->equals(rLocale))
        return cachedItem->xCI;
    else if (xMSF.is()) {
        for (cachedItem = (lookupTableItem*)lookupTable.First();
            cachedItem; cachedItem = (lookupTableItem*)lookupTable.Next()) {
        if (cachedItem->equals(rLocale))
            return cachedItem->xCI;
        }

        static sal_Unicode under = (sal_Unicode)'_';
        static OUString tw(OUString::createFromAscii("TW"));
        sal_Int32 l = rLocale.Language.getLength();
        sal_Int32 c = rLocale.Country.getLength();
        sal_Int32 v = rLocale.Variant.getLength();
        OUStringBuffer aBuf(l+c+v+3);

            // load service with name <base>_<lang>_<country>_<varian>
        if ((l > 0 && c > 0 && v > 0 &&
            createLocaleSpecificCharacterClassification(aBuf.append(rLocale.Language).append(under).append(
                    rLocale.Country).append(under).append(rLocale.Variant).makeStringAndClear(), rLocale)) ||
            // load service with name <base>_<lang>_<country>
        (l > 0 && c > 0 &&
            createLocaleSpecificCharacterClassification(aBuf.append(rLocale.Language).append(under).append(
                    rLocale.Country).makeStringAndClear(), rLocale)) ||
        (l > 0 && c > 0 && rLocale.Language.compareToAscii("zh") == 0 &&
                    (rLocale.Country.compareToAscii("HK") == 0 ||
                    rLocale.Country.compareToAscii("MO") == 0) &&
            // if the country code is HK or MO, one more step to try TW.
            createLocaleSpecificCharacterClassification(aBuf.append(rLocale.Language).append(under).append(
                    tw).makeStringAndClear(), rLocale)) ||
        (l > 0 &&
            // load service with name <base>_<lang>
            createLocaleSpecificCharacterClassification(rLocale.Language, rLocale))) {
        return cachedItem->xCI;
        } else if (xUCI.is()) {
        lookupTable.Insert( cachedItem = new lookupTableItem(rLocale, OUString::createFromAscii("Unicode"), xUCI) );
        return cachedItem->xCI;
        }
    }
    throw RuntimeException();
}

const sal_Char cClass[] = "com.sun.star.i18n.CharacterClassification";

OUString SAL_CALL
CharacterClassificationImpl::getImplementationName(void)
                throw( RuntimeException )
{
    return OUString::createFromAscii(cClass);
}

sal_Bool SAL_CALL
CharacterClassificationImpl::supportsService(const rtl::OUString& rServiceName)
                throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cClass);
}

Sequence< OUString > SAL_CALL
CharacterClassificationImpl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cClass);
    return aRet;
}

} } } }

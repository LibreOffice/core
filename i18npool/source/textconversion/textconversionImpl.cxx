/*************************************************************************
 *
 *  $RCSfile: textconversionImpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:53:51 $
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

#include <assert.h>
#include <textconversionImpl.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

TextConversionResult SAL_CALL
TextConversionImpl::getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& aLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    getLocaleSpecificTextConversion(aLocale);

    return xTC->getConversions(aText, nStartPos, nLength, aLocale, nConversionType, nConversionOptions);
}

OUString SAL_CALL
TextConversionImpl::getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& aLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    getLocaleSpecificTextConversion(aLocale);

    return xTC->getConversion(aText, nStartPos, nLength, aLocale, nConversionType, nConversionOptions);
}

sal_Bool SAL_CALL
TextConversionImpl::interactiveConversion( const Locale& aLocale, sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    getLocaleSpecificTextConversion(aLocale);

    return xTC->interactiveConversion(aLocale, nTextConversionType, nTextConversionOptions);
}

static inline sal_Bool operator != (const Locale& l1, const Locale& l2) {
    return l1.Language != l2.Language || l1.Country != l2.Country || l1.Variant != l2.Variant;
}

void SAL_CALL
TextConversionImpl::getLocaleSpecificTextConversion(const Locale& rLocale) throw( NoSupportException )
{
    if (xMSF.is() && rLocale != aLocale) {
        aLocale = rLocale;

        Reference < XInterface > xI;

        xI = xMSF->createInstance(
            OUString::createFromAscii("com.sun.star.i18n.TextConversion_") + aLocale.Language);

        if ( ! xI.is() )
            xI = xMSF->createInstance(
                OUString::createFromAscii("com.sun.star.i18n.TextConversion_") + aLocale.Language +
                OUString::createFromAscii("_") + aLocale.Country);
        if ( ! xI.is() )
            xI = xMSF->createInstance(
                OUString::createFromAscii("com.sun.star.i18n.TextConversion_") + aLocale.Language +
                OUString::createFromAscii("_") + aLocale.Country +
                OUString::createFromAscii("_") + aLocale.Variant);

        if (xI.is())
            xI->queryInterface( getCppuType((const Reference< XTextConversion>*)0) ) >>= xTC;
        else if (xTC.is())
            xTC.clear();
    }
    if (! xTC.is())
        throw NoSupportException(); // aLocale is not supported
}

const sal_Char cTextConversion[] = "com.sun.star.i18n.TextConversion";

OUString SAL_CALL
TextConversionImpl::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(cTextConversion);
}

sal_Bool SAL_CALL
TextConversionImpl::supportsService(const OUString& rServiceName)
                throw( RuntimeException )
{
    return rServiceName.equalsAscii(cTextConversion);
}

Sequence< OUString > SAL_CALL
TextConversionImpl::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cTextConversion);
    return aRet;
}

} } } }

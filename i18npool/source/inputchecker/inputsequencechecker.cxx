/*************************************************************************
 *
 *  $RCSfile: inputsequencechecker.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 11:07:23 $
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

#include <inputsequencechecker.hxx>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <i18nutil/unicode.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

InputSequenceCheckerImpl::InputSequenceCheckerImpl( const Reference < XMultiServiceFactory >& rxMSF ) : xMSF( rxMSF )
{
    serviceName = "com.sun.star.i18n.InputSequenceCheckerImpl";
    cachedItem = NULL;
}

InputSequenceCheckerImpl::InputSequenceCheckerImpl()
{
}

InputSequenceCheckerImpl::~InputSequenceCheckerImpl()
{
    // Clear lookuptable
    for (cachedItem = (lookupTableItem*)lookupTable.First();
            cachedItem; cachedItem = (lookupTableItem*)lookupTable.Next())
        delete cachedItem;
    lookupTable.Clear();
}

sal_Bool SAL_CALL
InputSequenceCheckerImpl::checkInputSequence(const OUString& Text, sal_Int32 nStartPos,
    sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(RuntimeException)
{
    if (inputCheckMode == InputSequenceCheckMode::PASSTHROUGH)
        return sal_True;

    sal_Char* language = getLanguageByScripType(Text[nStartPos], inputChar);

    if (language)
        return getInputSequenceChecker(language)->checkInputSequence(Text, nStartPos, inputChar, inputCheckMode);
    else
        return sal_True; // not a checkable languages.
}

static ScriptTypeList typeList[] = {
    //{ UnicodeScript_kHebrew,      UnicodeScript_kHebrew },    // 10,
    //{ UnicodeScript_kArabic,      UnicodeScript_kArabic },    // 11,
    //{ UnicodeScript_kDevanagari,      UnicodeScript_kDevanagari },    // 14,
    { UnicodeScript_kThai,          UnicodeScript_kThai },      // 24,

    { UnicodeScript_kScriptCount,       UnicodeScript_kScriptCount }    // 88
};

sal_Char* SAL_CALL
InputSequenceCheckerImpl::getLanguageByScripType(sal_Unicode cChar, sal_Unicode nChar)
{
    sal_Int16 type = unicode::getUnicodeScriptType( cChar, typeList, UnicodeScript_kScriptCount );

    if (type != UnicodeScript_kScriptCount &&
        type == unicode::getUnicodeScriptType( nChar, typeList, UnicodeScript_kScriptCount )) {
    switch(type) {
        case UnicodeScript_kThai:       return "th";
        //case UnicodeScript_kArabic:   return "ar";
        //case UnicodeScript_kHebrew:   return "he";
        //cace UnicodeScript_kDevanagari:   return "hi";
    }
    }
    return NULL;
}

Reference< XInputSequenceChecker >& SAL_CALL
InputSequenceCheckerImpl::getInputSequenceChecker(sal_Char* rLanguage) throw (RuntimeException)
{
    if (cachedItem && cachedItem->aLanguage == rLanguage) {
        return cachedItem->xISC;
    }
    else if (xMSF.is()) {
        for (cachedItem = (lookupTableItem*)lookupTable.First();
            cachedItem; cachedItem = (lookupTableItem*)lookupTable.Next()) {
        if (cachedItem->aLanguage == rLanguage) {
            return cachedItem->xISC;
        }
        }

        Reference < uno::XInterface > xI = xMSF->createInstance(
            OUString::createFromAscii("com.sun.star.i18n.InputSequenceChecker_") +
            OUString::createFromAscii(rLanguage));

        if ( xI.is() ) {
        Reference< XInputSequenceChecker > xISC;
        xI->queryInterface( getCppuType((const Reference< XInputSequenceChecker>*)0) ) >>= xISC;
        if (xISC.is()) {
            lookupTable.Insert(cachedItem = new lookupTableItem(rLanguage, xISC));
            return cachedItem->xISC;
        }
        }
    }
    throw RuntimeException();
}

OUString SAL_CALL
InputSequenceCheckerImpl::getImplementationName(void) throw( RuntimeException )
{
    return OUString::createFromAscii(serviceName);
}

sal_Bool SAL_CALL
InputSequenceCheckerImpl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(serviceName);
}

Sequence< OUString > SAL_CALL
InputSequenceCheckerImpl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(serviceName);
    return aRet;
}

} } } }

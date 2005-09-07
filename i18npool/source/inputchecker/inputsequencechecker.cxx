/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inputsequencechecker.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:17:12 $
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
        for (sal_Int32 l = 0; l < lookupTable.size(); l++)
            delete lookupTable[l];

        lookupTable.clear();
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
        //{ UnicodeScript_kHebrew,              UnicodeScript_kHebrew },        // 10,
        //{ UnicodeScript_kArabic,              UnicodeScript_kArabic },        // 11,
        { UnicodeScript_kDevanagari,UnicodeScript_kDevanagari,          UnicodeScript_kDevanagari },    // 14,
        { UnicodeScript_kThai,  UnicodeScript_kThai,                  UnicodeScript_kThai },          // 24,

        { UnicodeScript_kScriptCount,   UnicodeScript_kScriptCount,           UnicodeScript_kScriptCount }    // 88
};

sal_Char* SAL_CALL
InputSequenceCheckerImpl::getLanguageByScripType(sal_Unicode cChar, sal_Unicode nChar)
{
    sal_Int16 type = unicode::getUnicodeScriptType( cChar, typeList, UnicodeScript_kScriptCount );

    if (type != UnicodeScript_kScriptCount &&
            type == unicode::getUnicodeScriptType( nChar, typeList, UnicodeScript_kScriptCount )) {
        switch(type) {
            case UnicodeScript_kThai:           return (sal_Char*)"th";
            //case UnicodeScript_kArabic:       return (sal_Char*)"ar";
            //case UnicodeScript_kHebrew:       return (sal_Char*)"he";
            case UnicodeScript_kDevanagari:   return (sal_Char*)"hi";
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
            for (sal_Int32 l = 0; l < lookupTable.size(); l++) {
                cachedItem = lookupTable[l];
                if (cachedItem->aLanguage == rLanguage)
                    return cachedItem->xISC;
            }

            Reference < uno::XInterface > xI = xMSF->createInstance(
                        OUString::createFromAscii("com.sun.star.i18n.InputSequenceChecker_") +
                        OUString::createFromAscii(rLanguage));

            if ( xI.is() ) {
                Reference< XInputSequenceChecker > xISC;
                xI->queryInterface( getCppuType((const Reference< XInputSequenceChecker>*)0) ) >>= xISC;
                if (xISC.is()) {
                    lookupTable.push_back(cachedItem = new lookupTableItem(rLanguage, xISC));
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

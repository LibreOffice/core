/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "precompiled_linguistic.hxx"

#include <sal/config.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/linguistic2/XGrammarChecker.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "linguistic/misc.hxx"
#include "defs.hxx"
#include <cppuhelper/factory.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/linguistic2/SingleGrammarError.hpp>
#include <com/sun/star/linguistic2/GrammarCheckingResult.hpp>
#include "lngopt.hxx"
#include <comphelper/extract.hxx>
#include <unotools/processfactory.hxx>
#include <map>
#include <com/sun/star/text/TextMarkupType.hpp>

#include "grammarchecker.hxx"

using namespace ::utl;
using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////

GrammarChecker::GrammarChecker( /*uno::Reference< uno::XComponentContext > const & context*/ )
    /*m_xContext(context)*/
{}

GrammarChecker::~GrammarChecker()
{
}


sal_Bool SAL_CALL GrammarChecker::isSpellChecker() throw (uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    return sal_False;
}


sal_Bool SAL_CALL GrammarChecker::hasLocale( const lang::Locale & aLocale ) throw (uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    (void) aLocale;
    return sal_False;
}

uno::Sequence< lang::Locale > SAL_CALL GrammarChecker::getLocales(  ) throw (uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    return uno::Sequence< lang::Locale >();
}


void SAL_CALL GrammarChecker::startDocument(sal_Int32 nDocId)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    (void) nDocId;
}

void SAL_CALL GrammarChecker::startParagraph(sal_Int32 nDocId)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    (void) nDocId;
}

void SAL_CALL GrammarChecker::endParagraph( sal_Int32 nDocId )
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    (void) nDocId;
}

void SAL_CALL GrammarChecker::endDocument(sal_Int32 nDocId)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    (void) nDocId;
}

linguistic2::GrammarCheckingResult SAL_CALL GrammarChecker::doGrammarChecking(
        sal_Int32 nDocId,
        const rtl::OUString& rText,
        const lang::Locale& rLocale,
        sal_Int32 nStartOfSentencePos,
        sal_Int32 nSuggestedSentenceEndPos,
        const uno::Sequence< ::sal_Int32 >& rLanguagePortions,
        const uno::Sequence< lang::Locale >& rLanguagePortionsLocales )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());

    (void) rLanguagePortions;
    (void) rLanguagePortionsLocales;

    linguistic2::GrammarCheckingResult  aRes;
    aRes.nDocumentId                = nDocId;
    aRes.aText                      = rText;
    aRes.aLocale                    = rLocale;
    aRes.nEndOfSentencePos          = nSuggestedSentenceEndPos;
    aRes.xGrammarChecker            = this;
    aRes.aGrammarErrors             = GrammarCheckingInDummy( nDocId, rText, rLocale, nStartOfSentencePos, nSuggestedSentenceEndPos );;

    return aRes;
}

uno::Sequence< linguistic2::SingleGrammarError > GrammarChecker::GrammarCheckingInDummy(
    sal_Int32 nDocId,
    const OUString & rFlatParaText,
    const lang::Locale & rLocale,
    sal_Int32 nStartOfSentencePos,
    sal_Int32 nSuggestedSentenceEndPos )
{
    (void) nDocId;
    (void) rFlatParaText;
    (void) rLocale;
    (void) nStartOfSentencePos;
    (void) nSuggestedSentenceEndPos;


    typedef std::map< OUString, uno::Sequence<OUString> > Error_t;
    Error_t aError;
    uno::Sequence< OUString > aSuggestion(1);
    OUString *pSeggestion = aSuggestion.getArray();
    pSeggestion[0] = OUString::createFromAscii("Modified");

    aError[OUString::createFromAscii("GrammarError")]  = aSuggestion;
    aError[OUString::createFromAscii("Grammar Error")] = aSuggestion;

    typedef std::vector< linguistic2::SingleGrammarError> ErrorVector_t;
    ErrorVector_t aErrorVector;

    OUString aText = rFlatParaText.copy( nStartOfSentencePos, nSuggestedSentenceEndPos - nStartOfSentencePos );
    sal_Int32 nIndexOf = 0;
    for(Error_t::const_iterator it = aError.begin(); it != aError.end(); ++it)
    {

        while(nIndexOf >= 0)
        {
            nIndexOf=aText.indexOf(it->first, nIndexOf);
            if(nIndexOf > -1)
            {
                //error found
                linguistic2::SingleGrammarError aErr;
                aErr.nErrorStart        = nIndexOf + nStartOfSentencePos;
                nIndexOf += it->first.getLength();
                aErr.nErrorLength       = it->first.getLength();
                aErr.nErrorType         = text::TextMarkupType::GRAMMAR;
                aErr.nErrorLevel        = 0;
                aErr.aShortComment      = OUString();
                aErr.aFullComment       = OUString();
                aErr.aNewLocale         = rLocale;
                aErr.aSuggestions       = it->second;

                aErrorVector.push_back( aErr );
            }
        }
        nIndexOf = 0;
    }

    sal_Int32 nCount = aErrorVector.size();
    uno::Sequence< linguistic2::SingleGrammarError > aErrors( nCount );
    if( nCount > 0 )
    {
        linguistic2::SingleGrammarError* pErrors = aErrors.getArray();
        for (sal_Int32 i=0; i < nCount; ++i)
        {
            pErrors[i] = aErrorVector[i];
        }
    }
    return aErrors;
}


sal_Bool SAL_CALL GrammarChecker::hasOptionsDialog(  ) throw (uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    return sal_False;
}

void SAL_CALL GrammarChecker::runOptionsDialog()
    throw (uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
}

void SAL_CALL GrammarChecker::dispose(  ) throw (uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
}

void SAL_CALL GrammarChecker::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    (void) xListener;
}

void SAL_CALL GrammarChecker::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    (void) xListener;
}

sal_Bool SAL_CALL GrammarChecker::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());

    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); ++i )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

uno::Sequence< OUString > GrammarChecker::getSupportedServiceNames_Static(  ) throw()
{
    //osl::Guard< osl::Mutex > aGuard(GetMutex());

    uno::Sequence< OUString > aSNS( 1 );    // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = A2OU( "com.sun.star.linguistic2.GrammarChecker" );//SN_LINGU_SERVCICE_MANAGER
    return aSNS;
}

uno::Sequence< OUString > SAL_CALL GrammarChecker::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    return getSupportedServiceNames_Static();
}

OUString SAL_CALL GrammarChecker::getImplementationName(  ) throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    return getImplementationName_Static();
}

uno::Reference< uno::XInterface > SAL_CALL GrammarChecker_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory > & /*rSMgr*/ )
    throw(uno::Exception)
{
    uno::Reference< uno::XInterface > xService = (cppu::OWeakObject*) new GrammarChecker;
    return xService;
}

void * SAL_CALL GrammarChecker_getFactory( const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager, void * /*pRegistryKey*/ )
{

    void * pRet = 0;
    if ( !GrammarChecker::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                GrammarChecker::getImplementationName_Static(),
                GrammarChecker_CreateInstance,
                GrammarChecker::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}


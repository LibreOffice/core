/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grammarchecker.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 09:49:42 $
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
#include "precompiled_linguistic.hxx"

#include <sal/config.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/linguistic2/XGrammarChecker.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <vcl/unohelp.hxx>
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XGrammarCheckingResultBroadcaster.hpp>
#include "misc.hxx"
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
#include <cppuhelper/extract.hxx>
#include <unotools/processfactory.hxx>
#include <map>
#include <com/sun/star/text/TextMarkupType.hpp>

#include "grammarchecker.hxx"

using namespace ::utl;
using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////

GrammarChecker::GrammarChecker( /*uno::Reference< uno::XComponentContext > const & context*/ ) :
    m_aGCListeners( GetMutex() ) /*:
    m_xContext(context)*/
{}

GrammarChecker::~GrammarChecker()
{
}


// linguistic2::XGrammarChecker:
::sal_Bool SAL_CALL GrammarChecker::isSpellChecker( const lang::Locale & aLocale) throw (uno::RuntimeException)
{
    (void) aLocale;
    // TODO: Exchange the default return implementation for "isSpellChecker" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.

    //MutexGuard    aGuard( GetMutex() );
    return sal_False;
}


// linguistic2::XSupportedLocales:
::sal_Bool SAL_CALL GrammarChecker::hasLocale( const lang::Locale & aLocale ) throw (uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "hasLocale" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.

    (void) aLocale;
    //MutexGuard    aGuard( GetMutex() );
    return sal_False;
}

// linguistic2::XSupportedLocales:
uno::Sequence< lang::Locale > SAL_CALL GrammarChecker::getLocales(  ) throw (uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "getLocales" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return uno::Sequence< lang::Locale >();
}


void SAL_CALL GrammarChecker::startDocument(sal_Int32 nDocId)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) nDocId;
    // TODO: Insert your implementation for "startDocument" here.
}

void SAL_CALL GrammarChecker::startParagraph(sal_Int32 nDocId)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) nDocId;
    // TODO: Insert your implementation for "startParagraph" here.
}

void SAL_CALL GrammarChecker::endParagraph( sal_Int32 nDocId )
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) nDocId;
    // TODO: Insert your implementation for "endParagraph" here.
}

void SAL_CALL GrammarChecker::endDocument(sal_Int32 nDocId)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) nDocId;
    // TODO: Insert your implementation for "endDocument" here.
}


void SAL_CALL GrammarChecker::doGrammarChecking(
        sal_Int32 nDocId,
        const uno::Reference< text::XFlatParagraph > & xFlatPara,
        const lang::Locale & aLocale,
        sal_Int32 nStartOfSentencePos,
        sal_Int32 nSuggestedSentenceEndPos)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    // TODO: Insert your implementation for "doGrammarChecking" here.
    (void) nDocId;

    OUString aFlatParaText = xFlatPara->getText();
    linguistic2::GrammarCheckingResult paRes;
    //paRes->nDocId=nDocId;
    paRes.xPara=xFlatPara;
    paRes.aText=aFlatParaText;//.copy(nStartOfSentencePos, nSuggestedSentenceEndPos-nStartOfSentencePos);
    paRes.aLocale=aLocale;
    paRes.nEndOfSentencePos=nSuggestedSentenceEndPos;
    uno::Sequence< linguistic2::SingleGrammarError > aError;

    aError=GrammarCheckingInDummy(nDocId, aFlatParaText, aLocale, nStartOfSentencePos, nSuggestedSentenceEndPos);

    paRes.aGrammarErrors=aError;

    if(m_aGCListeners.getLength() > 0)
    {

        ::cppu::OInterfaceIteratorHelper pIterator( m_aGCListeners );
        while ( pIterator.hasMoreElements( ) )
        {
            uno::Reference< linguistic2::XGrammarCheckingResultListener > xLstnr( pIterator.next(), uno::UNO_QUERY );
            xLstnr->GrammarCheckingFinished(paRes);
        }
    }

}

uno::Sequence< linguistic2::SingleGrammarError > GrammarChecker::GrammarCheckingInDummy(
    sal_Int32 nDocId,
    const OUString aFlatParaText,
    const lang::Locale & rLocale,
    sal_Int32 nStartOfSentencePos,
    sal_Int32 nSuggestedSentenceEndPos
    )
{
    (void) nDocId;
    (void) aFlatParaText;
    (void) rLocale;
    (void) nStartOfSentencePos;
    (void) nSuggestedSentenceEndPos;


    typedef std::map< OUString, uno::Sequence<OUString> > Error_t;
    Error_t aError;
    uno::Sequence< OUString > aSeggestion(1);
    OUString *pSeggestion = aSeggestion.getArray();
    pSeggestion[0] = OUString::createFromAscii("Modified");

    aError[OUString::createFromAscii("GrammarError")]=aSeggestion;
    aError[OUString::createFromAscii("Grammar Error")]=aSeggestion;

    typedef std::vector< linguistic2::SingleGrammarError> ErrorVector_t;
    ErrorVector_t aErrorVector;

    OUString aText=aFlatParaText.copy(nStartOfSentencePos, nSuggestedSentenceEndPos-nStartOfSentencePos);
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
                aErr.nErrorStart=nIndexOf + nStartOfSentencePos;
                nIndexOf += it->first.getLength();
                aErr.nErrorLen=it->first.getLength();
                aErr.nErrorType=text::TextMarkupType::GRAMMAR;
                aErr.nErrorLevel=0;
                aErr.xGC=this;
                aErr.aShortComment=String::CreateFromAscii("");
                aErr.aFullComment=String::CreateFromAscii("");
                aErr.aNewLocale=rLocale;
                aErr.aSuggestions=it->second;
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


sal_Int32 SAL_CALL GrammarChecker::getEndOfSentencePos(
        sal_Int32 nDocId,
        const OUString & aFlatParaText,
        const lang::Locale & aLocale,
        sal_Int32 nStartOfSentencePos)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) nDocId;
    // TODO: Exchange the default return implementation for "getEndOfSentencePos" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    uno::Reference< i18n::XBreakIterator > xBreakIterator;
    if(!xBreakIterator.is())
    {
        xBreakIterator=vcl::unohelper::CreateBreakIterator();
    }
    sal_Int32 nEndPosition=0;
    if(xBreakIterator.is())
    {
        nEndPosition=xBreakIterator->endOfSentence(aFlatParaText, nStartOfSentencePos, aLocale);
    }
    return nEndPosition;
}

sal_Int32 SAL_CALL GrammarChecker::getStartOfSentencePos(
        sal_Int32 nDocId,
        const OUString & aFlatParaText,
        const lang::Locale & aLocale)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) nDocId;
    // TODO: Exchange the default return implementation for "getStartOfSentencePos" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    uno::Reference< i18n::XBreakIterator > xBreakIterator;
    if(!xBreakIterator.is())
    {
        xBreakIterator=vcl::unohelper::CreateBreakIterator();
    }
    sal_Int32 nStartPosition=0;
    if(xBreakIterator.is())
    {
        nStartPosition=xBreakIterator->beginOfSentence(aFlatParaText, 0, aLocale);
    }
    return nStartPosition;

}

::sal_Bool SAL_CALL GrammarChecker::requiresPreviousText(  ) throw (uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "requiresPreviousText" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return sal_False;
}

::sal_Bool SAL_CALL GrammarChecker::hasCheckingDialog(  ) throw (uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "hasCheckingDialog" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return sal_False;
}

::sal_Bool SAL_CALL GrammarChecker::hasOptionsDialog(  ) throw (uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "hasOptionsDialog" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return sal_False;
}

void SAL_CALL GrammarChecker::runCheckingDialog( sal_Int32 nDocId )
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) nDocId;
    // TODO: Insert your implementation for "runCheckingDialog" here.
}

void SAL_CALL GrammarChecker::runOptionsDialog( sal_Int32 nDocId )
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) nDocId;
    // TODO: Insert your implementation for "runOptionsDialog" here.
}

::sal_Bool SAL_CALL GrammarChecker::addGrammarCheckingResultListener( const uno::Reference< linguistic2::XGrammarCheckingResultListener >& xLstnr )
    throw (uno::RuntimeException)
{
    (void) xLstnr;

    osl::Guard< osl::Mutex > aGuard(GetMutex());

    sal_Bool bRes = sal_False;
    if (xLstnr.is())
    {
        sal_Int32   nCount = m_aGCListeners.getLength();
        bRes = m_aGCListeners.addInterface( xLstnr ) != nCount;
    }
    return bRes;
}

::sal_Bool SAL_CALL GrammarChecker::removeGrammarCheckingResultListener( const uno::Reference< linguistic2::XGrammarCheckingResultListener >& xLstnr )
    throw (uno::RuntimeException)
{
    (void) xLstnr;

    osl::Guard< osl::Mutex > aGuard(GetMutex());

    sal_Bool bRes = sal_False;
    if (xLstnr.is())
    {
        sal_Int32   nCount = m_aGCListeners.getLength();
        bRes = m_aGCListeners.removeInterface( xLstnr ) != nCount;
    }
    return bRes;
}

void SAL_CALL GrammarChecker::dispose(  ) throw (uno::RuntimeException)
{
}

void SAL_CALL GrammarChecker::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    (void) xListener;
}

void SAL_CALL GrammarChecker::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    (void) xListener;
}

sal_Bool SAL_CALL GrammarChecker::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());

    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( INT32 i = 0; i < aSNL.getLength(); ++i )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;
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

sal_Bool SAL_CALL GrammarChecker_writeInfo( void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += GrammarChecker::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        uno::Reference< registry::XRegistryKey > xNewKey =
            pRegistryKey->createKey( aImpl );
        uno::Sequence< OUString > aServices = GrammarChecker::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); ++i )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    catch(uno::Exception &)
    {
        return sal_False;
    }
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


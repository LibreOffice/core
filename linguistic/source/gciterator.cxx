/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gciterator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 09:49:22 $
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
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/linguistic2/XGrammarCheckingIterator.hpp>
#include <deque>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/text/XFlatParagraph.hpp>
#include <com/sun/star/text/XFlatParagraphIterator.hpp>
#include <map>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <vcl/unohelp.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <osl/conditn.hxx>
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XGrammarCheckingResultListener.hpp>
#include <com/sun/star/linguistic2/XGrammarChecker.hpp>
#include <com/sun/star/linguistic2/XGrammarCheckingResultBroadcaster.hpp>
#include "misc.hxx"
#include "defs.hxx"
#include <cppuhelper/factory.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/linguistic2/SingleGrammarError.hpp>

#include "gciterator.hxx"
#include "grammarchecker.hxx"

#include <unotools/processfactory.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include "lngopt.hxx"
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>

#include <vcl/unohelp.hxx>
#include <com/sun/star/linguistic2/SingleGrammarError.hpp>
#include <com/sun/star/linguistic2/GrammarCheckingResult.hpp>
#include <cppuhelper/extract.hxx>
#include <i18npool/mslangid.hxx>
#include <osl/thread.hxx>
#include <com/sun/star/text/XTextMarkup.hpp>


using namespace ::utl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace linguistic;

extern "C" void workerfunc (void * gci)
{
    ((GrammarCheckingIterator*)gci)->dequeueAndCheck();
}

////////////////////////////////////////////////////////////

GrammarCheckingIterator::GrammarCheckingIterator( /* uno::Reference< uno::XComponentContext > const & context */ ) :
    /*m_xContext(context)*/
    m_bEnd( sal_False ),
    m_nDocID( 0 )
{
    osl_createThread(workerfunc, this);
}

GrammarCheckingIterator::~GrammarCheckingIterator()
{
}

//create new DocID
const ::sal_Int32 GrammarCheckingIterator::NextDocId()
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());

    m_nDocID += 1;
    return m_nDocID;
}

lang::Locale getPrimaryLanguageOfSentence( uno::Reference< text::XFlatParagraph > xFlatPara, sal_Int32 nStartIndex )
{
    (void) nStartIndex;
    //get the language of the first word
    return xFlatPara->getLanguageOfText(nStartIndex, 1);//nStartIndex
}

void GrammarCheckingIterator::AddEntry(
    uno::WeakReference< text::XFlatParagraphIterator > xFlatParaIterator,
    uno::WeakReference< text::XFlatParagraph > xFlatPara,
    sal_Int32 nDocID,
    sal_Int32 nStartIndex,
    sal_Bool bAutomatic )
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());

    FPEntry aNewFPEntry;
    aNewFPEntry.m_xParaIterator = xFlatParaIterator;
    aNewFPEntry.m_xPara = xFlatPara;
    aNewFPEntry.m_nDocID = nDocID;
    aNewFPEntry.m_nStartIndex = nStartIndex;
    aNewFPEntry.m_bAutomatic = bAutomatic;

    // add new entry to the end of this queue
    m_aFPQueue.push_back(aNewFPEntry);

    // wake up the thread in order to do grammar checking
    m_aCondition.set();
}

//doGrammarChecking_Impl
void GrammarCheckingIterator::doGrammarChecking_Impl(
    sal_Int32 nDocId,
    const uno::Reference< text::XFlatParagraph > & xFlatPara,
    const lang::Locale & rLocale,
    sal_Int32 nStartOfSentencePos,
    sal_Int32 nSuggestedSentenceEndPos)
{
    //check the locale
    /*if(m_aGCLocales.size() == 0)
    {
        GetAvailableGCSvcs_Impl();
    }
    for (GCLocales_t::const_iterator it = m_aGCLocales.begin(); it != m_aGCLocales.end(); ++ it)
    {
        uno::Sequence< lang::Locale > aLocales = it->second;
        for (sal_Int32 i=0; i<aLocales.getLength(); ++i)
        {
            if( MsLangId::convertLocaleToLanguage(aLocales[i]) == MsLangId::convertLocaleToLanguage(rLocale))
            {
                GrammarChecker* pGC=new GrammarChecker();
                pGC->addGrammarCheckingResultListener((XGrammarCheckingResultListener *) this);
                pGC->doGrammarChecking(nDocId, xFlatPara, rLocale, nStartOfSentencePos, nSuggestedSentenceEndPos);//rFlatParaText
                break;
            }
        }
    }*/
    GrammarChecker* pGC=new GrammarChecker();
    pGC->addGrammarCheckingResultListener((XGrammarCheckingResultListener *) this);
    pGC->doGrammarChecking(nDocId, xFlatPara, rLocale, nStartOfSentencePos, nSuggestedSentenceEndPos);//rFlatParaText

}


//get every element from the queue and check it
void GrammarCheckingIterator::dequeueAndCheck()
{
    // ---- THREAD SAFE START ----
    osl::ResettableGuard< osl::Mutex > aGuard(GetMutex());
    bool bEnd( m_bEnd );
    aGuard.clear();
    // ---- THREAD SAFE END ----
    while( !bEnd )
    {
        // ---- THREAD SAFE START ----
        aGuard.reset();
        bool bQueueEmpty = m_aFPQueue.empty();
        aGuard.clear();
        // ---- THREAD SAFE END ----

        if( !bQueueEmpty )
        {
            // ---- THREAD SAFE START ----
            aGuard.reset();

            FPEntry aFPEntryItem = m_aFPQueue.front();

            uno::Reference< text::XFlatParagraphIterator > xFPIterator( uno::Reference< text::XFlatParagraphIterator >( aFPEntryItem.m_xParaIterator ), uno::UNO_QUERY);

            uno::Reference< text::XFlatParagraph > xFlatPara( uno::Reference< text::XFlatParagraph >( aFPEntryItem.m_xPara ), uno::UNO_QUERY );
            if(xFlatPara.is())
            {
                OUString aCurTxt = xFlatPara->getText();
                sal_Int32 nEndPos=aCurTxt.getLength();
                lang::Locale aCurLocale=getPrimaryLanguageOfSentence(xFlatPara, 0);
                sal_Int32 nCurDocID=aFPEntryItem.m_nDocID;

                sal_Int32 nStartPos=aFPEntryItem.m_nStartIndex;
                while(nStartPos<nEndPos)
                {

                    sal_Int32 nCurEndPos=getSuggestedEndOfSentence( aCurTxt, nStartPos, aCurLocale);
                    ////////////////////////////////////////////////
                    //..............doGrammarChecking...............
                    ////////////////////////////////////////////////
                    doGrammarChecking_Impl(nCurDocID, xFlatPara, aCurLocale, nStartPos, nCurEndPos);

                    nStartPos=nCurEndPos+1;

                    //update the startIndex of the entry
                    aFPEntryItem.m_nStartIndex=nStartPos;
                    m_aFPQueue.pop_front();
                    m_aFPQueue.push_front(aFPEntryItem);
                }
                //remove the first element from the queue
                m_aFPQueue.pop_front();



                if(xFlatPara->isModified( ))
                {
                    //there is something unprocessed left in the current flat paragraph
                    //add this flat paragraph to the end of the queue
                    AddEntry( xFPIterator, xFlatPara, nCurDocID, 0, aFPEntryItem.m_bAutomatic );
                }
                else
                {
                    //set "alread checked" flag for the current flat paragraph
                    //get the next flat paragraph to create a new entry

                    xFlatPara->setChecked(text::TextMarkupType::GRAMMAR, true);
                    uno::Reference< text::XFlatParagraph > xFlatParaNext = xFPIterator->getNextPara( );//getParaAfter( xFlatPara );//
                    if( xFlatParaNext.is() )
                    {
                        AddEntry(xFPIterator, xFlatParaNext, nCurDocID, 0, aFPEntryItem.m_bAutomatic);
                    }
                }
            }
            aGuard.clear();
            // ---- THREAD SAFE END ----
        }
        else
        {
            // ---- THREAD SAFE START ----
            aGuard.reset();
            // Check queue state again
            if ( m_aFPQueue.empty() )
                m_aCondition.reset();
            aGuard.clear();
            // ---- THREAD SAFE END ----

            //if the queue is empty
            // IMPORTANT: Don't call condition.wait() with locked
            // mutex. Otherwise you would keep out other threads
            // to add entries to the queue! A condition is thread-
            // safe implemented.
            m_aCondition.wait();
        }

        // ---- THREAD SAFE START ----
        aGuard.reset();
        bEnd = m_bEnd;
        aGuard.clear();
        // ---- THREAD SAFE END ----
    }
}


// linguistic2::XGrammarCheckingIterator:
void SAL_CALL GrammarCheckingIterator::startGrammarChecking(
        const uno::Reference< ::uno::XInterface > & xDoc,
        const uno::Reference< text::XFlatParagraphIteratorProvider > & xIteratorProvider,
        ::sal_Bool bAutomatic)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    (void) xDoc;

    /******************************************************************
              Add a returned value for this function????
    *******************************************************************/
    // TODO: Insert your implementation for "startGrammarChecking" here.

    // Create new entry and push it to the global queue
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    uno::Reference<text::XFlatParagraphIterator> xFPIterator=
        xIteratorProvider->getFlatParagraphIterator(
            text::TextMarkupType::GRAMMAR,bAutomatic);

    uno::Reference< text::XFlatParagraph > xPara= xFPIterator->getFirstPara();
    if(xPara.is())
    {
        ::sal_Int32 nDocID = NextDocId();
        m_aDocMap[xDoc]=nDocID;
        AddEntry(xFPIterator, xPara, nDocID, 0, bAutomatic);
    }
}

void SAL_CALL GrammarCheckingIterator::checkGrammarAtPos(
        const uno::Reference< ::uno::XInterface > & xDoc,
        const uno::Reference< text::XFlatParagraph > & xStartPara,
        sal_Int32 nPosInPara)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{

    // TODO: Insert your implementation for "checkGrammarAtPos" here.

    //for the context menu
    OUString aTxt = xStartPara->getText();
    //sal_Int32 nEndPos=rTxt.getLength();
    lang::Locale aLocale=getPrimaryLanguageOfSentence(xStartPara, nPosInPara);
    uno::Reference< i18n::XBreakIterator > xBreakIterator;
    if(!xBreakIterator.is())
    {
        xBreakIterator=vcl::unohelper::CreateBreakIterator();
    }
    if(xBreakIterator.is())
    {
        sal_Int32 nStartPosition=xBreakIterator->beginOfSentence(aTxt, nPosInPara, aLocale);;
        sal_Int32 nEndPosition=xBreakIterator->endOfSentence(aTxt, nStartPosition, aLocale);

        ////////////////////////////////////////////////
        //..............doGrammarChecking...............
        ////////////////////////////////////////////////

        sal_Int32 nDocID=m_aDocMap[xDoc];
        doGrammarChecking_Impl(nDocID, xStartPara, aLocale, nStartPosition, nEndPosition);//aTxt
    }
}
sal_Int32 GrammarCheckingIterator::getSuggestedEndOfSentence( const OUString aText, sal_Int32 nSentenceStartPos, const lang::Locale aLocale )
{
    // TODO: Exchange the default return implementation for "getEndOfSentence" !!!
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
        nEndPosition=xBreakIterator->endOfSentence(aText, nSentenceStartPos, aLocale);
    }
    return nEndPosition;
}

sal_Int32 SAL_CALL GrammarCheckingIterator::getEndOfSentence( sal_Int32 nSentenceStartPos )
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    // TODO: Exchange the default return implementation for "getEndOfSentence" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.

    (void)nSentenceStartPos;
    /*uno::Reference< i18n::XBreakIterator > xBreakIterator;
    if(!xBreakIterator.is())
    {
        xBreakIterator=vcl::unohelper::CreateBreakIterator();
    }
    sal_Int32 nEndPosition=0;
    if(xBreakIterator.is())
    {

        struct FPEntry aNewFPEntry=m_aFPQueue.front();
            uno::Reference< text::XFlatParagraph > xFlatPara( uno::Reference< text::XFlatParagraph >( aNewFPEntry.xPara ), uno::UNO_QUERY );
            if(xFlatPara.is())
            {
                OUString aCurTxt = xFlatPara->getText();
                lang::Locale aCurLocale=getPrimaryLanguageOfSentence(xFlatPara, 0);
                nEndPosition=xBreakIterator->endOfSentence(aCurTxt, nSentenceStartPos, aCurLocale);
            }
    }
    return nEndPosition;*/
    return 0;

}

::sal_Bool SAL_CALL GrammarCheckingIterator::isChecking( const uno::Reference< ::uno::XInterface > & xDoc )
    throw (uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "isChecking" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    sal_Bool bCheck=sal_False;
    if(m_aDocMap.find(xDoc) != m_aDocMap.end())
    {
        sal_Int32 nDocID = m_aDocMap[xDoc];
        if(!m_aFPQueue.empty())
        {
            sal_Int32 nSize = m_aFPQueue.size();
            for( sal_Int32 i=0; i < nSize; ++i )
            {
                FPEntry aNewFPEntry = m_aFPQueue[i];
                if( nDocID == aNewFPEntry.m_nDocID )
                {
                    return true;
                }
            }
        }
    }
    return bCheck;

}

// linguistic2::XGrammarCheckingResultListener:
::sal_Bool SAL_CALL GrammarCheckingIterator::GrammarCheckingFinished( const linguistic2::GrammarCheckingResult & aRes )
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    // TODO: Exchange the default return implementation for "GrammarCheckingFinished" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.


    //mark the error sign
    uno::Sequence< linguistic2::SingleGrammarError > aErrors=aRes.aGrammarErrors;
    if(aErrors.getLength()>0)
    {
        for( sal_Int16 i=0; i < aErrors.getLength(); ++i)
        {

            linguistic2::SingleGrammarError aError = aErrors[aErrors.getLength()-1-i];
            ///////////////////////////////////////////////////////////////////////////////////////////////
            //spelling checkers and grammar checkers spontaneously simultaneously check the same sentence//
            //linguistic::GetLinguProperties()????????????????????????????????????????????????????????????
            ///////////////////////////////////////////////////////////////////////////////////////////////

            uno::Sequence< beans::PropertyValue > aAttributes;
            OUString aIdentifier;
            uno::Reference< container::XStringKeyMap > xStringKeyMap;
            //display the grammar error mark
            aRes.xPara->commitTextMarkup(aError.nErrorType, aIdentifier ,aError.nErrorStart ,aError.nErrorLen, xStringKeyMap);

        }
    }

    sal_Bool bResult=aRes.nEndOfSentencePos < aRes.xPara->getText().getLength();
    aRes.xPara->setChecked(text::TextMarkupType::GRAMMAR, bResult);
    return bResult;
}

/*// linguistic2::XGrammarCheckingResultListener:
void SAL_CALL GrammarCheckingIterator::processGrammarCheckingResult(
        com::sun::star::linguistic2::GrammarCheckingResult aRes,
        sal_Bool bLastInCurrentPara)
    throw (uno::RuntimeException)
{
    (void) aRes;
    (void) bLastInCurrentPara;
}*/

void SAL_CALL GrammarCheckingIterator::dispose(  ) throw (uno::RuntimeException)
{

}

void SAL_CALL GrammarCheckingIterator::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    (void) xListener;
}

void SAL_CALL GrammarCheckingIterator::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    (void) xListener;
}

const void GrammarCheckingIterator::GetAvailableGCSvcs_Impl()
{
    uno::Reference< lang::XMultiServiceFactory >  xFac( getProcessServiceFactory() );
    if (xFac.is())
    {
        uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
        uno::Reference< container::XEnumeration > xEnum;
        if (xEnumAccess.is())
            xEnum = xEnumAccess->createContentEnumeration(
                    A2OU( "com.sun.star.lingu2.GrammarChecker" ) );//SN_SPELLCHECKER

        if (xEnum.is())
        {
            while (xEnum->hasMoreElements())
            {
                uno::Any aCurrent = xEnum->nextElement();
                uno::Reference< lang::XSingleServiceFactory > xFactory;

                if (!::cppu::extractInterface( xFactory, aCurrent ))
                    continue;

                uno::Reference< linguistic2::XGrammarChecker > xSvc( xFactory->createInstance(), uno::UNO_QUERY );
                if (xSvc.is())
                {
                    OUString            aImplName;
                    //uno::Sequence< INT16 >    aLanguages;
                    uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                    if (xInfo.is())
                        aImplName = xInfo->getImplementationName();
                    DBG_ASSERT( aImplName.getLength(),
                            "empty implementation name" );
                    uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                    DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                    if (xSuppLoc.is()) {
                        uno::Sequence< lang::Locale > aLocaleSequence(xSuppLoc->getLocales());
                        m_aGCLocales[aImplName]=aLocaleSequence;
                        //aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                    }

                    //pAvailSpellSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                    //                  pAvailSpellSvcs->Count() );
                    //m_aGCLocales[aImplName]=aLocaleSequence;//aLanguages;
                }
            }
        }
    }
}

sal_Bool SAL_CALL GrammarCheckingIterator::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());

    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( INT32 i = 0; i < aSNL.getLength(); ++i )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;
}

uno::Sequence< OUString > GrammarCheckingIterator::getSupportedServiceNames_Static() throw()
{
    //osl::Guard< osl::Mutex > aGuard(GetMutex());

    uno::Sequence< OUString > aSNS( 1 );    // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = A2OU( "com.sun.star.linguistic2.GrammarCheckingIterator" );//SN_LINGU_SERVCICE_MANAGER
    return aSNS;
}

uno::Sequence< OUString > SAL_CALL GrammarCheckingIterator::getSupportedServiceNames() throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    return getSupportedServiceNames_Static();
}

OUString SAL_CALL GrammarCheckingIterator::getImplementationName() throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard(GetMutex());
    return getImplementationName_Static();
}

sal_Bool SAL_CALL GrammarCheckingIterator_writeInfo( void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += GrammarCheckingIterator::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        uno::Reference< registry::XRegistryKey > xNewKey =
            pRegistryKey->createKey( aImpl );
        uno::Sequence< OUString > aServices = GrammarCheckingIterator::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); ++i )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    catch(uno::Exception &)
    {
        return sal_False;
    }
}

uno::Reference< uno::XInterface > SAL_CALL GrammarCheckingIterator_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory > & /*rSMgr*/ )
    throw(uno::Exception)
{
    uno::Reference< uno::XInterface > xService = (cppu::OWeakObject*) new GrammarCheckingIterator;
    return xService;
}

void * SAL_CALL GrammarCheckingIterator_getFactory(
    const sal_Char * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    void * /*pRegistryKey*/ )
{

    void * pRet = 0;
    if ( !GrammarCheckingIterator::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                GrammarCheckingIterator::getImplementationName_Static(),
                GrammarCheckingIterator_CreateInstance,
                GrammarCheckingIterator::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}


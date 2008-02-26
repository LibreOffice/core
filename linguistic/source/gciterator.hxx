/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gciterator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 09:49:32 $
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

#ifndef _LINGUISTIC_GRAMMARCHECKINGITERATOR_HXX_
#define _LINGUISTIC_GRAMMARCHECKINGITERATOR_HXX_


#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/linguistic2/XGrammarCheckingIterator.hpp>
#include <com/sun/star/linguistic2/XGrammarCheckingResultListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/weakref.hxx>

#include <map>
#include <deque>


class GrammarCheckingIterator:
    public cppu::WeakImplHelper4
    <
        ::com::sun::star::linguistic2::XGrammarCheckingIterator,
        ::com::sun::star::linguistic2::XGrammarCheckingResultListener,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{
//    uno::Reference< uno::XComponentContext >    m_xContext;

    //the struct for every entry stored in the queue
    //FlatParagraphEntry
    struct FPEntry
    {
        // flat paragraph iterator
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraphIterator > m_xParaIterator;
        // flat paragraph
        ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraph > m_xPara;
        // document ID to identify different documents
        ::sal_Int32     m_nDocID;
        // the starting position to be checked
        ::sal_Int32     m_nStartIndex;
        // the flag to identify whether the document does automatical grammar checking
        ::sal_Bool      m_bAutomatic;

        FPEntry() :
            m_nDocID( 0 ),
            m_nStartIndex( 0 ),
            m_bAutomatic( 0 )
        {
        }
    };


    //the queue is keeping track of all senteces to be checked
    //every element of this queue is a FlatParagraphEntry struct-object
    //FlatParagraphQueue_t
    typedef std::deque< FPEntry > FPQueue_t;

    //m_aFlatParagraphQueue
    FPQueue_t m_aFPQueue;

    // the flag to end the endless loop
    ::sal_Bool m_bEnd;

    //m_aDocumentInterfaceMap
    //parameter ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > -->the document
    //parameter sal_Int32 -->DocID to indentify the document
    typedef std::map< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >, ::sal_Int32 > DocMap_t;
    DocMap_t m_aDocMap;

    //m_aGrammarCheckerLocales is mapping all existing grammar checkers
    //parameter ::rtl::OUString -->implementation name
    //parameter ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > -->
    //    a list of languages in which every grammar checker could check grammar errors
    typedef std::map< ::rtl::OUString, ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > > GCLocales_t;
    GCLocales_t m_aGCLocales;

    osl::Condition m_aCondition;

    ::sal_Int32 m_nDocID;

    const ::sal_Int32 NextDocId();



    void doGrammarChecking_Impl( ::sal_Int32 nDocId,
        const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraph >& xFlatPara,
        const ::com::sun::star::lang::Locale & rLocale,
        ::sal_Int32 nStartOfSentencePos,
        ::sal_Int32 nSuggestedSentenceEndPos);

    void AddEntry(
        ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraphIterator > xFlatParaIterator,
        ::com::sun::star::uno::WeakReference< ::com::sun::star::text::XFlatParagraph > xFlatPara,
        ::sal_Int32 nDocID, ::sal_Int32 nStartIndex, ::sal_Bool bAutomatic );

    sal_Int32 getSuggestedEndOfSentence( const ::rtl::OUString aText, sal_Int32 nSentenceStartPos, const ::com::sun::star::lang::Locale aLocale );

    const void GetAvailableGCSvcs_Impl();
    // disallow use of copy c-tor and assignment operator
    GrammarCheckingIterator( const GrammarCheckingIterator & );
    GrammarCheckingIterator & operator = ( const GrammarCheckingIterator & );

public:
    void dequeueAndCheck();
    explicit GrammarCheckingIterator( /* com::sun::star::uno::Reference< uno::XComponentContext > const & rXContext */ );
    virtual ~GrammarCheckingIterator();

    // XGrammarCheckingIterator

    virtual void SAL_CALL startGrammarChecking( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDoc, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraphIteratorProvider >& xIteratorProvider, ::sal_Bool bAutomatic ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL checkGrammarAtPos( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDoc, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraph >& xStartPara, ::sal_Int32 nPosInPara ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getEndOfSentence( ::sal_Int32 nSentenceStartPos ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isChecking( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xDoc ) throw (::com::sun::star::uno::RuntimeException);


    // XGrammarCheckingResultListener
    virtual ::sal_Bool SAL_CALL GrammarCheckingFinished( const ::com::sun::star::linguistic2::GrammarCheckingResult& aRes ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //virtual void SAL_CALL processGrammarCheckingResult( ::com::sun::star::linguistic2::GrammarCheckingResult aRes, ::sal_Bool bLastInCurrentPara )throw (::com::sun::staruno::RuntimeException);


    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);




    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    static inline ::rtl::OUString getImplementationName_Static();

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static() throw();

    inline ::osl::Mutex & GetMutex()
    {
        static osl::Mutex aMutex;
        return aMutex;
    }

};

inline ::rtl::OUString GrammarCheckingIterator::getImplementationName_Static()
{
    return A2OU( "com.sun.star.lingu2.GrammarCheckingIterator" );
}

#endif


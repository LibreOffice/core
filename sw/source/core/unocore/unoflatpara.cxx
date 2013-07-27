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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <editeng/unolingu.hxx>

#include <unobaseclass.hxx>
#include <unoflatpara.hxx>

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <unotextmarkup.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <viewimp.hxx>
#include <breakit.hxx>
#include <pam.hxx>
#include <unobaseclass.hxx>
#include <unotextrange.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <IGrammarContact.hxx>
#include <viewopt.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/text/XTextRange.hpp>

using namespace ::com::sun::star;

/******************************************************************************
 * SwXFlatParagraph
 ******************************************************************************/

SwXFlatParagraph::SwXFlatParagraph( SwTxtNode& rTxtNode, rtl::OUString aExpandText, const ModelToViewHelper::ConversionMap* pMap ) :
    SwXTextMarkup( rTxtNode, pMap ),
    maExpandText( aExpandText )
{
}

SwXFlatParagraph::~SwXFlatParagraph()
{
}

uno::Sequence< uno::Type > SwXFlatParagraph::getTypes(  ) throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes = SwXTextMarkup::getTypes();
    aTypes.realloc( aTypes.getLength() + 1 );
    aTypes[aTypes.getLength()-1] = ::getCppuType((uno::Reference< text::XFlatParagraph >*)0);
    return aTypes;
}

uno::Sequence< sal_Int8 > SwXFlatParagraph::getImplementationId(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

uno::Any SAL_CALL SwXFlatParagraph::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    if ( rType == ::getCppuType((uno::Reference< text::XFlatParagraph >*)0) )
    {
        return uno::makeAny( uno::Reference < text::XFlatParagraph >(this) );
    }
    else
        return SwXTextMarkup::queryInterface( rType );
}

void SAL_CALL SwXFlatParagraph::acquire() throw()
{
    SwXTextMarkup::acquire();
}

void SAL_CALL SwXFlatParagraph::release() throw()
{
    SwXTextMarkup::release();
}

const SwTxtNode* SwXFlatParagraph::getTxtNode() const
{
    return mpTxtNode;
}

css::uno::Reference< css::container::XStringKeyMap > SAL_CALL SwXFlatParagraph::getMarkupInfoContainer() throw (css::uno::RuntimeException)
{
    return SwXTextMarkup::getMarkupInfoContainer();
}

void SAL_CALL SwXFlatParagraph::commitTextRangeMarkup(::sal_Int32 nType, const ::rtl::OUString & aIdentifier, const uno::Reference< text::XTextRange> & xRange,
                                                      const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXTextMarkup::commitTextRangeMarkup( nType, aIdentifier, xRange,  xMarkupInfoContainer );
}


void SAL_CALL SwXFlatParagraph::commitStringMarkup(::sal_Int32 nType, const ::rtl::OUString & rIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength, const css::uno::Reference< css::container::XStringKeyMap > & rxMarkupInfoContainer) throw (css::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXTextMarkup::commitStringMarkup( nType, rIdentifier, nStart, nLength,  rxMarkupInfoContainer );
}

// text::XFlatParagraph:
::rtl::OUString SAL_CALL SwXFlatParagraph::getText() throw (uno::RuntimeException)
{
    return maExpandText;
}

// text::XFlatParagraph:
void SAL_CALL SwXFlatParagraph::setChecked( ::sal_Int32 nType, ::sal_Bool bVal ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if ( mpTxtNode )
    {
        if ( text::TextMarkupType::SPELLCHECK == nType )
            mpTxtNode->SetWrongDirty( !bVal );
        else if ( text::TextMarkupType::SMARTTAG == nType )
            mpTxtNode->SetSmartTagDirty( !bVal );
        else if( text::TextMarkupType::PROOFREADING == nType )
        {
            mpTxtNode->SetGrammarCheckDirty( !bVal );
            if( bVal )
                ::finishGrammarCheck( *mpTxtNode );
        }
    }
}

// text::XFlatParagraph:
::sal_Bool SAL_CALL SwXFlatParagraph::isChecked( ::sal_Int32 nType ) throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if ( mpTxtNode )
    {
        if ( text::TextMarkupType::SPELLCHECK == nType )
            return mpTxtNode->IsWrongDirty();
        else if ( text::TextMarkupType::PROOFREADING == nType )
            return mpTxtNode->IsGrammarCheckDirty();
        else if ( text::TextMarkupType::SMARTTAG == nType )
            return mpTxtNode->IsSmartTagDirty();
    }

    return sal_False;
}

// text::XFlatParagraph:
::sal_Bool SAL_CALL SwXFlatParagraph::isModified() throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0 == mpTxtNode;
}

// text::XFlatParagraph:
lang::Locale SAL_CALL SwXFlatParagraph::getLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!mpTxtNode)
        return SvxCreateLocale( LANGUAGE_NONE );

    const lang::Locale aLocale( SW_BREAKITER()->GetLocale( mpTxtNode->GetLang( static_cast<sal_uInt16>(nPos), static_cast<sal_uInt16>(nLen) ) ) );
    return aLocale;
}

// text::XFlatParagraph:
lang::Locale SAL_CALL SwXFlatParagraph::getPrimaryLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (!mpTxtNode)
        return SvxCreateLocale( LANGUAGE_NONE );

    const lang::Locale aLocale( SW_BREAKITER()->GetLocale( mpTxtNode->GetLang( static_cast<sal_uInt16>(nPos), static_cast<sal_uInt16>(nLen) ) ) );
    return aLocale;
}

// text::XFlatParagraph:
void SAL_CALL SwXFlatParagraph::changeText(::sal_Int32 nPos, ::sal_Int32 nLen, const ::rtl::OUString & aNewText, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if ( !mpTxtNode )
        return;

    SwTxtNode* pOldTxtNode = mpTxtNode;

    SwPaM aPaM( *mpTxtNode, static_cast<sal_uInt16>(nPos), *mpTxtNode, static_cast<sal_uInt16>(nPos + nLen) );

    UnoActionContext aAction( mpTxtNode->GetDoc() );

    const uno::Reference< text::XTextRange > xRange =
        SwXTextRange::CreateXTextRange(
            *mpTxtNode->GetDoc(), *aPaM.GetPoint(), aPaM.GetMark() );
    uno::Reference< beans::XPropertySet > xPropSet( xRange, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        for ( sal_uInt16 i = 0; i < aAttributes.getLength(); ++i )
            xPropSet->setPropertyValue( aAttributes[i].Name, aAttributes[i].Value );
    }

    mpTxtNode = pOldTxtNode; // setPropertyValue() modifies this. We restore the old state.

    IDocumentContentOperations* pIDCO = mpTxtNode->getIDocumentContentOperations();
    pIDCO->ReplaceRange( aPaM, aNewText, false );

    mpTxtNode = 0;
}

// text::XFlatParagraph:
void SAL_CALL SwXFlatParagraph::changeAttributes(::sal_Int32 nPos, ::sal_Int32 nLen, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if ( !mpTxtNode )
        return;

    SwPaM aPaM( *mpTxtNode, static_cast<sal_uInt16>(nPos), *mpTxtNode, static_cast<sal_uInt16>(nPos + nLen) );

    UnoActionContext aAction( mpTxtNode->GetDoc() );

    const uno::Reference< text::XTextRange > xRange =
        SwXTextRange::CreateXTextRange(
            *mpTxtNode->GetDoc(), *aPaM.GetPoint(), aPaM.GetMark() );
    uno::Reference< beans::XPropertySet > xPropSet( xRange, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        for ( sal_uInt16 i = 0; i < aAttributes.getLength(); ++i )
            xPropSet->setPropertyValue( aAttributes[i].Name, aAttributes[i].Value );
    }

    mpTxtNode = 0;
}

// text::XFlatParagraph:
css::uno::Sequence< ::sal_Int32 > SAL_CALL SwXFlatParagraph::getLanguagePortions() throw (css::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return css::uno::Sequence< ::sal_Int32>();
}


const uno::Sequence< sal_Int8 >&
SwXFlatParagraph::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> aSeq(CreateUnoTunnelId());
    return aSeq;
}


sal_Int64 SAL_CALL
SwXFlatParagraph::getSomething(
        const uno::Sequence< sal_Int8 >& rId)
    throw (uno::RuntimeException)
{
    return sw::UnoTunnelImpl(rId, this);
}


/******************************************************************************
 * SwXFlatParagraphIterator
 ******************************************************************************/

SwXFlatParagraphIterator::SwXFlatParagraphIterator( SwDoc& rDoc, sal_Int32 nType, sal_Bool bAutomatic )
    : mpDoc( &rDoc ),
      mnType( nType ),
      mbAutomatic( bAutomatic ),
      mnCurrentNode( 0 ),
      mnStartNode( 0 ),
      mnEndNode( rDoc.GetNodes().Count() ),
      mbWrapped( sal_False )
{
    //mnStartNode = mnCurrentNode = get node from current cursor TODO!

    // register as listener and get notified when document is closed
    mpDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD )->Add(this);
}

SwXFlatParagraphIterator::~SwXFlatParagraphIterator()
{
}


void SwXFlatParagraphIterator::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    ClientModify( this, pOld, pNew );
    // check if document gets closed...
    if(!GetRegisteredIn())
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        mpDoc = 0;
    }
}


uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getFirstPara()
    throw( uno::RuntimeException )
{
    return getNextPara();   // TODO
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getNextPara()
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference< text::XFlatParagraph > xRet;
    if (!mpDoc)
        return xRet;

    SwTxtNode* pRet = 0;
    if ( mbAutomatic )
    {
        ViewShell* pViewShell = 0;
        mpDoc->GetEditShell( &pViewShell );

        SwPageFrm* pCurrentPage = pViewShell ? pViewShell->Imp()->GetFirstVisPage() : 0;
        SwPageFrm* pStartPage = pCurrentPage;
        SwPageFrm* pStopPage = 0;

        while ( pCurrentPage != pStopPage )
        {
            if (mnType != text::TextMarkupType::SPELLCHECK || pCurrentPage->IsInvalidSpelling() )
            {
                // this method is supposed to return an empty paragraph in case Online Checking is disabled
                if ( ( mnType == text::TextMarkupType::PROOFREADING || mnType == text::TextMarkupType::SPELLCHECK )
                    && !pViewShell->GetViewOptions()->IsOnlineSpell() )
                    return xRet;

                // search for invalid content:
                SwCntntFrm* pCnt = pCurrentPage->ContainsCntnt();

                while( pCnt && pCurrentPage->IsAnLower( pCnt ) )
                {
                    SwTxtNode* pTxtNode = dynamic_cast<SwTxtNode*>( pCnt->GetNode()->GetTxtNode() );

                    if ( pTxtNode &&
                        ((mnType == text::TextMarkupType::SPELLCHECK &&
                                pTxtNode->IsWrongDirty()) ||
                         (mnType == text::TextMarkupType::PROOFREADING &&
                                pTxtNode->IsGrammarCheckDirty())) )
                    {
                        pRet = pTxtNode;
                        break;
                    }

                    pCnt = pCnt->GetNextCntntFrm();
                }
            }

            if ( pRet )
                break;

            // if there is no invalid text node on the current page,
            // we validate the page
            pCurrentPage->ValidateSpelling();

            // proceed with next page, wrap at end of document if required:
            pCurrentPage = static_cast<SwPageFrm*>(pCurrentPage->GetNext());

            if ( !pCurrentPage && !pStopPage )
            {
                pStopPage = pStartPage;
                pCurrentPage = static_cast<SwPageFrm*>(pViewShell->GetLayout()->Lower());
            }
        }
    }
    else    // non-automatic checking
    {
        const SwNodes& rNodes = mpDoc->GetNodes();
        const sal_uLong nMaxNodes = rNodes.Count();

        while ( mnCurrentNode < mnEndNode && mnCurrentNode < nMaxNodes )
        {
            SwNode* pNd = rNodes[ mnCurrentNode ];

            ++mnCurrentNode;

            pRet = dynamic_cast<SwTxtNode*>(pNd);
            if ( pRet )
                break;

            if ( mnCurrentNode == mnEndNode && !mbWrapped )
            {
                mnCurrentNode = 0;
                mnEndNode = mnStartNode;
            }
        }
    }

    if ( pRet )
    {
        // Expand the string:
        rtl::OUString aExpandText;
        const ModelToViewHelper::ConversionMap* pConversionMap =
                pRet->BuildConversionMap( aExpandText );

        xRet = new SwXFlatParagraph( *pRet, aExpandText, pConversionMap );
        // keep hard references...
        m_aFlatParaList.insert( xRet );
    }

    return xRet;
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getLastPara()
    throw( uno::RuntimeException )
{
    return getNextPara();
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getParaAfter(const uno::Reference< text::XFlatParagraph > & xPara)
    throw ( uno::RuntimeException, lang::IllegalArgumentException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference< text::XFlatParagraph > xRet;
    if (!mpDoc)
        return xRet;

    const uno::Reference<lang::XUnoTunnel> xFPTunnel(xPara, uno::UNO_QUERY);
    OSL_ASSERT(xFPTunnel.is());
    SwXFlatParagraph* const pFlatParagraph(sw::UnoTunnelGetImplementation<SwXFlatParagraph>(xFPTunnel));

    if ( !pFlatParagraph )
        return xRet;

    const SwTxtNode* pCurrentNode = pFlatParagraph->getTxtNode();

    if ( !pCurrentNode )
        return xRet;

    SwTxtNode* pNextTxtNode = 0;
    const SwNodes& rNodes = pCurrentNode->GetDoc()->GetNodes();

    for( sal_uLong nCurrentNode = pCurrentNode->GetIndex() + 1; nCurrentNode < rNodes.Count(); ++nCurrentNode )
    {
        SwNode* pNd = rNodes[ nCurrentNode ];
        pNextTxtNode = dynamic_cast<SwTxtNode*>(pNd);
        if ( pNextTxtNode )
            break;
    }

    if ( pNextTxtNode )
    {
        // Expand the string:
        rtl::OUString aExpandText;
        const ModelToViewHelper::ConversionMap* pConversionMap =
                pNextTxtNode->BuildConversionMap( aExpandText );

        xRet = new SwXFlatParagraph( *pNextTxtNode, aExpandText, pConversionMap );
        // keep hard references...
        m_aFlatParaList.insert( xRet );
    }

    return xRet;
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getParaBefore(const uno::Reference< text::XFlatParagraph > & xPara )
    throw ( uno::RuntimeException, lang::IllegalArgumentException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference< text::XFlatParagraph > xRet;
    if (!mpDoc)
        return xRet;

    const uno::Reference<lang::XUnoTunnel> xFPTunnel(xPara, uno::UNO_QUERY);
    OSL_ASSERT(xFPTunnel.is());
    SwXFlatParagraph* const pFlatParagraph(sw::UnoTunnelGetImplementation<SwXFlatParagraph>(xFPTunnel));

    if ( !pFlatParagraph )
        return xRet;

    const SwTxtNode* pCurrentNode = pFlatParagraph->getTxtNode();

    if ( !pCurrentNode )
        return xRet;

    SwTxtNode* pPrevTxtNode = 0;
    const SwNodes& rNodes = pCurrentNode->GetDoc()->GetNodes();

    for( sal_uLong nCurrentNode = pCurrentNode->GetIndex() - 1; nCurrentNode > 0; --nCurrentNode )
    {
        SwNode* pNd = rNodes[ nCurrentNode ];
        pPrevTxtNode = dynamic_cast<SwTxtNode*>(pNd);
        if ( pPrevTxtNode )
            break;
    }

    if ( pPrevTxtNode )
    {
        // Expand the string:
        rtl::OUString aExpandText;
        const ModelToViewHelper::ConversionMap* pConversionMap =
                pPrevTxtNode->BuildConversionMap( aExpandText );

        xRet = new SwXFlatParagraph( *pPrevTxtNode, aExpandText, pConversionMap );
        // keep hard references...
        m_aFlatParaList.insert( xRet );
    }

    return xRet;
}

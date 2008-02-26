/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoflatpara.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 09:48:23 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _UNOFLATPARA_HXX
#include <unoflatpara.hxx>
#endif

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
#include <unoobj.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <rootfrm.hxx>

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

const SwTxtNode* SwXFlatParagraph::getTxtNode() const
{
    return mpTxtNode;
}

css::uno::Reference< css::container::XStringKeyMap > SAL_CALL SwXFlatParagraph::getMarkupInfoContainer() throw (css::uno::RuntimeException)
{
    return SwXTextMarkup::getMarkupInfoContainer();
}

void SAL_CALL SwXFlatParagraph::commitTextMarkup(::sal_Int32 nType, const ::rtl::OUString & rIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength, const css::uno::Reference< css::container::XStringKeyMap > & rxMarkupInfoContainer) throw (css::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXTextMarkup::commitTextMarkup( nType, rIdentifier, nStart, nLength,  rxMarkupInfoContainer );
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
        else if ( text::TextMarkupType::GRAMMAR == nType )
            mpTxtNode->SetGrammarCheckDirty( !bVal );
        else if ( text::TextMarkupType::SMARTTAG == nType )
            mpTxtNode->SetSmartTagDirty( !bVal );
    }
}

// text::XFlatParagraph:
::sal_Bool SAL_CALL SwXFlatParagraph::isChecked( ::sal_Int32 nType ) throw (uno::RuntimeException)
{
    if ( mpTxtNode )
    {
        if ( text::TextMarkupType::SPELLCHECK == nType )
            return mpTxtNode->IsWrongDirty();
        else if ( text::TextMarkupType::GRAMMAR == nType )
            return mpTxtNode->IsGrammarCheckDirty();
        else if ( text::TextMarkupType::SMARTTAG == nType )
            return mpTxtNode->IsSmartTagDirty();
    }

    return sal_False;
}

// text::XFlatParagraph:
::sal_Bool SAL_CALL SwXFlatParagraph::isModified() throw (uno::RuntimeException)
{
    return 0 == mpTxtNode;
}

// text::XFlatParagraph:
lang::Locale SAL_CALL SwXFlatParagraph::getLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    const lang::Locale aLocale( SW_BREAKITER()->GetLocale( mpTxtNode->GetLang( static_cast<USHORT>(nPos), static_cast<USHORT>(nLen) ) ) );
    return aLocale;
}

// text::XFlatParagraph:
lang::Locale SAL_CALL SwXFlatParagraph::getPrimaryLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen)
    throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    const lang::Locale aLocale( SW_BREAKITER()->GetLocale( mpTxtNode->GetLang( static_cast<USHORT>(nPos), static_cast<USHORT>(nLen) ) ) );
    return aLocale;
}

// text::XFlatParagraph:
void SAL_CALL SwXFlatParagraph::changeText(::sal_Int32 nPos, ::sal_Int32 nLen, const ::rtl::OUString & aNewText, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if ( !mpTxtNode )
        return;

    SwTxtNode* pOldTxtNode = mpTxtNode;

    SwPaM aPaM( *mpTxtNode, static_cast<USHORT>(nPos), *mpTxtNode, static_cast<USHORT>(nPos + nLen) );

    UnoActionContext aAction( mpTxtNode->GetDoc() );

    uno::Reference< text::XTextRange > xRange = SwXTextRange::CreateTextRangeFromPosition( mpTxtNode->GetDoc(), *aPaM.GetPoint(), aPaM.GetMark() );
    uno::Reference< beans::XPropertySet > xPropSet( xRange, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        for ( USHORT i = 0; i < aAttributes.getLength(); ++i )
            xPropSet->setPropertyValue( aAttributes[i].Name, aAttributes[i].Value );
    }

    mpTxtNode = pOldTxtNode; // setPropertyValue() modifies this. We restore the old state.

    IDocumentContentOperations* pIDCO = mpTxtNode->getIDocumentContentOperations();
    pIDCO->Replace( aPaM, aNewText, false );

    mpTxtNode = 0;
}

// text::XFlatParagraph:
void SAL_CALL SwXFlatParagraph::changeAttributes(::sal_Int32 nPos, ::sal_Int32 nLen, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if ( !mpTxtNode )
        return;

    SwPaM aPaM( *mpTxtNode, static_cast<USHORT>(nPos), *mpTxtNode, static_cast<USHORT>(nPos + nLen) );

    UnoActionContext aAction( mpTxtNode->GetDoc() );

    uno::Reference< text::XTextRange > xRange = SwXTextRange::CreateTextRangeFromPosition( mpTxtNode->GetDoc(), *aPaM.GetPoint(), aPaM.GetMark() );
    uno::Reference< beans::XPropertySet > xPropSet( xRange, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        for ( USHORT i = 0; i < aAttributes.getLength(); ++i )
            xPropSet->setPropertyValue( aAttributes[i].Name, aAttributes[i].Value );
    }

    mpTxtNode = 0;
}

// text::XFlatParagraph:
css::uno::Sequence< ::sal_Int32 > SAL_CALL SwXFlatParagraph::getLanguagePortions() throw (css::uno::RuntimeException)
{
    return css::uno::Sequence< ::sal_Int32>();
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
}

SwXFlatParagraphIterator::~SwXFlatParagraphIterator()
{
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

    uno::Reference< text::XFlatParagraph > xRet = 0;
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
            if ( pCurrentPage->IsInvalidSpelling() )
            {
                // search for invalid content:
                SwCntntFrm* pCnt = pCurrentPage->ContainsCntnt();

                while( pCnt && pCurrentPage->IsAnLower( pCnt ) )
                {
                    SwTxtNode* pTxtNode = dynamic_cast<SwTxtNode*>( pCnt->GetNode()->GetTxtNode() );

                    if ( pTxtNode &&
                        ((mnType == text::TextMarkupType::SPELLCHECK &&
                                pTxtNode->IsWrongDirty()) ||
                         (mnType == text::TextMarkupType::GRAMMAR &&
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
                pCurrentPage = static_cast<SwPageFrm*>(mpDoc->GetRootFrm()->Lower());
            }
        }
    }
    else    // non-automatic checking
    {
        const SwNodes& rNodes = mpDoc->GetNodes();
        const ULONG nMaxNodes = rNodes.Count();

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
    uno::Reference< text::XFlatParagraph > xRet;

    text::XFlatParagraph* pFP = xPara.get();
    SwXFlatParagraph* pFlatParagraph = static_cast<SwXFlatParagraph*>(pFP);

    if ( !pFlatParagraph )
        return xRet;

    const SwTxtNode* pCurrentNode = pFlatParagraph->getTxtNode();

    if ( !pCurrentNode )
        return xRet;

    SwTxtNode* pNextTxtNode = 0;
    const SwNodes& rNodes = pCurrentNode->GetDoc()->GetNodes();

    for( ULONG nCurrentNode = pCurrentNode->GetIndex() + 1; nCurrentNode < rNodes.Count(); ++nCurrentNode )
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
    uno::Reference< text::XFlatParagraph > xRet;

    text::XFlatParagraph* pFP = xPara.get();
    SwXFlatParagraph* pFlatParagraph = static_cast<SwXFlatParagraph*>(pFP);

    if ( !pFlatParagraph )
        return xRet;

    const SwTxtNode* pCurrentNode = pFlatParagraph->getTxtNode();

    if ( !pCurrentNode )
        return xRet;

    SwTxtNode* pPrevTxtNode = 0;
    const SwNodes& rNodes = pCurrentNode->GetDoc()->GetNodes();

    for( ULONG nCurrentNode = pCurrentNode->GetIndex() - 1; nCurrentNode > 0; --nCurrentNode )
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


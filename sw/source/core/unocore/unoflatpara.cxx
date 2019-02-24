/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <editeng/unolingu.hxx>

#include <unobaseclass.hxx>
#include <unocrsrhelper.hxx>
#include <unoflatpara.hxx>

#include <vcl/svapp.hxx>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <unotextmarkup.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <viewsh.hxx>
#include <viewimp.hxx>
#include <breakit.hxx>
#include <pam.hxx>
#include <unotextrange.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <IGrammarContact.hxx>
#include <viewopt.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/text/XTextRange.hpp>

using namespace ::com::sun::star;

namespace SwUnoCursorHelper {

uno::Reference<text::XFlatParagraphIterator>
CreateFlatParagraphIterator(SwDoc & rDoc, sal_Int32 const nTextMarkupType,
        bool const bAutomatic)
{
    return new SwXFlatParagraphIterator(rDoc, nTextMarkupType, bAutomatic);
}

}

SwXFlatParagraph::SwXFlatParagraph( SwTextNode& rTextNode, const OUString& aExpandText, const ModelToViewHelper& rMap )
    : SwXFlatParagraph_Base(& rTextNode, rMap)
    , maExpandText(aExpandText)
{
}

SwXFlatParagraph::~SwXFlatParagraph()
{
}


// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXFlatParagraph::getPropertySetInfo()
{
    static comphelper::PropertyMapEntry s_Entries[] = {
        { OUString("FieldPositions"), -1, ::cppu::UnoType<uno::Sequence<sal_Int32>>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString("FootnotePositions"), -1, ::cppu::UnoType<uno::Sequence<sal_Int32>>::get(), beans::PropertyAttribute::READONLY, 0 },
        { OUString(), -1, css::uno::Type(), 0, 0 }
    };
    return new comphelper::PropertySetInfo(s_Entries);
}

void SAL_CALL
SwXFlatParagraph::setPropertyValue(const OUString&, const uno::Any&)
{
    throw lang::IllegalArgumentException("no values can be set",
            static_cast< ::cppu::OWeakObject*>(this), 0);
}

uno::Any SAL_CALL
SwXFlatParagraph::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard g;

    if (rPropertyName == "FieldPositions")
    {
        return uno::makeAny( comphelper::containerToSequence( GetConversionMap().getFieldPositions() ) );
    }
    else if (rPropertyName == "FootnotePositions")
    {
        return uno::makeAny( comphelper::containerToSequence( GetConversionMap().getFootnotePositions() ) );
    }
    return uno::Any();
}

void SAL_CALL
SwXFlatParagraph::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    SAL_WARN("sw.uno",
        "SwXFlatParagraph::addPropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXFlatParagraph::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/)
{
    SAL_WARN("sw.uno",
        "SwXFlatParagraph::removePropertyChangeListener(): not implemented");
}

void SAL_CALL
SwXFlatParagraph::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    SAL_WARN("sw.uno",
        "SwXFlatParagraph::addVetoableChangeListener(): not implemented");
}

void SAL_CALL
SwXFlatParagraph::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/)
{
    SAL_WARN("sw.uno",
        "SwXFlatParagraph::removeVetoableChangeListener(): not implemented");
}


css::uno::Reference< css::container::XStringKeyMap > SAL_CALL SwXFlatParagraph::getMarkupInfoContainer()
{
    return SwXTextMarkup::getMarkupInfoContainer();
}

void SAL_CALL SwXFlatParagraph::commitTextRangeMarkup(::sal_Int32 nType, const OUString & aIdentifier, const uno::Reference< text::XTextRange> & xRange,
                                                      const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer)
{
    SolarMutexGuard aGuard;
    SwXTextMarkup::commitTextRangeMarkup( nType, aIdentifier, xRange,  xMarkupInfoContainer );
}

void SAL_CALL SwXFlatParagraph::commitStringMarkup(::sal_Int32 nType, const OUString & rIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength, const css::uno::Reference< css::container::XStringKeyMap > & rxMarkupInfoContainer)
{
    SolarMutexGuard aGuard;
    SwXTextMarkup::commitStringMarkup( nType, rIdentifier, nStart, nLength,  rxMarkupInfoContainer );
}

// text::XFlatParagraph:
OUString SAL_CALL SwXFlatParagraph::getText()
{
    return maExpandText;
}

// text::XFlatParagraph:
void SAL_CALL SwXFlatParagraph::setChecked( ::sal_Int32 nType, sal_Bool bVal )
{
    SolarMutexGuard aGuard;

    if (GetTextNode())
    {
        if ( text::TextMarkupType::SPELLCHECK == nType )
        {
            GetTextNode()->SetWrongDirty(
                bVal ? SwTextNode::WrongState::DONE : SwTextNode::WrongState::TODO);
        }
        else if ( text::TextMarkupType::SMARTTAG == nType )
            GetTextNode()->SetSmartTagDirty( !bVal );
        else if( text::TextMarkupType::PROOFREADING == nType )
        {
            GetTextNode()->SetGrammarCheckDirty( !bVal );
            if( bVal )
                ::finishGrammarCheck( *GetTextNode() );
        }
    }
}

// text::XFlatParagraph:
sal_Bool SAL_CALL SwXFlatParagraph::isChecked( ::sal_Int32 nType )
{
    SolarMutexGuard aGuard;
    if (GetTextNode())
    {
        if ( text::TextMarkupType::SPELLCHECK == nType )
            return !GetTextNode()->IsWrongDirty();
        else if ( text::TextMarkupType::PROOFREADING == nType )
            return !GetTextNode()->IsGrammarCheckDirty();
        else if ( text::TextMarkupType::SMARTTAG == nType )
            return !GetTextNode()->IsSmartTagDirty();
    }

    return true;
}

// text::XFlatParagraph:
sal_Bool SAL_CALL SwXFlatParagraph::isModified()
{
    SolarMutexGuard aGuard;
    return nullptr == GetTextNode();
}

// text::XFlatParagraph:
lang::Locale SAL_CALL SwXFlatParagraph::getLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen)
{
    SolarMutexGuard aGuard;
    if (!GetTextNode())
        return LanguageTag::convertToLocale( LANGUAGE_NONE );

    const lang::Locale aLocale( SW_BREAKITER()->GetLocale( GetTextNode()->GetLang(nPos, nLen) ) );
    return aLocale;
}

// text::XFlatParagraph:
lang::Locale SAL_CALL SwXFlatParagraph::getPrimaryLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen)
{
    SolarMutexGuard aGuard;

    if (!GetTextNode())
        return LanguageTag::convertToLocale( LANGUAGE_NONE );

    const lang::Locale aLocale( SW_BREAKITER()->GetLocale( GetTextNode()->GetLang(nPos, nLen) ) );
    return aLocale;
}

// text::XFlatParagraph:
void SAL_CALL SwXFlatParagraph::changeText(::sal_Int32 nPos, ::sal_Int32 nLen, const OUString & aNewText, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes)
{
    SolarMutexGuard aGuard;

    if (!GetTextNode())
        return;

    SwTextNode *const pOldTextNode = GetTextNode();

    if (nPos < 0 || pOldTextNode->Len() < nPos || nLen < 0 || static_cast<sal_uInt32>(pOldTextNode->Len()) < static_cast<sal_uInt32>(nPos) + nLen)
    {
        throw lang::IllegalArgumentException();
    }

    SwPaM aPaM( *GetTextNode(), nPos, *GetTextNode(), nPos+nLen );

    UnoActionContext aAction( GetTextNode()->GetDoc() );

    const uno::Reference< text::XTextRange > xRange =
        SwXTextRange::CreateXTextRange(
            *GetTextNode()->GetDoc(), *aPaM.GetPoint(), aPaM.GetMark() );
    uno::Reference< beans::XPropertySet > xPropSet( xRange, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        for ( sal_Int32 i = 0; i < aAttributes.getLength(); ++i )
            xPropSet->setPropertyValue( aAttributes[i].Name, aAttributes[i].Value );
    }

    IDocumentContentOperations& rIDCO = pOldTextNode->getIDocumentContentOperations();
    rIDCO.ReplaceRange( aPaM, aNewText, false );

    ClearTextNode(); // TODO: is this really needed?
}

// text::XFlatParagraph:
void SAL_CALL SwXFlatParagraph::changeAttributes(::sal_Int32 nPos, ::sal_Int32 nLen, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes)
{
    SolarMutexGuard aGuard;

    if (!GetTextNode())
        return;

    if (nPos < 0 || GetTextNode()->Len() < nPos || nLen < 0 || static_cast<sal_uInt32>(GetTextNode()->Len()) < static_cast<sal_uInt32>(nPos) + nLen)
    {
        throw lang::IllegalArgumentException();
    }

    SwPaM aPaM( *GetTextNode(), nPos, *GetTextNode(), nPos+nLen );

    UnoActionContext aAction( GetTextNode()->GetDoc() );

    const uno::Reference< text::XTextRange > xRange =
        SwXTextRange::CreateXTextRange(
            *GetTextNode()->GetDoc(), *aPaM.GetPoint(), aPaM.GetMark() );
    uno::Reference< beans::XPropertySet > xPropSet( xRange, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        for ( sal_Int32 i = 0; i < aAttributes.getLength(); ++i )
            xPropSet->setPropertyValue( aAttributes[i].Name, aAttributes[i].Value );
    }

    ClearTextNode(); // TODO: is this really needed?
}

// text::XFlatParagraph:
css::uno::Sequence< ::sal_Int32 > SAL_CALL SwXFlatParagraph::getLanguagePortions()
{
    return css::uno::Sequence< ::sal_Int32>();
}

namespace
{
    class theSwXFlatParagraphUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXFlatParagraphUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 >&
SwXFlatParagraph::getUnoTunnelId()
{
    return theSwXFlatParagraphUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXFlatParagraph::getSomething(
        const uno::Sequence< sal_Int8 >& rId)
{
    return sw::UnoTunnelImpl(rId, this);
}

SwXFlatParagraphIterator::SwXFlatParagraphIterator( SwDoc& rDoc, sal_Int32 nType, bool bAutomatic )
    : mpDoc( &rDoc ),
      mnType( nType ),
      mbAutomatic( bAutomatic ),
      mnCurrentNode( 0 ),
      mnEndNode( rDoc.GetNodes().Count() )
{
    //mnStartNode = mnCurrentNode = get node from current cursor TODO!

    // register as listener and get notified when document is closed
    StartListening(mpDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_STANDARD )->GetNotifier());
}

SwXFlatParagraphIterator::~SwXFlatParagraphIterator()
{
    SolarMutexGuard aGuard;
    EndListeningAll();
}

void SwXFlatParagraphIterator::Notify( const SfxHint& rHint )
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        SolarMutexGuard aGuard;
        mpDoc = nullptr;
    }
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getFirstPara()
{
    return getNextPara();   // TODO
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getNextPara()
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XFlatParagraph > xRet;
    if (!mpDoc)
        return xRet;

    SwTextNode* pRet = nullptr;
    if ( mbAutomatic )
    {
        SwViewShell* pViewShell = mpDoc->getIDocumentLayoutAccess().GetCurrentViewShell();

        SwPageFrame* pCurrentPage = pViewShell ? pViewShell->Imp()->GetFirstVisPage(pViewShell->GetOut()) : nullptr;
        SwPageFrame* pStartPage = pCurrentPage;
        SwPageFrame* pStopPage = nullptr;

        while ( pCurrentPage && pCurrentPage != pStopPage )
        {
            if (mnType != text::TextMarkupType::SPELLCHECK || pCurrentPage->IsInvalidSpelling() )
            {
                // this method is supposed to return an empty paragraph in case Online Checking is disabled
                if ( ( mnType == text::TextMarkupType::PROOFREADING || mnType == text::TextMarkupType::SPELLCHECK )
                    && !pViewShell->GetViewOptions()->IsOnlineSpell() )
                    return xRet;

                // search for invalid content:
                SwContentFrame* pCnt = pCurrentPage->ContainsContent();

                while( pCnt && pCurrentPage->IsAnLower( pCnt ) )
                {
                    if (pCnt->IsTextFrame())
                    {
                        SwTextFrame const*const pText(static_cast<SwTextFrame const*>(pCnt));
                        if (sw::MergedPara const*const pMergedPara = pText->GetMergedPara()
            )
                        {
                            SwTextNode * pTextNode(nullptr);
                            for (auto const& e : pMergedPara->extents)
                            {
                                if (e.pNode != pTextNode)
                                {
                                    pTextNode = e.pNode;
                                    if ((mnType == text::TextMarkupType::SPELLCHECK
                                            && pTextNode->IsWrongDirty()) ||
                                        (mnType == text::TextMarkupType::PROOFREADING
                                             && pTextNode->IsGrammarCheckDirty()))
                                    {
                                        pRet = pTextNode;
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            SwTextNode const*const pTextNode(pText->GetTextNodeFirst());
                            if ((mnType == text::TextMarkupType::SPELLCHECK
                                    && pTextNode->IsWrongDirty()) ||
                                (mnType == text::TextMarkupType::PROOFREADING
                                     && pTextNode->IsGrammarCheckDirty()))

                            {
                                pRet = const_cast<SwTextNode*>(pTextNode);
                            }
                        }

                        if (pRet)
                        {
                            break;
                        }
                    }

                    pCnt = pCnt->GetNextContentFrame();
                }
            }

            if ( pRet )
                break;

            // if there is no invalid text node on the current page,
            // we validate the page
            pCurrentPage->ValidateSpelling();

            // proceed with next page, wrap at end of document if required:
            pCurrentPage = static_cast<SwPageFrame*>(pCurrentPage->GetNext());

            if ( !pCurrentPage && !pStopPage )
            {
                pStopPage = pStartPage;
                pCurrentPage = static_cast<SwPageFrame*>(pViewShell->GetLayout()->Lower());
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

            pRet = dynamic_cast<SwTextNode*>(pNd);
            if ( pRet )
                break;

            if ( mnCurrentNode == mnEndNode )
            {
                mnCurrentNode = 0;
                mnEndNode = 0;
            }
        }
    }

    if ( pRet )
    {
        // Expand the string:
        const ModelToViewHelper aConversionMap(*pRet, mpDoc->getIDocumentLayoutAccess().GetCurrentLayout());
        const OUString& aExpandText = aConversionMap.getViewText();

        xRet = new SwXFlatParagraph( *pRet, aExpandText, aConversionMap );
        // keep hard references...
        m_aFlatParaList.insert( xRet );
    }

    return xRet;
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getLastPara()
{
    return getNextPara();
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getParaAfter(const uno::Reference< text::XFlatParagraph > & xPara)
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XFlatParagraph > xRet;
    if (!mpDoc)
        return xRet;

    const uno::Reference<lang::XUnoTunnel> xFPTunnel(xPara, uno::UNO_QUERY);
    SAL_WARN_IF(!xFPTunnel.is(), "sw.core", "invalid argument");
    SwXFlatParagraph* const pFlatParagraph(sw::UnoTunnelGetImplementation<SwXFlatParagraph>(xFPTunnel));

    if ( !pFlatParagraph )
        return xRet;

    SwTextNode const*const pCurrentNode = pFlatParagraph->GetTextNode();

    if ( !pCurrentNode )
        return xRet;

    SwTextNode* pNextTextNode = nullptr;
    const SwNodes& rNodes = pCurrentNode->GetDoc()->GetNodes();

    for( sal_uLong nCurrentNode = pCurrentNode->GetIndex() + 1; nCurrentNode < rNodes.Count(); ++nCurrentNode )
    {
        SwNode* pNd = rNodes[ nCurrentNode ];
        pNextTextNode = dynamic_cast<SwTextNode*>(pNd);
        if ( pNextTextNode )
            break;
    }

    if ( pNextTextNode )
    {
        // Expand the string:
        const ModelToViewHelper aConversionMap(*pNextTextNode, mpDoc->getIDocumentLayoutAccess().GetCurrentLayout());
        const OUString& aExpandText = aConversionMap.getViewText();

        xRet = new SwXFlatParagraph( *pNextTextNode, aExpandText, aConversionMap );
        // keep hard references...
        m_aFlatParaList.insert( xRet );
    }

    return xRet;
}

uno::Reference< text::XFlatParagraph > SwXFlatParagraphIterator::getParaBefore(const uno::Reference< text::XFlatParagraph > & xPara )
{
    SolarMutexGuard aGuard;

    uno::Reference< text::XFlatParagraph > xRet;
    if (!mpDoc)
        return xRet;

    const uno::Reference<lang::XUnoTunnel> xFPTunnel(xPara, uno::UNO_QUERY);

    SAL_WARN_IF(!xFPTunnel.is(), "sw.core", "invalid argument");
    SwXFlatParagraph* const pFlatParagraph(sw::UnoTunnelGetImplementation<SwXFlatParagraph>(xFPTunnel));

    if ( !pFlatParagraph )
        return xRet;

    SwTextNode const*const pCurrentNode = pFlatParagraph->GetTextNode();

    if ( !pCurrentNode )
        return xRet;

    SwTextNode* pPrevTextNode = nullptr;
    const SwNodes& rNodes = pCurrentNode->GetDoc()->GetNodes();

    for( sal_uLong nCurrentNode = pCurrentNode->GetIndex() - 1; nCurrentNode > 0; --nCurrentNode )
    {
        SwNode* pNd = rNodes[ nCurrentNode ];
        pPrevTextNode = dynamic_cast<SwTextNode*>(pNd);
        if ( pPrevTextNode )
            break;
    }

    if ( pPrevTextNode )
    {
        // Expand the string:
        const ModelToViewHelper aConversionMap(*pPrevTextNode, mpDoc->getIDocumentLayoutAccess().GetCurrentLayout());
        const OUString& aExpandText = aConversionMap.getViewText();

        xRet = new SwXFlatParagraph( *pPrevTextNode, aExpandText, aConversionMap );
        // keep hard references...
        m_aFlatParaList.insert( xRet );
    }

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

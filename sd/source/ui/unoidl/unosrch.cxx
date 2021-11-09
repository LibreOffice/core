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

#include <memory>
#include <sal/config.h>

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vcl/svapp.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>
#include <editeng/unoipset.hxx>
#include <editeng/unotext.hxx>
#include <tools/debug.hxx>

#include <unoprnms.hxx>
#include <unosrch.hxx>

using namespace ::com::sun::star;

#define WID_SEARCH_BACKWARDS    0
#define WID_SEARCH_CASE         1
#define WID_SEARCH_WORDS        2

static const SfxItemPropertyMapEntry* ImplGetSearchPropertyMap()
{
    static const SfxItemPropertyMapEntry aSearchPropertyMap_Impl[] =
    {
        { UNO_NAME_SEARCH_BACKWARDS,  WID_SEARCH_BACKWARDS,   cppu::UnoType<bool>::get(),    0,  0 },
        { UNO_NAME_SEARCH_CASE,       WID_SEARCH_CASE,        cppu::UnoType<bool>::get(),    0,  0 },
        { UNO_NAME_SEARCH_WORDS,      WID_SEARCH_WORDS,       cppu::UnoType<bool>::get(),    0,  0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aSearchPropertyMap_Impl;
}

namespace {

class SearchContext_impl
{
    uno::Reference< drawing::XShapes > mxShapes;
    sal_Int32 mnIndex;

public:
    SearchContext_impl(uno::Reference<drawing::XShapes> const& xShapes)
        : mxShapes( xShapes ), mnIndex( -1 ) {}

    uno::Reference< drawing::XShape > firstShape()
    {
        mnIndex = -1;
        return nextShape();
    }

    uno::Reference< drawing::XShape > nextShape()
    {
        uno::Reference< drawing::XShape >  xShape;
        mnIndex++;
        if( mxShapes.is() && mxShapes->getCount() > mnIndex )
        {
            mxShapes->getByIndex( mnIndex ) >>= xShape;
        }
        return xShape;
    }
};

}

/* ================================================================= */
/** this class implements a search or replace operation on a given
    page or a given sdrobj
  */

SdUnoSearchReplaceShape::SdUnoSearchReplaceShape( drawing::XDrawPage* pPage ) noexcept
    : mpPage(pPage)
{
}

SdUnoSearchReplaceShape::~SdUnoSearchReplaceShape() noexcept
{
}

// util::XReplaceable
uno::Reference< util::XReplaceDescriptor > SAL_CALL SdUnoSearchReplaceShape::createReplaceDescriptor()
{
    return new SdUnoSearchReplaceDescriptor;
}

sal_Int32 SAL_CALL SdUnoSearchReplaceShape::replaceAll( const uno::Reference< util::XSearchDescriptor >& xDesc )
{
    SdUnoSearchReplaceDescriptor* pDescr = comphelper::getFromUnoTunnel<SdUnoSearchReplaceDescriptor>( xDesc );
    if( pDescr == nullptr )
        return 0;

    sal_Int32 nFound    = 0;

    uno::Reference< drawing::XShapes >  xShapes;
    uno::Reference< drawing::XShape >  xShape;

    std::vector<SearchContext_impl> aContexts;
    if(mpPage)
    {
        xShapes = mpPage;

        if( xShapes->getCount() )
        {
            aContexts.push_back(SearchContext_impl(xShapes));
            xShape = aContexts.back().firstShape();
        }
        else
        {
            xShapes = nullptr;
        }
    }

    while( xShape.is() )
    {
        // replace in xShape
        uno::Reference< text::XText >  xText(xShape, uno::UNO_QUERY);
        uno::Reference< text::XTextRange >  xRange = xText;
        uno::Reference< text::XTextRange >  xFound;

        while( xRange.is() )
        {
            xFound = Search( xRange, pDescr );
            if( !xFound.is() )
                break;

            xFound->setString( pDescr->getReplaceString() );
            xRange = xFound->getEnd();
            nFound++;
        }
        // done with xShape -> get next shape

        // test if it's a group
        uno::Reference< drawing::XShapes > xGroupShape( xShape, uno::UNO_QUERY );
        if( xGroupShape.is() && ( xGroupShape->getCount() > 0 ) )
        {
            aContexts.push_back(SearchContext_impl(xGroupShape));
            xShape = aContexts.back().firstShape();
        }
        else
        {
            if (!aContexts.empty())
                xShape = aContexts.back().nextShape();
            else
                xShape = nullptr;
        }

        // test parent contexts for next shape if none
        // is found in the current context
        while (!aContexts.empty() && !xShape.is())
        {
            aContexts.pop_back();
            if (!aContexts.empty())
                xShape = aContexts.back().nextShape();
        }
    }

    return nFound;
}

// XSearchable
uno::Reference< css::util::XSearchDescriptor > SAL_CALL SdUnoSearchReplaceShape::createSearchDescriptor(  )
{
    return new SdUnoSearchReplaceDescriptor;
}

uno::Reference< css::container::XIndexAccess > SAL_CALL SdUnoSearchReplaceShape::findAll( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
{
    SdUnoSearchReplaceDescriptor* pDescr = comphelper::getFromUnoTunnel<SdUnoSearchReplaceDescriptor>( xDesc );
    if( pDescr == nullptr )
        return uno::Reference< container::XIndexAccess > ();

    sal_Int32 nSequence = 32;
    sal_Int32 nFound    = 0;

    uno::Sequence < uno::Reference< uno::XInterface >  > aSeq( nSequence );

    uno::Reference< uno::XInterface > * pArray = aSeq.getArray();

    uno::Reference< drawing::XShapes >  xShapes;
    uno::Reference< drawing::XShape >  xShape;

    std::vector<SearchContext_impl> aContexts;
    if(mpPage)
    {
        xShapes = mpPage;

        if( xShapes->getCount() > 0 )
        {
            aContexts.push_back(SearchContext_impl(xShapes));
            xShape = aContexts.back().firstShape();
        }
        else
        {
            xShapes = nullptr;
        }
    }

    while( xShape.is() )
    {
        // find in xShape
        uno::Reference< text::XText >  xText(xShape, uno::UNO_QUERY);
        uno::Reference< text::XTextRange >  xRange = xText;
        uno::Reference< text::XTextRange >  xFound;

        while( xRange.is() )
        {
            xFound = Search( xRange, pDescr );
            if( !xFound.is() )
                break;

            if( nFound >= nSequence )
            {
                nSequence += 32;
                aSeq.realloc( nSequence );
                pArray = aSeq.getArray();
            }

            pArray[nFound++] = xFound;

            xRange = xFound->getEnd();
        }
        // done with shape -> get next shape

        // test if it's a group
        uno::Reference< drawing::XShapes >  xGroupShape;
        xGroupShape.set( xShape, uno::UNO_QUERY );

        if( xGroupShape.is() && xGroupShape->getCount() > 0 )
        {
            aContexts.push_back(SearchContext_impl(xGroupShape));
            xShape = aContexts.back().firstShape();
        }
        else
        {
            if (!aContexts.empty())
                xShape = aContexts.back().nextShape();
            else
                xShape = nullptr;
        }

        // test parent contexts for next shape if none
        // is found in the current context
        while (!aContexts.empty() && !xShape.is())
        {
            aContexts.pop_back();
            if (!aContexts.empty())
                xShape = aContexts.back().nextShape();
        }
    }

    if( nFound != nSequence )
        aSeq.realloc( nFound );

    uno::Reference<css::container::XIndexAccess> xRet(new SdUnoFindAllAccess(aSeq));
    return xRet;
}

uno::Reference< css::uno::XInterface > SAL_CALL SdUnoSearchReplaceShape::findFirst( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
{
    uno::Reference< text::XTextRange > xRange( GetCurrentShape(), uno::UNO_QUERY );
    if( xRange.is() )
        return findNext( xRange, xDesc );

    return uno::Reference< uno::XInterface > ();
}

uno::Reference< drawing::XShape >  SdUnoSearchReplaceShape::GetCurrentShape() const noexcept
{
    uno::Reference< drawing::XShape >  xShape;

    if( mpPage && mpPage->getCount() > 0)
        mpPage->getByIndex(0) >>= xShape;

    return xShape;

}

uno::Reference< css::uno::XInterface > SAL_CALL SdUnoSearchReplaceShape::findNext( const css::uno::Reference< css::uno::XInterface >& xStartAt, const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
{
    SdUnoSearchReplaceDescriptor* pDescr = comphelper::getFromUnoTunnel<SdUnoSearchReplaceDescriptor>( xDesc );

    uno::Reference< uno::XInterface > xFound;

    uno::Reference< text::XTextRange > xRange( xStartAt, uno::UNO_QUERY );
    if(pDescr && xRange.is() )
    {

        uno::Reference< text::XTextRange > xCurrentRange( xStartAt, uno::UNO_QUERY );

        uno::Reference< drawing::XShape > xCurrentShape( GetShape( xCurrentRange ) );

        while(!xFound.is() && xRange.is())
        {
            xFound = Search( xRange, pDescr );
            if(!xFound.is())
            {
                // we need a new starting range now
                xRange = nullptr;

                if(mpPage)
                {
                    // we do a page wide search, so skip to the next shape here
                    // get next shape on our page
                    uno::Reference< drawing::XShape > xFound2( GetNextShape( mpPage, xCurrentShape ) );
                    if( xFound2.is() && (xFound2.get() != xCurrentShape.get()) )
                        xCurrentShape = xFound2;
                    else
                        xCurrentShape = nullptr;

                    xRange.set( xCurrentShape, uno::UNO_QUERY );
                    if(!(xCurrentShape.is() && (xRange.is())))
                        xRange = nullptr;
                }
                else
                {
                    // we search only in this shape, so end search if we have
                    // not found anything
                }
            }
        }
    }
    return xFound;
}

/** this method returns the shape that follows xCurrentShape in the shape collection xShapes.
    It steps recursive into groupshapes and returns the xCurrentShape if it is the last
    shape in this collection */
uno::Reference< drawing::XShape >  SdUnoSearchReplaceShape::GetNextShape( const uno::Reference< container::XIndexAccess >&  xShapes, const uno::Reference< drawing::XShape >&  xCurrentShape ) noexcept
{
    uno::Reference< drawing::XShape >  xFound;

    if(xShapes.is() && xCurrentShape.is())
    {
        const sal_Int32 nCount = xShapes->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            uno::Reference< drawing::XShape > xSearchShape;
            xShapes->getByIndex(i) >>= xSearchShape;

            if( xSearchShape.is() )
            {
                uno::Reference< container::XIndexAccess > xGroup( xSearchShape, uno::UNO_QUERY );

                if( xCurrentShape.get() == xSearchShape.get() )
                {
                    if( xGroup.is() && xGroup->getCount() > 0 )
                    {
                        xGroup->getByIndex( 0 ) >>= xFound;
                    }
                    else
                    {
                        i++;
                        if( i < nCount )
                            xShapes->getByIndex( i ) >>= xFound;
                        else
                            xFound = xCurrentShape;
                    }

                    break;
                }
                else if( xGroup.is() )
                {
                    xFound = GetNextShape( xGroup, xCurrentShape );
                    if( xFound.is() )
                    {
                        if( xFound.get() == xCurrentShape.get() )
                        {
                            // the current shape was found at the end of the group
                            i++;
                            if( i < nCount )
                            {
                                xShapes->getByIndex(i) >>= xFound;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    return xFound;
}

uno::Reference< text::XTextRange >  SdUnoSearchReplaceShape::Search( const uno::Reference< text::XTextRange >&  xText, SdUnoSearchReplaceDescriptor* pDescr )
{
    if(!xText.is())
        return uno::Reference< text::XTextRange > ();

    uno::Reference< text::XText > xParent( xText->getText() );

    if( !xParent.is() )
    {
        xParent.set( xText, uno::UNO_QUERY );
    }

    const OUString aText( xParent->getString() );

    const sal_Int32 nTextLen = aText.getLength();

    std::unique_ptr<sal_Int32[]> pConvertPos( new sal_Int32[nTextLen+2] );
    std::unique_ptr<sal_Int32[]> pConvertPara( new sal_Int32[nTextLen+2] );

    const sal_Unicode* pText = aText.getStr();

    sal_Int32* pPos = pConvertPos.get();
    sal_Int32* pPara = pConvertPara.get();

    sal_Int32 nLastPos = 0, nLastPara = 0;

    uno::Reference< container::XEnumerationAccess > xEnumAccess( xParent, uno::UNO_QUERY );

    // first we fill the arrays with the position and paragraph for every character
    // inside the text
    if( xEnumAccess.is() )
    {
        uno::Reference< container::XEnumeration >  xParaEnum( xEnumAccess->createEnumeration() );

        while(xParaEnum->hasMoreElements())
        {
            int ndbg = 0;
            uno::Reference< text::XTextContent >  xParagraph( xParaEnum->nextElement(), uno::UNO_QUERY );
            if( xParagraph.is() )
                xEnumAccess.set(xParagraph, css::uno::UNO_QUERY);
            else
                xEnumAccess.clear();

            if( xEnumAccess.is() )
            {
                uno::Reference< container::XEnumeration >  xPortionEnum( xEnumAccess->createEnumeration() );
                if( xPortionEnum.is() )
                {
                    while(xPortionEnum->hasMoreElements())
                    {
                        uno::Reference< text::XTextRange >  xPortion( xPortionEnum->nextElement(), uno::UNO_QUERY );
                        if( xPortion.is() )
                        {
                            const OUString aPortion( xPortion->getString() );
                            const sal_Int32 nLen = aPortion.getLength();

                            ESelection aStartSel( GetSelection( xPortion->getStart() ) );
                            ESelection aEndSel( GetSelection( xPortion->getEnd() ) );

                            // special case for empty portions with content or length one portions with content (fields)
                            if( (aStartSel.nStartPos == aEndSel.nStartPos) || ( (aStartSel.nStartPos == (aEndSel.nStartPos - 1)) && (nLen > 1) ) )
                            {
                                for( sal_Int32 i = 0; i < nLen; i++ )
                                {
                                    if( ndbg < (nTextLen+2) )
                                    {
                                        *pPos++ = aStartSel.nStartPos;
                                        *pPara++ = aStartSel.nStartPara;

                                        ndbg += 1;
                                        pText++;
                                    }
                                    else
                                    {
                                        OSL_FAIL( "array overflow while searching" );
                                    }
                                }

                                nLastPos = aStartSel.nStartPos;
                            }
                            // normal case
                            else
                            {
                                for( sal_Int32 i = 0; i < nLen; i++ )
                                {
                                    if( ndbg < (nTextLen+2) )
                                    {
                                        *pPos++ = aStartSel.nStartPos++;
                                        *pPara++ = aStartSel.nStartPara;

                                        ndbg += 1;
                                        pText++;
                                    }
                                    else
                                    {
                                        OSL_FAIL( "array overflow while searching" );
                                    }
                                }

                                nLastPos = aStartSel.nStartPos - 1;
                                DBG_ASSERT( aEndSel.nStartPos == aStartSel.nStartPos, "Search is not working" );
                            }
                            nLastPara = aStartSel.nStartPara;
                        }
                    }
                }
            }

            if( ndbg < (nTextLen+2) )
            {
                *pPos++ = nLastPos + 1;
                *pPara++ = nLastPara;

                pText++;
            }
            else
            {
                OSL_FAIL( "array overflow while searching" );
            }
        }
    }

    uno::Reference< text::XTextRange >  xFound;
    ESelection aSel;

    if( xText.is() )
        aSel = GetSelection( xText );

    sal_Int32 nStartPos;
    sal_Int32 nEndPos   = 0;
    for( nStartPos = 0; nStartPos < nTextLen; nStartPos++ )
    {
        if( pConvertPara[nStartPos] == aSel.nStartPara && pConvertPos[nStartPos] == aSel.nStartPos )
            break;
    }

    if( Search( aText, nStartPos, nEndPos, pDescr ) )
    {
        if( nStartPos <= nTextLen && nEndPos <= nTextLen )
        {
            ESelection aSelection( pConvertPara[nStartPos], pConvertPos[nStartPos],
                             pConvertPara[nEndPos], pConvertPos[nEndPos] );

            SvxUnoTextBase* pParent = comphelper::getFromUnoTunnel<SvxUnoTextBase>( xParent );

            if(pParent)
            {
                rtl::Reference<SvxUnoTextRange> pRange = new SvxUnoTextRange( *pParent );
                xFound = pRange;
                pRange->SetSelection(aSelection);
            }
        }
        else
        {
            OSL_FAIL("Array overflow while searching!");
        }
    }

    return xFound;
}

bool SdUnoSearchReplaceShape::Search( const OUString& rText, sal_Int32& nStartPos, sal_Int32& nEndPos, SdUnoSearchReplaceDescriptor* pDescr ) noexcept
{
    OUString aSearchStr( pDescr->getSearchString() );
    OUString aText( rText );

    if( !pDescr->IsCaseSensitive() )
    {
        aText = aText.toAsciiLowerCase();
        aSearchStr = aSearchStr.toAsciiLowerCase();
    }

    sal_Int32 nFound = aText.indexOf( aSearchStr, nStartPos );
    if( nFound != -1 )
    {
        nStartPos = nFound;
        nEndPos   = nFound + aSearchStr.getLength();

        if(pDescr->IsWords())
        {
            if( (nStartPos > 0 && aText[nStartPos-1] > ' ') ||
                (nEndPos < aText.getLength() && aText[nEndPos] > ' ') )
            {
                nStartPos++;
                return Search( aText, nStartPos, nEndPos, pDescr );
            }
        }

        return true;
    }
    else
        return false;
}

ESelection SdUnoSearchReplaceShape::GetSelection( const uno::Reference< text::XTextRange >&  xTextRange ) noexcept
{
    ESelection aSel;
    SvxUnoTextRangeBase* pRange = comphelper::getFromUnoTunnel<SvxUnoTextRangeBase>( xTextRange );

    if(pRange)
        aSel = pRange->GetSelection();

    return aSel;
}

uno::Reference< drawing::XShape >  SdUnoSearchReplaceShape::GetShape( const uno::Reference< text::XTextRange >&  xTextRange ) noexcept
{
    uno::Reference< drawing::XShape >  xShape;

    if(xTextRange.is())
    {
        uno::Reference< text::XText >  xText( xTextRange->getText() );

        if(xText.is())
        {
            do
            {
                xShape.set( xText, uno::UNO_QUERY );
                if(!xShape.is())
                {
                    uno::Reference< text::XText > xParent( xText->getText() );
                    if(!xParent.is() || xText.get() == xParent.get())
                        return xShape;

                    xText = xParent;
                }
            } while( !xShape.is() );
        }
    }

    return xShape;
}

/* ================================================================= */
/** this class holds the parameters and status of a search or replace
    operation performed by class SdUnoSearchReplaceShape
  */

UNO3_GETIMPLEMENTATION_IMPL( SdUnoSearchReplaceDescriptor );

SdUnoSearchReplaceDescriptor::SdUnoSearchReplaceDescriptor()
{
    mpPropSet.reset( new SvxItemPropertySet(ImplGetSearchPropertyMap(), SdrObject::GetGlobalDrawObjectItemPool()) );

    mbBackwards = false;
    mbCaseSensitive = false;
    mbWords = false;
}

SdUnoSearchReplaceDescriptor::~SdUnoSearchReplaceDescriptor() noexcept
{
}

// XSearchDescriptor
OUString SAL_CALL SdUnoSearchReplaceDescriptor::getSearchString()
{
    return maSearchStr;
}

void SAL_CALL SdUnoSearchReplaceDescriptor::setSearchString( const OUString& aString )
{
    maSearchStr = aString;
}

// XReplaceDescriptor
OUString SAL_CALL SdUnoSearchReplaceDescriptor::getReplaceString()
{
    return maReplaceStr;
}

void SAL_CALL SdUnoSearchReplaceDescriptor::setReplaceString( const OUString& aReplaceString )
{
    maReplaceStr = aReplaceString;
}

// XPropertySet
uno::Reference< css::beans::XPropertySetInfo > SAL_CALL SdUnoSearchReplaceDescriptor::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SdUnoSearchReplaceDescriptor::setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMapEntry* pEntry = mpPropSet->getPropertyMapEntry(aPropertyName);

    bool bOk = false;

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case WID_SEARCH_BACKWARDS:
        bOk = (aValue >>= mbBackwards);
        break;
    case WID_SEARCH_CASE:
        bOk = (aValue >>= mbCaseSensitive);
        break;
    case WID_SEARCH_WORDS:
        bOk = (aValue >>= mbWords);
        break;
    default:
        throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    if( !bOk )
        throw lang::IllegalArgumentException();
}

uno::Any SAL_CALL SdUnoSearchReplaceDescriptor::getPropertyValue( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    uno::Any aAny;

    const SfxItemPropertyMapEntry* pEntry = mpPropSet->getPropertyMapEntry(PropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
    case WID_SEARCH_BACKWARDS:
        aAny <<= mbBackwards;
        break;
    case WID_SEARCH_CASE:
        aAny <<= mbCaseSensitive;
        break;
    case WID_SEARCH_WORDS:
        aAny <<= mbWords;
        break;
    default:
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    return aAny;
}

void SAL_CALL SdUnoSearchReplaceDescriptor::addPropertyChangeListener( const OUString& , const css::uno::Reference< css::beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdUnoSearchReplaceDescriptor::removePropertyChangeListener( const OUString& , const css::uno::Reference< css::beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdUnoSearchReplaceDescriptor::addVetoableChangeListener( const OUString& , const css::uno::Reference< css::beans::XVetoableChangeListener >&  ) {}
void SAL_CALL SdUnoSearchReplaceDescriptor::removeVetoableChangeListener( const OUString& , const css::uno::Reference< css::beans::XVetoableChangeListener >&  ) {}

/* ================================================================= */

SdUnoFindAllAccess::SdUnoFindAllAccess( uno::Sequence< uno::Reference< uno::XInterface >  > const & rSequence ) noexcept
:maSequence( rSequence )
{
}

SdUnoFindAllAccess::~SdUnoFindAllAccess() noexcept
{
}

// XElementAccess
uno::Type SAL_CALL SdUnoFindAllAccess::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SdUnoFindAllAccess::hasElements()
{
    return maSequence.hasElements();
}

// XIndexAccess
sal_Int32 SAL_CALL SdUnoFindAllAccess::getCount()
{
    return maSequence.getLength();
}

uno::Any SAL_CALL SdUnoFindAllAccess::getByIndex( sal_Int32 Index )
{
    if( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Any aAny;
    aAny <<= maSequence[Index];
    return aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <sal/config.h>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <svx/unoshape.hxx>
#include <svx/svdpool.hxx>
#include <svx/unoprov.hxx>
#include <editeng/unotext.hxx>

#include <comphelper/extract.hxx>

#include "unoprnms.hxx"
#include "unosrch.hxx"

using namespace ::com::sun::star;

#define WID_SEARCH_BACKWARDS    0
#define WID_SEARCH_CASE         1
#define WID_SEARCH_WORDS        2

const SfxItemPropertyMapEntry* ImplGetSearchPropertyMap()
{
    static const SfxItemPropertyMapEntry aSearchPropertyMap_Impl[] =
    {
        { OUString(UNO_NAME_SEARCH_BACKWARDS),  WID_SEARCH_BACKWARDS,   cppu::UnoType<bool>::get(),    0,  0 },
        { OUString(UNO_NAME_SEARCH_CASE),       WID_SEARCH_CASE,        cppu::UnoType<bool>::get(),    0,  0 },
        { OUString(UNO_NAME_SEARCH_WORDS),      WID_SEARCH_WORDS,       cppu::UnoType<bool>::get(),    0,  0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };

    return aSearchPropertyMap_Impl;
}

class SearchContext_impl
{
    uno::Reference< drawing::XShapes > mxShapes;
    sal_Int32 mnIndex;
    SearchContext_impl* mpParent;

public:
    SearchContext_impl( uno::Reference< drawing::XShapes > const & xShapes, SearchContext_impl* pParent = nullptr )
        : mxShapes( xShapes ), mnIndex( -1 ), mpParent( pParent ) {}

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

    SearchContext_impl* getParent() const { return mpParent; }
};

/* ================================================================= */
/** this class implements a search or replace operation on a given
    page or a given sdrobj
  */

SdUnoSearchReplaceShape::SdUnoSearchReplaceShape( drawing::XDrawPage* pPage ) throw()
    : mpShape(nullptr)
    , mpPage(pPage)
{
}

SdUnoSearchReplaceShape::~SdUnoSearchReplaceShape() throw()
{
}

// util::XReplaceable
uno::Reference< util::XReplaceDescriptor > SAL_CALL SdUnoSearchReplaceShape::createReplaceDescriptor()
{
    return new SdUnoSearchReplaceDescriptor(true);
}

sal_Int32 SAL_CALL SdUnoSearchReplaceShape::replaceAll( const uno::Reference< util::XSearchDescriptor >& xDesc )
{
    SdUnoSearchReplaceDescriptor* pDescr = SdUnoSearchReplaceDescriptor::getImplementation( xDesc );
    if( pDescr == nullptr )
        return 0;

    sal_Int32 nFound    = 0;

    uno::Reference< drawing::XShapes >  xShapes;
    uno::Reference< drawing::XShape >  xShape;

    SearchContext_impl* pContext = nullptr;
    if(mpPage)
    {
        uno::Reference< drawing::XDrawPage > xPage( mpPage );

        xShapes.set( xPage, uno::UNO_QUERY );

        if( xShapes.is() && (xShapes->getCount() > 0) )
        {
            pContext = new SearchContext_impl( xShapes );
            xShape = pContext->firstShape();
        }
        else
        {
            xShapes = nullptr;
        }
    }
    else
    {
        xShape = mpShape;
    }

    while( xShape.is() )
    {
        // replace in xShape
        uno::Reference< text::XText >  xText(xShape, uno::UNO_QUERY);
        uno::Reference< text::XTextRange >  xRange(xText, uno::UNO_QUERY);
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
            pContext = new SearchContext_impl( xGroupShape, pContext );
            xShape = pContext->firstShape();
        }
        else
        {
            if( pContext )
                xShape = pContext->nextShape();
            else
                xShape = nullptr;
        }

        // test parent contexts for next shape if none
        // is found in the current context
        while( pContext && !xShape.is() )
        {
            if( pContext->getParent() )
            {
                SearchContext_impl* pOldContext = pContext;
                pContext = pContext->getParent();
                delete pOldContext;
                xShape = pContext->nextShape();
            }
            else
            {
                delete pContext;
                pContext = nullptr;
                xShape = nullptr;
            }
        }
    }

    return nFound;
}

// XSearchable
uno::Reference< css::util::XSearchDescriptor > SAL_CALL SdUnoSearchReplaceShape::createSearchDescriptor(  )
{
    return new SdUnoSearchReplaceDescriptor(false);
}

uno::Reference< css::container::XIndexAccess > SAL_CALL SdUnoSearchReplaceShape::findAll( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
{
    SdUnoSearchReplaceDescriptor* pDescr = SdUnoSearchReplaceDescriptor::getImplementation( xDesc );
    if( pDescr == nullptr )
        return uno::Reference< container::XIndexAccess > ();

    sal_Int32 nSequence = 32;
    sal_Int32 nFound    = 0;

    uno::Sequence < uno::Reference< uno::XInterface >  > aSeq( nSequence );

    uno::Reference< uno::XInterface > * pArray = aSeq.getArray();

    uno::Reference< drawing::XShapes >  xShapes;
    uno::Reference< drawing::XShape >  xShape;

    SearchContext_impl* pContext = nullptr;
    if(mpPage)
    {
        uno::Reference< drawing::XDrawPage >  xPage( mpPage );
        xShapes.set( xPage, uno::UNO_QUERY );

        if( xShapes.is() && xShapes->getCount() > 0 )
        {
            pContext = new SearchContext_impl( xShapes );
            xShape = pContext->firstShape();
        }
        else
        {
            xShapes = nullptr;
        }
    }
    else
    {
        xShape = mpShape;
    }
    while( xShape.is() )
    {
        // find in xShape
        uno::Reference< text::XText >  xText(xShape, uno::UNO_QUERY);
        uno::Reference< text::XTextRange >  xRange(xText, uno::UNO_QUERY);
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
            pContext = new SearchContext_impl( xGroupShape, pContext );
            xShape = pContext->firstShape();
        }
        else
        {
            if( pContext )
                xShape = pContext->nextShape();
            else
                xShape = nullptr;
        }

        // test parent contexts for next shape if none
        // is found in the current context
        while( pContext && !xShape.is() )
        {
            if( pContext->getParent() )
            {
                SearchContext_impl* pOldContext = pContext;
                pContext = pContext->getParent();
                delete pOldContext;
                xShape = pContext->nextShape();
            }
            else
            {
                delete pContext;
                pContext = nullptr;
                xShape = nullptr;
            }
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

uno::Reference< drawing::XShape >  SdUnoSearchReplaceShape::GetCurrentShape() const throw()
{
    uno::Reference< drawing::XShape >  xShape;

    if( mpPage )
    {
        uno::Reference< drawing::XDrawPage >  xPage( mpPage );
        uno::Reference< container::XIndexAccess >  xShapes( xPage, uno::UNO_QUERY );
        if( xShapes.is() )
        {
            if(xShapes->getCount() > 0)
            {
                xShapes->getByIndex(0) >>= xShape;
            }
        }
    }
    else if( mpShape )
    {
        xShape = mpShape;
    }

    return xShape;

}

uno::Reference< css::uno::XInterface > SAL_CALL SdUnoSearchReplaceShape::findNext( const css::uno::Reference< css::uno::XInterface >& xStartAt, const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
{
    SdUnoSearchReplaceDescriptor* pDescr = SdUnoSearchReplaceDescriptor::getImplementation( xDesc );

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
                    uno::Reference< drawing::XDrawPage >  xPage( mpPage );

                    // we do a page wide search, so skip to the next shape here
                    uno::Reference< container::XIndexAccess > xShapes( xPage, uno::UNO_QUERY );

                    // get next shape on our page
                    if( xShapes.is() )
                    {
                        uno::Reference< drawing::XShape > xFound2( GetNextShape( xShapes, xCurrentShape ) );
                        if( xFound2.is() && (xFound2.get() != xCurrentShape.get()) )
                            xCurrentShape = xFound2;
                        else
                            xCurrentShape = nullptr;

                        xRange.set( xCurrentShape, uno::UNO_QUERY );
                        if(!(xCurrentShape.is() && (xRange.is())))
                            xRange = nullptr;
                    }
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
uno::Reference< drawing::XShape >  SdUnoSearchReplaceShape::GetNextShape( const uno::Reference< container::XIndexAccess >&  xShapes, const uno::Reference< drawing::XShape >&  xCurrentShape ) throw()
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

uno::Reference< text::XTextRange >  SdUnoSearchReplaceShape::Search( const uno::Reference< text::XTextRange >&  xText, SdUnoSearchReplaceDescriptor* pDescr ) throw()
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

    sal_Int32* pConvertPos = new sal_Int32[nTextLen+2];
    sal_Int32* pConvertPara = new sal_Int32[nTextLen+2];

    const sal_Unicode* pText = aText.getStr();

    sal_Int32* pPos = pConvertPos;
    sal_Int32* pPara = pConvertPara;

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

    uno::Reference< text::XTextRange > xRangeRef( xText, uno::UNO_QUERY );
    if( xRangeRef.is() )
        aSel = GetSelection( xRangeRef );

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
            SvxUnoTextRange *pRange;

            SvxUnoTextBase* pParent = SvxUnoTextBase::getImplementation( xParent );

            if(pParent)
            {
                pRange = new SvxUnoTextRange( *pParent );
                xFound = pRange;
                pRange->SetSelection(aSelection);

            }
        }
        else
        {
            OSL_FAIL("Array overflow while searching!");
        }
    }

    delete[] pConvertPos;
    delete[] pConvertPara;

    return xFound;
}

bool SdUnoSearchReplaceShape::Search( const OUString& rText, sal_Int32& nStartPos, sal_Int32& nEndPos, SdUnoSearchReplaceDescriptor* pDescr ) throw()
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

ESelection SdUnoSearchReplaceShape::GetSelection( const uno::Reference< text::XTextRange >&  xTextRange ) throw()
{
    ESelection aSel;
    SvxUnoTextRangeBase* pRange = SvxUnoTextRangeBase::getImplementation( xTextRange );

    if(pRange)
        aSel = pRange->GetSelection();

    return aSel;
}

uno::Reference< drawing::XShape >  SdUnoSearchReplaceShape::GetShape( const uno::Reference< text::XTextRange >&  xTextRange ) throw()
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

SdUnoSearchReplaceDescriptor::SdUnoSearchReplaceDescriptor( bool bReplace )
{
    mpPropSet.reset( new SvxItemPropertySet(ImplGetSearchPropertyMap(), SdrObject::GetGlobalDrawObjectItemPool()) );

    mbBackwards = false;
    mbCaseSensitive = false;
    mbWords = false;

    mbReplace = bReplace;
}

SdUnoSearchReplaceDescriptor::~SdUnoSearchReplaceDescriptor() throw()
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

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(aPropertyName);

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

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(PropertyName);

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

SdUnoFindAllAccess::SdUnoFindAllAccess( uno::Sequence< uno::Reference< uno::XInterface >  >& rSequence ) throw()
:maSequence( rSequence )
{
}

SdUnoFindAllAccess::~SdUnoFindAllAccess() throw()
{
}

// XElementAccess
uno::Type SAL_CALL SdUnoFindAllAccess::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SAL_CALL SdUnoFindAllAccess::hasElements()
{
    return maSequence.getLength() > 0;
}

// XIndexAccess
sal_Int32 SAL_CALL SdUnoFindAllAccess::getCount()
{
    return maSequence.getLength();
}

uno::Any SAL_CALL SdUnoFindAllAccess::getByIndex( sal_Int32 Index )
{
    uno::Any aAny;

    if( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();

    const uno::Reference< uno::XInterface >  *pRefs = maSequence.getConstArray();
    if(pRefs)
        aAny <<= pRefs[ Index ];
    return aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

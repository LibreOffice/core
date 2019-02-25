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


// Global header


#include <limits.h>
#include <utility>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <tools/debug.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>


// Project-local header


#include <editeng/editdata.hxx>
#include <editeng/unopracc.hxx>
#include <editeng/unoedprx.hxx>
#include <editeng/AccessibleStaticTextBase.hxx>
#include <editeng/AccessibleEditableTextPara.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

/* TODO:
   =====

   - separate adapter functionality from AccessibleStaticText class

   - refactor common loops into templates, using mem_fun

 */

namespace accessibility
{
    typedef std::vector< beans::PropertyValue > PropertyValueVector;

    class PropertyValueEqualFunctor
    {
        const beans::PropertyValue& m_rPValue;

    public:
        explicit PropertyValueEqualFunctor(const beans::PropertyValue& rPValue)
            : m_rPValue(rPValue)
        {}
        bool operator() ( const beans::PropertyValue& rhs ) const
        {
            return ( m_rPValue.Name == rhs.Name && m_rPValue.Value == rhs.Value );
        }
    };
    sal_Unicode const cNewLine(0x0a);


    // Static Helper


    static ESelection MakeSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                              sal_Int32 nEndPara, sal_Int32 nEndIndex )
    {
        DBG_ASSERT(nStartPara >= 0 &&
                   nStartIndex >= 0 &&
                   nEndPara >= 0 &&
                   nEndIndex >= 0,
                   "AccessibleStaticTextBase_Impl::MakeSelection: index value overflow");

        return ESelection(nStartPara, nStartIndex, nEndPara, nEndIndex);
    }


    // AccessibleStaticTextBase_Impl declaration


    /** AccessibleStaticTextBase_Impl

        This class implements the AccessibleStaticTextBase
        functionality, mainly by forwarding the calls to an aggregated
        AccessibleEditableTextPara. As this is a therefore non-trivial
        adapter, factoring out the common functionality from
        AccessibleEditableTextPara might be a profitable future task.
     */
    class AccessibleStaticTextBase_Impl
    {
        friend class AccessibleStaticTextBase;
    public:

        // receive pointer to our frontend class and view window
        AccessibleStaticTextBase_Impl();

        void SetEditSource( std::unique_ptr< SvxEditSource > && pEditSource );

        void SetEventSource( const uno::Reference< XAccessible >& rInterface )
        {

            mxThis = rInterface;
        }

        void SetOffset( const Point& );

        void Dispose();

        AccessibleEditableTextPara& GetParagraph( sal_Int32 nPara ) const;
        sal_Int32                   GetParagraphCount() const;

        EPosition                   Index2Internal( sal_Int32 nFlatIndex ) const
        {

            return ImpCalcInternal( nFlatIndex, false );
        }

        EPosition                   Range2Internal( sal_Int32 nFlatIndex ) const
        {

            return ImpCalcInternal( nFlatIndex, true );
        }

        sal_Int32                   Internal2Index( EPosition nEEIndex ) const;

        void                        CorrectTextSegment( TextSegment&    aTextSegment,
                                                        int             nPara   ) const;

        bool                    SetSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                                  sal_Int32 nEndPara, sal_Int32 nEndIndex );
        bool                    CopyText( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                              sal_Int32 nEndPara, sal_Int32 nEndIndex );

        tools::Rectangle                   GetParagraphBoundingBox() const;
        bool                    RemoveLineBreakCount( sal_Int32& rIndex );

    private:

        EPosition                   ImpCalcInternal( sal_Int32 nFlatIndex, bool bExclusive ) const;

        // our frontend class (the one implementing the actual
        // interface). That's not necessarily the one containing the impl
        // pointer
        uno::Reference< XAccessible > mxThis;

        // implements our functionality, we're just an adapter (guarded by solar mutex)
        mutable rtl::Reference<AccessibleEditableTextPara> mxTextParagraph;

        // a wrapper for the text forwarders (guarded by solar mutex)
        mutable SvxEditSourceAdapter maEditSource;
    };


    // AccessibleStaticTextBase_Impl implementation


    AccessibleStaticTextBase_Impl::AccessibleStaticTextBase_Impl() :
        mxTextParagraph( new AccessibleEditableTextPara(nullptr) ),
        maEditSource()
    {

        // TODO: this is still somewhat of a hack, all the more since
        // now the maTextParagraph has an empty parent reference set
    }

    void AccessibleStaticTextBase_Impl::SetEditSource( std::unique_ptr< SvxEditSource > && pEditSource )
    {

        maEditSource.SetEditSource( std::move(pEditSource) );
        if( mxTextParagraph.is() )
            mxTextParagraph->SetEditSource( &maEditSource );
    }

    void AccessibleStaticTextBase_Impl::SetOffset( const Point& rPoint )
    {
        if( mxTextParagraph.is() )
            mxTextParagraph->SetEEOffset( rPoint );
    }

    void AccessibleStaticTextBase_Impl::Dispose()
    {

        // we're the owner of the paragraph, so destroy it, too
        if( mxTextParagraph.is() )
            mxTextParagraph->Dispose();

        // drop references
        mxThis = nullptr;
        mxTextParagraph.clear();
    }

    AccessibleEditableTextPara& AccessibleStaticTextBase_Impl::GetParagraph( sal_Int32 nPara ) const
    {

        if( !mxTextParagraph.is() )
            throw lang::DisposedException ("object has been already disposed", mxThis );

        // TODO: Have a different method on AccessibleEditableTextPara
        // that does not care about state changes
        mxTextParagraph->SetParagraphIndex( nPara );

        return *mxTextParagraph;
    }

    sal_Int32 AccessibleStaticTextBase_Impl::GetParagraphCount() const
    {

        if( !mxTextParagraph.is() )
            return 0;
        else
            return mxTextParagraph->GetTextForwarder().GetParagraphCount();
    }

    sal_Int32 AccessibleStaticTextBase_Impl::Internal2Index( EPosition nEEIndex ) const
    {
        // XXX checks for overflow and returns maximum if so
        sal_Int32 aRes(0);
        for(sal_Int32 i=0; i<nEEIndex.nPara; ++i)
        {
            sal_Int32 nCount = GetParagraph(i).getCharacterCount();
            if (SAL_MAX_INT32 - aRes > nCount)
                return SAL_MAX_INT32;
            aRes += nCount;
        }

        if (SAL_MAX_INT32 - aRes > nEEIndex.nIndex)
            return SAL_MAX_INT32;
        return aRes + nEEIndex.nIndex;
    }

    void AccessibleStaticTextBase_Impl::CorrectTextSegment( TextSegment&    aTextSegment,
                                                            int             nPara   ) const
    {
        // Keep 'invalid' values at the TextSegment
        if( aTextSegment.SegmentStart != -1 &&
            aTextSegment.SegmentEnd != -1 )
        {
            // #112814# Correct TextSegment by paragraph offset
            sal_Int32 nOffset(0);
            int i;
            for(i=0; i<nPara; ++i)
                nOffset += GetParagraph(i).getCharacterCount();

            aTextSegment.SegmentStart += nOffset;
            aTextSegment.SegmentEnd += nOffset;
        }
    }

    EPosition AccessibleStaticTextBase_Impl::ImpCalcInternal( sal_Int32 nFlatIndex, bool bExclusive ) const
    {

        if( nFlatIndex < 0 )
            throw lang::IndexOutOfBoundsException("AccessibleStaticTextBase_Impl::Index2Internal: character index out of bounds",
                                                  mxThis);
        // gratuitously accepting larger indices here, AccessibleEditableTextPara will throw eventually

        sal_Int32 nCurrPara, nCurrIndex, nParas, nCurrCount;
        for( nCurrPara=0, nParas=GetParagraphCount(), nCurrCount=0, nCurrIndex=0; nCurrPara<nParas; ++nCurrPara )
        {
            nCurrCount = GetParagraph( nCurrPara ).getCharacterCount();
            nCurrIndex += nCurrCount;
            if( nCurrIndex >= nFlatIndex )
            {
                // check overflow
                DBG_ASSERT(nCurrPara >= 0 &&
                           nFlatIndex - nCurrIndex + nCurrCount >= 0,
                           "AccessibleStaticTextBase_Impl::Index2Internal: index value overflow");

                return EPosition(nCurrPara, nFlatIndex - nCurrIndex + nCurrCount);
            }
        }

        // #102170# Allow one-past the end for ranges
        if( bExclusive && nCurrIndex == nFlatIndex )
        {
            // check overflow
            DBG_ASSERT(nCurrPara > 0 &&
                       nFlatIndex - nCurrIndex + nCurrCount >= 0,
                       "AccessibleStaticTextBase_Impl::Index2Internal: index value overflow");

            return EPosition(nCurrPara-1, nFlatIndex - nCurrIndex + nCurrCount);
        }

        // not found? Out of bounds
        throw lang::IndexOutOfBoundsException("AccessibleStaticTextBase_Impl::Index2Internal: character index out of bounds",
                                              mxThis);
    }

    bool AccessibleStaticTextBase_Impl::SetSelection( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                                          sal_Int32 nEndPara, sal_Int32 nEndIndex )
    {

        if( !mxTextParagraph.is() )
            return false;

        try
        {
            SvxEditViewForwarder& rCacheVF = mxTextParagraph->GetEditViewForwarder( true );
            return rCacheVF.SetSelection( MakeSelection(nStartPara, nStartIndex, nEndPara, nEndIndex) );
        }
        catch( const uno::RuntimeException& )
        {
            return false;
        }
    }

    bool AccessibleStaticTextBase_Impl::CopyText( sal_Int32 nStartPara, sal_Int32 nStartIndex,
                                                      sal_Int32 nEndPara, sal_Int32 nEndIndex )
    {

        if( !mxTextParagraph.is() )
            return false;

        try
        {
            SvxEditViewForwarder& rCacheVF = mxTextParagraph->GetEditViewForwarder( true );
            mxTextParagraph->GetTextForwarder();    // MUST be after GetEditViewForwarder(), see method docs
            bool aRetVal;

            // save current selection
            ESelection aOldSelection;

            rCacheVF.GetSelection( aOldSelection );
            rCacheVF.SetSelection( MakeSelection(nStartPara, nStartIndex, nEndPara, nEndIndex) );
            aRetVal = rCacheVF.Copy();
            rCacheVF.SetSelection( aOldSelection ); // restore

            return aRetVal;
        }
        catch( const uno::RuntimeException& )
        {
            return false;
        }
    }

    tools::Rectangle AccessibleStaticTextBase_Impl::GetParagraphBoundingBox() const
    {
        tools::Rectangle aRect;
        if( mxTextParagraph.is() )
        {
            awt::Rectangle aAwtRect = mxTextParagraph->getBounds();
            aRect = tools::Rectangle( Point( aAwtRect.X, aAwtRect.Y ), Size( aAwtRect.Width, aAwtRect.Height ) );
        }
        else
        {
            aRect.SetEmpty();
        }
        return aRect;
    }
    //the input argument is the index(including "\n" ) in the string.
    //the function will calculate the actual index(not including "\n") in the string.
    //and return true if the index is just at a "\n"
    bool AccessibleStaticTextBase_Impl::RemoveLineBreakCount( sal_Int32& rIndex )
    {
        // get the total char number inside the cell.
        sal_Int32 i, nCount, nParas;
        for( i=0, nCount=0, nParas=GetParagraphCount(); i<nParas; ++i )
            nCount += GetParagraph(i).getCharacterCount();
        nCount = nCount + (nParas-1);
        if( nCount == 0 &&  rIndex == 0) return false;


        sal_Int32 nCurrPara, nCurrCount;
        sal_Int32 nLineBreakPos = 0, nLineBreakCount = 0;
        sal_Int32 nParaCount = GetParagraphCount();
        for ( nCurrCount = 0, nCurrPara = 0; nCurrPara < nParaCount; nCurrPara++ )
        {
            nCurrCount += GetParagraph( nCurrPara ).getCharacterCount();
            nLineBreakPos = nCurrCount++;
            if ( rIndex == nLineBreakPos )
            {
                rIndex -= (++nLineBreakCount);//(++nLineBreakCount);
                if ( rIndex < 0)
                {
                    rIndex = 0;
                }
                //if the index is at the last position of the last paragraph
                //there is no "\n" , so we should increase rIndex by 1 and return false.
                if ( (nCurrPara+1) == nParaCount )
                {
                    rIndex++;
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else if ( rIndex < nLineBreakPos )
            {
                rIndex -= nLineBreakCount;
                return false;
            }
            else
            {
                nLineBreakCount++;
            }
        }
        return false;
    }


    // AccessibleStaticTextBase implementation

    AccessibleStaticTextBase::AccessibleStaticTextBase( std::unique_ptr< SvxEditSource > && pEditSource ) :
        mpImpl( new AccessibleStaticTextBase_Impl() )
    {
        SolarMutexGuard aGuard;

        SetEditSource( std::move(pEditSource) );
    }

    AccessibleStaticTextBase::~AccessibleStaticTextBase()
    {
    }

    void AccessibleStaticTextBase::SetEditSource( std::unique_ptr< SvxEditSource > && pEditSource )
    {
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->SetEditSource( std::move(pEditSource) );
    }

    void AccessibleStaticTextBase::SetEventSource( const uno::Reference< XAccessible >& rInterface )
    {
        mpImpl->SetEventSource( rInterface );

    }

    void AccessibleStaticTextBase::SetOffset( const Point& rPoint )
    {
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->SetOffset( rPoint );
    }

    void AccessibleStaticTextBase::Dispose()
    {
        mpImpl->Dispose();

    }

    // XAccessibleContext
    sal_Int32 AccessibleStaticTextBase::getAccessibleChildCount()
    {
        // no children at all
        return 0;
    }

    uno::Reference< XAccessible > AccessibleStaticTextBase::getAccessibleChild( sal_Int32 /*i*/ )
    {
        // no children at all
        return uno::Reference< XAccessible >();
    }

    uno::Reference< XAccessible > AccessibleStaticTextBase::getAccessibleAtPoint( const awt::Point& /*_aPoint*/ )
    {
        // no children at all
        return uno::Reference< XAccessible >();
    }

    // XAccessibleText
    sal_Int32 SAL_CALL AccessibleStaticTextBase::getCaretPosition()
    {
        SolarMutexGuard aGuard;

        sal_Int32 i, nPos, nParas;
        for( i=0, nPos=-1, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
        {
            if( (nPos=mpImpl->GetParagraph(i).getCaretPosition()) != -1 )
                return nPos;
        }

        return nPos;
    }

    sal_Bool SAL_CALL AccessibleStaticTextBase::setCaretPosition( sal_Int32 nIndex )
    {
        return setSelection(nIndex, nIndex);
    }

    sal_Unicode SAL_CALL AccessibleStaticTextBase::getCharacter( sal_Int32 nIndex )
    {
        SolarMutexGuard aGuard;

        EPosition aPos( mpImpl->Index2Internal(nIndex) );

        return mpImpl->GetParagraph( aPos.nPara ).getCharacter( aPos.nIndex );
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleStaticTextBase::getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes )
    {
        SolarMutexGuard aGuard;

        //get the actual index without "\n"
        mpImpl->RemoveLineBreakCount( nIndex );

        EPosition aPos( mpImpl->Index2Internal(nIndex) );

        return mpImpl->GetParagraph( aPos.nPara ).getCharacterAttributes( aPos.nIndex, aRequestedAttributes );
    }

    awt::Rectangle SAL_CALL AccessibleStaticTextBase::getCharacterBounds( sal_Int32 nIndex )
    {
        SolarMutexGuard aGuard;

        // #108900# Allow ranges for nIndex, as one-past-the-end
        // values are now legal, too.
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        // #i70916# Text in spread sheet cells return the wrong extents
        AccessibleEditableTextPara& rPara = mpImpl->GetParagraph( aPos.nPara );
        awt::Rectangle aParaBounds( rPara.getBounds() );
        awt::Rectangle aBounds( rPara.getCharacterBounds( aPos.nIndex ) );
        aBounds.X += aParaBounds.X;
        aBounds.Y += aParaBounds.Y;

        return aBounds;
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getCharacterCount()
    {
        SolarMutexGuard aGuard;

        sal_Int32 i, nCount, nParas;
        for( i=0, nCount=0, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
            nCount += mpImpl->GetParagraph(i).getCharacterCount();
        //count on the number of "\n" which equals number of paragraphs decrease 1.
        nCount = nCount + (nParas-1);
        return nCount;
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getIndexAtPoint( const awt::Point& rPoint )
    {
        SolarMutexGuard aGuard;

        const sal_Int32 nParas( mpImpl->GetParagraphCount() );
        sal_Int32 nIndex;
        int i;
        for( i=0; i<nParas; ++i )
        {
            // TODO: maybe exploit the fact that paragraphs are
            // ordered vertically for early exit

            // #i70916# Text in spread sheet cells return the wrong extents
            AccessibleEditableTextPara& rPara = mpImpl->GetParagraph( i );
            awt::Rectangle aParaBounds( rPara.getBounds() );
            awt::Point aPoint( rPoint );
            aPoint.X -= aParaBounds.X;
            aPoint.Y -= aParaBounds.Y;

            // #112814# Use correct index offset
            if ( ( nIndex = rPara.getIndexAtPoint( aPoint ) ) != -1 )
                return mpImpl->Internal2Index(EPosition(i, nIndex));
        }

        return -1;
    }

    OUString SAL_CALL AccessibleStaticTextBase::getSelectedText()
    {
        SolarMutexGuard aGuard;

        sal_Int32 nStart( getSelectionStart() );
        sal_Int32 nEnd( getSelectionEnd() );

        // #104481# Return the empty string for 'no selection'
        if( nStart < 0 || nEnd < 0 )
            return OUString();

        return getTextRange( nStart, nEnd );
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getSelectionStart()
    {
        SolarMutexGuard aGuard;

        sal_Int32 i, nPos, nParas;
        for( i=0, nPos=-1, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
        {
            if( (nPos=mpImpl->GetParagraph(i).getSelectionStart()) != -1 )
                return nPos;
        }

        return nPos;
    }

    sal_Int32 SAL_CALL AccessibleStaticTextBase::getSelectionEnd()
    {
        SolarMutexGuard aGuard;

        sal_Int32 i, nPos, nParas;
        for( i=0, nPos=-1, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
        {
            if( (nPos=mpImpl->GetParagraph(i).getSelectionEnd()) != -1 )
                return nPos;
        }

        return nPos;
    }

    sal_Bool SAL_CALL AccessibleStaticTextBase::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aGuard;

        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        return mpImpl->SetSelection( aStartIndex.nPara, aStartIndex.nIndex,
                                     aEndIndex.nPara, aEndIndex.nIndex );
    }

    OUString SAL_CALL AccessibleStaticTextBase::getText()
    {
        SolarMutexGuard aGuard;

        sal_Int32 i, nParas;
        OUStringBuffer aRes;
        for( i=0, nParas=mpImpl->GetParagraphCount(); i<nParas; ++i )
            aRes.append(mpImpl->GetParagraph(i).getText());

        return aRes.makeStringAndClear();
    }

    OUString SAL_CALL AccessibleStaticTextBase::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aGuard;

        if( nStartIndex > nEndIndex )
            std::swap(nStartIndex, nEndIndex);
        //if startindex equals endindex we will get nothing. So return an empty string directly.
        if ( nStartIndex == nEndIndex )
        {
            return OUString();
        }
        bool bStart = mpImpl->RemoveLineBreakCount( nStartIndex );
        //if the start index is just at a "\n", we need to begin from the next char
        if ( bStart )
        {
            nStartIndex++;
        }
        //we need to find out whether the previous position of the current endindex is at "\n" or not
        //if yes we need to mark it and add "\n" at the end of the result
        sal_Int32 nTemp = nEndIndex - 1;
        bool bEnd = mpImpl->RemoveLineBreakCount( nTemp );
        bool bTemp = mpImpl->RemoveLineBreakCount( nEndIndex );
        //if the below condition is true it indicates an empty paragraph with just a "\n"
        //so we need to set one "\n" flag to avoid duplication.
        if ( bStart && bEnd && ( nStartIndex == nEndIndex) )
        {
            bEnd = false;
        }
        //if the current endindex is at a "\n", we need to increase endindex by 1 to make sure
        //the char before "\n" is included. Because string returned by this function will not include
        //the char at the endindex.
        if ( bTemp )
        {
            nEndIndex++;
        }
        OUStringBuffer aRes;
        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        // #102170# Special case: start and end paragraph are identical
        if( aStartIndex.nPara == aEndIndex.nPara )
        {
            //we don't return the string directly now for that we have to do some further process for "\n"
            aRes = mpImpl->GetParagraph( aStartIndex.nPara ).getTextRange( aStartIndex.nIndex, aEndIndex.nIndex );
        }
        else
        {
            sal_Int32 i( aStartIndex.nPara );
            aRes = mpImpl->GetParagraph(i).getTextRange( aStartIndex.nIndex,
                                                         mpImpl->GetParagraph(i).getCharacterCount()/*-1*/);
            ++i;

            // paragraphs inbetween are fully included
            for( ; i<aEndIndex.nPara; ++i )
            {
                aRes.append(cNewLine);
                aRes.append(mpImpl->GetParagraph(i).getText());
            }

            if( i<=aEndIndex.nPara )
            {
                //if the below condition is matched it means that endindex is at mid of the last paragraph
                //we need to add a "\n" before we add the last part of the string.
                if ( !bEnd && aEndIndex.nIndex )
                {
                    aRes.append(cNewLine);
                }
                aRes.append(mpImpl->GetParagraph(i).getTextRange( 0, aEndIndex.nIndex ));
            }
        }
        //According to the flag we marked before, we have to add "\n" at the beginning
        //or at the end of the result string.
        if ( bStart )
        {
            aRes.insert(0, OUStringLiteral1(cNewLine));
        }
        if ( bEnd )
        {
            aRes.append(OUStringLiteral1(cNewLine));
        }
        return aRes.makeStringAndClear();
    }

    css::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aGuard;

        bool bLineBreak = mpImpl->RemoveLineBreakCount( nIndex );
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        css::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            // #106393# Special casing one behind last paragraph is
            // not necessary, since then, we return the content and
            // boundary of that last paragraph. Range2Internal is
            // tolerant against that, and returns the last paragraph
            // in aPos.nPara.

            // retrieve full text of the paragraph
            aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara ).getText();

            // #112814# Adapt the start index with the paragraph offset
            aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara, 0 ) );
            aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
        }
        else if ( AccessibleTextType::ATTRIBUTE_RUN == aTextType )
        {
              SvxAccessibleTextAdapter& rTextForwarder = mpImpl->GetParagraph( aPos.nIndex ).GetTextForwarder();
              sal_Int32 nStartIndex, nEndIndex;
              if ( rTextForwarder.GetAttributeRun( nStartIndex, nEndIndex, aPos.nPara, aPos.nIndex, true ) )
              {
                     aResult.SegmentText = getTextRange( nStartIndex, nEndIndex );
                     aResult.SegmentStart = nStartIndex;
                     aResult.SegmentEnd = nEndIndex;
              }
        }
        else
        {
            // No special handling required, forward to wrapped class
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextAtIndex( aPos.nIndex, aTextType );

            // #112814# Adapt the start index with the paragraph offset
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
            if ( bLineBreak )
            {
                aResult.SegmentText = OUString(cNewLine);
            }
        }

        return aResult;
    }

    css::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aGuard;

        sal_Int32 nOldIdx = nIndex;
        bool bLineBreak =  mpImpl->RemoveLineBreakCount( nIndex );
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        css::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            if( aPos.nIndex == mpImpl->GetParagraph( aPos.nPara ).getCharacterCount() )
            {
                // #103589# Special casing one behind the last paragraph
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara ).getText();

                // #112814# Adapt the start index with the paragraph offset
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara, 0 ) );
            }
            else if( aPos.nPara > 0 )
            {
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara - 1 ).getText();

                // #112814# Adapt the start index with the paragraph offset
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara - 1, 0 ) );
            }

            aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
        }
        else
        {
            // No special handling required, forward to wrapped class
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextBeforeIndex( aPos.nIndex, aTextType );

            // #112814# Adapt the start index with the paragraph offset
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
            if ( bLineBreak && (nOldIdx-1) >= 0)
            {
                aResult = getTextAtIndex( nOldIdx-1, aTextType );
            }
        }

        return aResult;
    }

    css::accessibility::TextSegment SAL_CALL AccessibleStaticTextBase::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aGuard;

        sal_Int32 nTemp = nIndex+1;
        bool bLineBreak = mpImpl->RemoveLineBreakCount( nTemp );
        mpImpl->RemoveLineBreakCount( nIndex );
        EPosition aPos( mpImpl->Range2Internal(nIndex) );

        css::accessibility::TextSegment aResult;

        if( AccessibleTextType::PARAGRAPH == aTextType )
        {
            // Special casing one behind the last paragraph is not
            // necessary, this case is invalid here for
            // getTextBehindIndex
            if( aPos.nPara + 1 < mpImpl->GetParagraphCount() )
            {
                aResult.SegmentText = mpImpl->GetParagraph( aPos.nPara + 1 ).getText();

                // #112814# Adapt the start index with the paragraph offset
                aResult.SegmentStart = mpImpl->Internal2Index( EPosition( aPos.nPara + 1, 0 ) );
                aResult.SegmentEnd = aResult.SegmentStart + aResult.SegmentText.getLength();
            }
        }
        else
        {
            // No special handling required, forward to wrapped class
            aResult = mpImpl->GetParagraph( aPos.nPara ).getTextBehindIndex( aPos.nIndex, aTextType );

            // #112814# Adapt the start index with the paragraph offset
            mpImpl->CorrectTextSegment( aResult, aPos.nPara );
            if ( bLineBreak )
            {
                aResult.SegmentText = OUStringLiteral1(cNewLine) + aResult.SegmentText;
            }
       }

        return aResult;
    }

    sal_Bool SAL_CALL AccessibleStaticTextBase::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aGuard;

        if( nStartIndex > nEndIndex )
            std::swap(nStartIndex, nEndIndex);

        EPosition aStartIndex( mpImpl->Range2Internal(nStartIndex) );
        EPosition aEndIndex( mpImpl->Range2Internal(nEndIndex) );

        return mpImpl->CopyText( aStartIndex.nPara, aStartIndex.nIndex,
                                 aEndIndex.nPara, aEndIndex.nIndex );
    }

    // XAccessibleTextAttributes
    uno::Sequence< beans::PropertyValue > AccessibleStaticTextBase::getDefaultAttributes( const uno::Sequence< OUString >& RequestedAttributes )
    {
        // get the intersection of the default attributes of all paragraphs

        SolarMutexGuard aGuard;

        PropertyValueVector aDefAttrVec(
                comphelper::sequenceToContainer<PropertyValueVector>(mpImpl->GetParagraph( 0 ).getDefaultAttributes( RequestedAttributes )) );

        const sal_Int32 nParaCount = mpImpl->GetParagraphCount();
        for ( sal_Int32 nPara = 1; nPara < nParaCount; ++nPara )
        {
            uno::Sequence< beans::PropertyValue > aSeq = mpImpl->GetParagraph( nPara ).getDefaultAttributes( RequestedAttributes );
            PropertyValueVector aIntersectionVec;

            for ( const auto& rDefAttr : aDefAttrVec )
            {
                const beans::PropertyValue* pItr = aSeq.getConstArray();
                const beans::PropertyValue* pEnd  = pItr + aSeq.getLength();
                const beans::PropertyValue* pFind = std::find_if( pItr, pEnd, PropertyValueEqualFunctor(rDefAttr) );
                if ( pFind != pEnd )
                {
                    aIntersectionVec.push_back( *pFind );
                }
            }

            aDefAttrVec.swap( aIntersectionVec );

            if ( aDefAttrVec.empty() )
            {
                break;
            }
        }

        return comphelper::containerToSequence(aDefAttrVec);
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL AccessibleStaticTextBase::getRunAttributes( sal_Int32 nIndex, const uno::Sequence< OUString >& RequestedAttributes )
    {
        // get those default attributes of the paragraph, which are not part
        // of the intersection of all paragraphs and add them to the run attributes

        SolarMutexGuard aGuard;

        EPosition aPos( mpImpl->Index2Internal( nIndex ) );
        AccessibleEditableTextPara& rPara = mpImpl->GetParagraph( aPos.nPara );
        uno::Sequence< beans::PropertyValue > aDefAttrSeq = rPara.getDefaultAttributes( RequestedAttributes );
        uno::Sequence< beans::PropertyValue > aRunAttrSeq = rPara.getRunAttributes( aPos.nIndex, RequestedAttributes );
        uno::Sequence< beans::PropertyValue > aIntersectionSeq = getDefaultAttributes( RequestedAttributes );
        PropertyValueVector aDiffVec;

        const beans::PropertyValue* pDefAttr = aDefAttrSeq.getConstArray();
        const sal_Int32 nLength = aDefAttrSeq.getLength();
        for ( sal_Int32 i = 0; i < nLength; ++i )
        {
            const beans::PropertyValue* pItr = aIntersectionSeq.getConstArray();
            const beans::PropertyValue* pEnd  = pItr + aIntersectionSeq.getLength();
            bool bNone = std::none_of( pItr, pEnd, PropertyValueEqualFunctor( pDefAttr[i] ) );
            if ( bNone && pDefAttr[i].Handle != 0)
            {
                aDiffVec.push_back( pDefAttr[i] );
            }
        }

        return ::comphelper::concatSequences( aRunAttrSeq, comphelper::containerToSequence(aDiffVec) );
    }

    tools::Rectangle AccessibleStaticTextBase::GetParagraphBoundingBox() const
    {
        return mpImpl->GetParagraphBoundingBox();
    }

}  // end of namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

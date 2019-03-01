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

#include "PresenterTextView.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterTimer.hxx"

#include <algorithm>
#include <cmath>
#include <numeric>

#include <sal/log.hxx>

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/CTLScriptType.hpp>
#include <com/sun/star/i18n/ScriptDirection.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

const static sal_Int64 CaretBlinkInterval = 500 * 1000 * 1000;

//#define SHOW_CHARACTER_BOXES

namespace {
    sal_Int32 Signum (const sal_Int32 nValue)
    {
        if (nValue < 0)
            return -1;
        else if (nValue > 0)
            return +1;
        else
            return 0;
    }
}

namespace sdext { namespace presenter {

//===== PresenterTextView =====================================================

PresenterTextView::PresenterTextView (
    const Reference<XComponentContext>& rxContext,
    const Reference<rendering::XCanvas>& rxCanvas,
    const ::std::function<void (const css::awt::Rectangle&)>& rInvalidator)
    : mxCanvas(rxCanvas),
      mxBreakIterator(),
      mxScriptTypeDetector(),
      maLocation(0,0),
      maSize(0,0),
      mpFont(),
      maParagraphs(),
      mpCaret(new PresenterTextCaret(
          rxContext,
          [this] (sal_Int32 const nParagraphIndex, sal_Int32 const nCharacterIndex)
              { return this->GetCaretBounds(nParagraphIndex, nCharacterIndex); },
          rInvalidator)),
      mnLeftOffset(0),
      mnTopOffset(0),
      mbIsFormatPending(false),
      maTextChangeBroadcaster()
{
    Reference<lang::XMultiComponentFactory> xFactory (
        rxContext->getServiceManager(), UNO_QUERY);
    if ( ! xFactory.is())
        return;

    // Create the break iterator that we use to break text into lines.
    mxBreakIterator = i18n::BreakIterator::create(rxContext);

    // Create the script type detector that is used to split paragraphs into
    // portions of the same text direction.
    mxScriptTypeDetector.set(
        xFactory->createInstanceWithContext(
            "com.sun.star.i18n.ScriptTypeDetector",
            rxContext),
        UNO_QUERY_THROW);
}

void PresenterTextView::SetText (const Reference<text::XText>& rxText)
{
    maParagraphs.clear();

    Reference<container::XEnumerationAccess> xParagraphAccess (rxText, UNO_QUERY);
    if ( ! xParagraphAccess.is())
        return;

    Reference<container::XEnumeration> xParagraphs (
        xParagraphAccess->createEnumeration() , UNO_QUERY);
    if ( ! xParagraphs.is())
        return;

    if ( ! mpFont || ! mpFont->PrepareFont(mxCanvas))
        return;

    sal_Int32 nCharacterCount (0);
    while (xParagraphs->hasMoreElements())
    {
        SharedPresenterTextParagraph pParagraph (new PresenterTextParagraph(
            maParagraphs.size(),
            mxBreakIterator,
            mxScriptTypeDetector,
            Reference<text::XTextRange>(xParagraphs->nextElement(), UNO_QUERY),
            mpCaret));
        pParagraph->SetupCellArray(mpFont);
        pParagraph->SetCharacterOffset(nCharacterCount);
        nCharacterCount += pParagraph->GetCharacterCount();
        maParagraphs.push_back(pParagraph);
    }

    if (mpCaret)
        mpCaret->HideCaret();

    RequestFormat();
}

void PresenterTextView::SetTextChangeBroadcaster (
    const ::std::function<void ()>& rBroadcaster)
{
    maTextChangeBroadcaster = rBroadcaster;
}

void PresenterTextView::SetLocation (const css::geometry::RealPoint2D& rLocation)
{
    maLocation = rLocation;

    for (auto& rxParagraph : maParagraphs)
    {
        rxParagraph->SetOrigin(
            maLocation.X - mnLeftOffset,
            maLocation.Y - mnTopOffset);
    }
}

void PresenterTextView::SetSize (const css::geometry::RealSize2D& rSize)
{
    maSize = rSize;
    RequestFormat();
}

double PresenterTextView::GetTotalTextHeight()
{
    if (mbIsFormatPending)
    {
        if ( ! mpFont->PrepareFont(mxCanvas))
            return 0;
        Format();
    }

    return std::accumulate(maParagraphs.begin(), maParagraphs.end(), double(0),
        [](const double& nTotalHeight, const SharedPresenterTextParagraph& rxParagraph) {
            return nTotalHeight + rxParagraph->GetTotalTextHeight();
        });
}

void PresenterTextView::SetFont (const PresenterTheme::SharedFontDescriptor& rpFont)
{
    mpFont = rpFont;
    RequestFormat();
}

void PresenterTextView::SetOffset(
    const double nLeft,
    const double nTop)
{
    mnLeftOffset = nLeft;
    mnTopOffset = nTop;

    // Trigger an update of the text origin stored at the individual paragraphs.
    SetLocation(maLocation);
}

void PresenterTextView::MoveCaret (
    const sal_Int32 nDistance,
    const sal_Int16 nTextType)
{
    if ( ! mpCaret)
        return;

    // When the caret has not been visible yet then move it to the beginning
    // of the text.
    if (mpCaret->GetParagraphIndex() < 0)
    {
        mpCaret->SetPosition(0,0);
        return;
    }

    sal_Int32 nParagraphIndex (mpCaret->GetParagraphIndex());
    sal_Int32 nCharacterIndex (mpCaret->GetCharacterIndex());
    switch (nTextType)
    {
        default:
        case AccessibleTextType::CHARACTER:
            nCharacterIndex += nDistance;
            break;

        case AccessibleTextType::WORD:
        {
            sal_Int32 nRemainingDistance (nDistance);
            while (nRemainingDistance != 0)
            {
                SharedPresenterTextParagraph pParagraph (GetParagraph(nParagraphIndex));
                if (pParagraph)
                {
                    const sal_Int32 nDelta (Signum(nDistance));
                    nCharacterIndex = pParagraph->GetWordBoundary(nCharacterIndex, nDelta);
                    if (nCharacterIndex < 0)
                    {
                        // Go to previous or next paragraph.
                        nParagraphIndex += nDelta;
                        if (nParagraphIndex < 0)
                        {
                            nParagraphIndex = 0;
                            nCharacterIndex = 0;
                            nRemainingDistance = 0;
                        }
                        else if (sal_uInt32(nParagraphIndex) >= maParagraphs.size())
                        {
                            nParagraphIndex = maParagraphs.size()-1;
                            pParagraph = GetParagraph(nParagraphIndex);
                            if (pParagraph)
                                nCharacterIndex = pParagraph->GetCharacterCount();
                            nRemainingDistance = 0;
                        }
                        else
                        {
                            nRemainingDistance -= nDelta;

                            // Move caret one character to the end of
                            // the previous or the start of the next paragraph.
                            pParagraph = GetParagraph(nParagraphIndex);
                            if (pParagraph)
                            {
                                if (nDistance<0)
                                    nCharacterIndex = pParagraph->GetCharacterCount();
                                else
                                    nCharacterIndex = 0;
                            }
                        }
                    }
                    else
                        nRemainingDistance -= nDelta;
                }
                else
                    break;
            }
            break;
        }
    }

    // Move the caret to the new position.
    mpCaret->SetPosition(nParagraphIndex, nCharacterIndex);
}

void PresenterTextView::Paint (
    const css::awt::Rectangle& rUpdateBox)
{
    if ( ! mxCanvas.is())
        return;
    if ( ! mpFont->PrepareFont(mxCanvas))
        return;

    if (mbIsFormatPending)
        Format();

    // Setup the clipping rectangle.  Horizontally we make it a little
    // larger to allow characters (and the caret) to stick out of their
    // bounding boxes.  This can happen on some characters (like the
    // uppercase J) for typographical reasons.
    const sal_Int32 nAdditionalLeftBorder (10);
    const sal_Int32 nAdditionalRightBorder (5);
    double nX (maLocation.X - mnLeftOffset);
    double nY (maLocation.Y - mnTopOffset);
    const sal_Int32 nClipLeft (::std::max(
        PresenterGeometryHelper::Round(maLocation.X)-nAdditionalLeftBorder, rUpdateBox.X));
    const sal_Int32 nClipTop (::std::max(
        PresenterGeometryHelper::Round(maLocation.Y), rUpdateBox.Y));
    const sal_Int32 nClipRight (::std::min(
        PresenterGeometryHelper::Round(maLocation.X+maSize.Width)+nAdditionalRightBorder, rUpdateBox.X+rUpdateBox.Width));
    const sal_Int32 nClipBottom (::std::min(
        PresenterGeometryHelper::Round(maLocation.Y+maSize.Height), rUpdateBox.Y+rUpdateBox.Height));
    if (nClipLeft>=nClipRight || nClipTop>=nClipBottom)
        return;

    const awt::Rectangle aClipBox(
        nClipLeft,
        nClipTop,
        nClipRight - nClipLeft,
        nClipBottom - nClipTop);
    Reference<rendering::XPolyPolygon2D> xClipPolygon (
        PresenterGeometryHelper::CreatePolygon(aClipBox, mxCanvas->getDevice()));

    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        xClipPolygon);

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,nX, 0,1,nY),
        nullptr,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, mpFont->mnColor);

    for (const auto& rxParagraph : maParagraphs)
    {
        rxParagraph->Paint(
            mxCanvas,
            maSize,
            mpFont,
            aViewState,
            aRenderState,
            mnTopOffset,
            nClipTop,
            nClipBottom);
    }

    aRenderState.AffineTransform.m02 = 0;
    aRenderState.AffineTransform.m12 = 0;

#ifdef SHOW_CHARACTER_BOXES
    PresenterCanvasHelper::SetDeviceColor(aRenderState, 0x00808080);
    for (sal_Int32 nParagraphIndex(0), nParagraphCount(GetParagraphCount());
         nParagraphIndex<nParagraphCount;
         ++nParagraphIndex)
    {
        const SharedPresenterTextParagraph pParagraph (GetParagraph(nParagraphIndex));
        if ( ! pParagraph)
            continue;
        for (sal_Int32 nCharacterIndex(0),nCharacterCount(pParagraph->GetCharacterCount());
             nCharacterIndex<nCharacterCount; ++nCharacterIndex)
        {
            const awt::Rectangle aBox (pParagraph->GetCharacterBounds(nCharacterIndex, false));
            mxCanvas->drawPolyPolygon (
                PresenterGeometryHelper::CreatePolygon(
                    aBox,
                    mxCanvas->getDevice()),
                aViewState,
                aRenderState);
        }
    }
    PresenterCanvasHelper::SetDeviceColor(aRenderState, mpFont->mnColor);
#endif

    if (mpCaret && mpCaret->IsVisible())
    {
        mxCanvas->fillPolyPolygon (
            PresenterGeometryHelper::CreatePolygon(
                mpCaret->GetBounds(),
                mxCanvas->getDevice()),
            aViewState,
            aRenderState);
    }
}

const SharedPresenterTextCaret& PresenterTextView::GetCaret() const
{
    return mpCaret;
}

awt::Rectangle PresenterTextView::GetCaretBounds (
    sal_Int32 nParagraphIndex,
    const sal_Int32 nCharacterIndex) const
{
    SharedPresenterTextParagraph pParagraph (GetParagraph(nParagraphIndex));

    if (pParagraph)
        return pParagraph->GetCharacterBounds(nCharacterIndex, true);
    else
        return awt::Rectangle(0,0,0,0);
}

//----- private ---------------------------------------------------------------

void PresenterTextView::RequestFormat()
{
    mbIsFormatPending = true;
}

void PresenterTextView::Format()
{
    mbIsFormatPending = false;

    double nY (0);
    for (const auto& rxParagraph : maParagraphs)
    {
        rxParagraph->Format(nY, maSize.Width, mpFont);
        nY += rxParagraph->GetTotalTextHeight();
    }

    if (maTextChangeBroadcaster)
        maTextChangeBroadcaster();
}

sal_Int32 PresenterTextView::GetParagraphCount() const
{
    return maParagraphs.size();
}

SharedPresenterTextParagraph PresenterTextView::GetParagraph (
    const sal_Int32 nParagraphIndex) const
{
    if (nParagraphIndex < 0)
        return SharedPresenterTextParagraph();
    else if (nParagraphIndex>=sal_Int32(maParagraphs.size()))
        return SharedPresenterTextParagraph();
    else
        return maParagraphs[nParagraphIndex];
}

//===== PresenterTextParagraph ================================================

PresenterTextParagraph::PresenterTextParagraph (
    const sal_Int32 nParagraphIndex,
    const Reference<i18n::XBreakIterator>& rxBreakIterator,
    const Reference<i18n::XScriptTypeDetector>& rxScriptTypeDetector,
    const Reference<text::XTextRange>& rxTextRange,
    const SharedPresenterTextCaret& rpCaret)
    : msParagraphText(),
      mnParagraphIndex(nParagraphIndex),
      mpCaret(rpCaret),
      mxBreakIterator(rxBreakIterator),
      mxScriptTypeDetector(rxScriptTypeDetector),
      maLines(),
      mnVerticalOffset(0),
      mnXOrigin(0),
      mnYOrigin(0),
      mnWidth(0),
      mnAscent(0),
      mnDescent(0),
      mnLineHeight(-1),
      mnWritingMode (text::WritingMode2::LR_TB),
      mnCharacterOffset(0),
      maCells()
{
    if (!rxTextRange.is())
        return;

    Reference<beans::XPropertySet> xProperties (rxTextRange, UNO_QUERY);
    try
    {
        xProperties->getPropertyValue("WritingMode") >>= mnWritingMode;
    }
    catch(beans::UnknownPropertyException&)
    {
        // Ignore the exception.  Use the default value.
    }

    msParagraphText = rxTextRange->getString();
}

void PresenterTextParagraph::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const geometry::RealSize2D& rSize,
    const PresenterTheme::SharedFontDescriptor& rpFont,
    const rendering::ViewState& rViewState,
    rendering::RenderState& rRenderState,
    const double nTopOffset,
    const double nClipTop,
    const double nClipBottom)
{
    if (mnLineHeight <= 0)
        return;

    sal_Int8 nTextDirection (GetTextDirection());

    const double nSavedM12 (rRenderState.AffineTransform.m12);

    if ( ! IsTextReferencePointLeft())
        rRenderState.AffineTransform.m02 += rSize.Width;

#ifdef SHOW_CHARACTER_BOXES
    for (sal_Int32 nIndex=0,nCount=maLines.size();
         nIndex<nCount;
         ++nIndex)
    {
        Line& rLine (maLines[nIndex]);
        rLine.ProvideLayoutedLine(msParagraphText, rpFont, nTextDirection);
    }
#endif

    for (sal_Int32 nIndex=0,nCount=maLines.size();
         nIndex<nCount;
         ++nIndex, rRenderState.AffineTransform.m12 += mnLineHeight)
    {
        Line& rLine (maLines[nIndex]);

        // Paint only visible lines.
        const double nLineTop = rLine.mnBaseLine - mnAscent - nTopOffset;
        if (nLineTop + mnLineHeight< nClipTop)
            continue;
        else if (nLineTop > nClipBottom)
            break;
        rLine.ProvideLayoutedLine(msParagraphText, rpFont, nTextDirection);

        rRenderState.AffineTransform.m12 = nSavedM12 + rLine.mnBaseLine;

        rxCanvas->drawTextLayout (
            rLine.mxLayoutedLine,
            rViewState,
            rRenderState);
    }
    rRenderState.AffineTransform.m12 = nSavedM12;

    if ( ! IsTextReferencePointLeft())
        rRenderState.AffineTransform.m02 -= rSize.Width;
}

void PresenterTextParagraph::Format (
    const double nY,
    const double nWidth,
    const PresenterTheme::SharedFontDescriptor& rpFont)
{
    // Make sure that the text view is in a valid and sane state.
    if ( ! mxBreakIterator.is() || ! mxScriptTypeDetector.is())
        return;
    if (nWidth<=0)
        return;
    if ( ! rpFont || ! rpFont->mxFont.is())
        return;

    sal_Int32 nPosition (0);

    mnWidth = nWidth;
    maLines.clear();
    mnLineHeight = 0;
    mnAscent = 0;
    mnDescent = 0;
    mnVerticalOffset = nY;
    maWordBoundaries.clear();
    maWordBoundaries.push_back(0);

    const rendering::FontMetrics aMetrics (rpFont->mxFont->getFontMetrics());
    mnAscent = aMetrics.Ascent;
    mnDescent = aMetrics.Descent;
    mnLineHeight = aMetrics.Ascent + aMetrics.Descent + aMetrics.ExternalLeading;
    nPosition = 0;
    i18n::Boundary aCurrentLine(0,0);
    while (true)
    {
        const i18n::Boundary aWordBoundary = mxBreakIterator->nextWord(
            msParagraphText,
            nPosition,
            lang::Locale(),
            i18n::WordType::ANYWORD_IGNOREWHITESPACES);
        AddWord(nWidth, aCurrentLine, aWordBoundary.startPos, rpFont);

        // Remember the new word boundary for caret travelling by words.
        // Prevent duplicates.
        if (aWordBoundary.startPos > maWordBoundaries.back())
            maWordBoundaries.push_back(aWordBoundary.startPos);

        if (aWordBoundary.endPos>aWordBoundary.startPos)
            AddWord(nWidth, aCurrentLine, aWordBoundary.endPos, rpFont);

        if (aWordBoundary.startPos<0 || aWordBoundary.endPos<0)
            break;
        if (nPosition >= aWordBoundary.endPos)
            break;
        nPosition = aWordBoundary.endPos;
    }

    if (aCurrentLine.endPos>aCurrentLine.startPos)
        AddLine(aCurrentLine);

}

sal_Int32 PresenterTextParagraph::GetWordBoundary(
    const sal_Int32 nLocalCharacterIndex,
    const sal_Int32 nDistance)
{
    OSL_ASSERT(nDistance==-1 || nDistance==+1);

    if (nLocalCharacterIndex < 0)
    {
        // The caller asked for the start or end position of the paragraph.
        if (nDistance < 0)
            return 0;
        else
            return GetCharacterCount();
    }

    sal_Int32 nIndex (0);
    for (sal_Int32 nCount (maWordBoundaries.size()); nIndex<nCount; ++nIndex)
    {
        if (maWordBoundaries[nIndex] >= nLocalCharacterIndex)
        {
            // When inside the word (not at its start or end) then
            // first move to the start or end before going the previous or
            // next word.
            if (maWordBoundaries[nIndex] > nLocalCharacterIndex)
                if (nDistance > 0)
                    --nIndex;
            break;
        }
    }

    nIndex += nDistance;

    if (nIndex < 0)
        return -1;
    else if (sal_uInt32(nIndex)>=maWordBoundaries.size())
        return -1;
    else
        return maWordBoundaries[nIndex];
}

sal_Int32 PresenterTextParagraph::GetCaretPosition() const
{
    if (mpCaret && mpCaret->GetParagraphIndex()==mnParagraphIndex)
        return mpCaret->GetCharacterIndex();
    else
        return -1;
}

void PresenterTextParagraph::SetCaretPosition (const sal_Int32 nPosition) const
{
    if (mpCaret && mpCaret->GetParagraphIndex()==mnParagraphIndex)
        return mpCaret->SetPosition(mnParagraphIndex, nPosition);
}

void PresenterTextParagraph::SetOrigin (const double nXOrigin, const double nYOrigin)
{
    mnXOrigin = nXOrigin;
    mnYOrigin = nYOrigin;
}

awt::Point PresenterTextParagraph::GetRelativeLocation() const
{
    return awt::Point(
        sal_Int32(mnXOrigin),
        sal_Int32(mnYOrigin + mnVerticalOffset));
}

awt::Size PresenterTextParagraph::GetSize()
{
    return awt::Size(
        sal_Int32(mnWidth),
        sal_Int32(GetTotalTextHeight()));
}

void PresenterTextParagraph::AddWord (
    const double nWidth,
    i18n::Boundary& rCurrentLine,
    const sal_Int32 nWordBoundary,
    const PresenterTheme::SharedFontDescriptor& rpFont)
{
    sal_Int32 nLineStart (0);
    if ( ! maLines.empty())
        nLineStart = rCurrentLine.startPos;

    const OUString sLineCandidate (
        msParagraphText.copy(nLineStart, nWordBoundary-nLineStart));

    css::geometry::RealRectangle2D aLineBox (
        PresenterCanvasHelper::GetTextBoundingBox (
            rpFont->mxFont,
            sLineCandidate,
            mnWritingMode));
    const double nLineWidth (aLineBox.X2 - aLineBox.X1);

    if (nLineWidth >= nWidth)
    {
        // Add new line with a single word (so far).
        AddLine(rCurrentLine);
    }
    rCurrentLine.endPos = nWordBoundary;
}

void PresenterTextParagraph::AddLine (
    i18n::Boundary& rCurrentLine)
{
    Line aLine (rCurrentLine.startPos, rCurrentLine.endPos);

    // Find the start and end of the line with respect to cells.
    if (!maLines.empty())
    {
        aLine.mnLineStartCellIndex = maLines.back().mnLineEndCellIndex;
        aLine.mnBaseLine = maLines.back().mnBaseLine + mnLineHeight;
    }
    else
    {
        aLine.mnLineStartCellIndex = 0;
        aLine.mnBaseLine = mnVerticalOffset + mnAscent;
    }
    sal_Int32 nCellIndex (aLine.mnLineStartCellIndex);
    double nWidth (0);
    for ( ; nCellIndex<sal_Int32(maCells.size()); ++nCellIndex)
    {
        const Cell& rCell (maCells[nCellIndex]);
        if (rCell.mnCharacterIndex+rCell.mnCharacterCount > aLine.mnLineEndCharacterIndex)
            break;
        nWidth += rCell.mnCellWidth;
    }
    aLine.mnLineEndCellIndex = nCellIndex;
    aLine.mnWidth = nWidth;

    maLines.push_back(aLine);

    rCurrentLine.startPos = rCurrentLine.endPos;
}

double PresenterTextParagraph::GetTotalTextHeight()
{
    return maLines.size() * mnLineHeight;
}

void PresenterTextParagraph::SetCharacterOffset (const sal_Int32 nCharacterOffset)
{
    mnCharacterOffset = nCharacterOffset;
}

sal_Int32 PresenterTextParagraph::GetCharacterCount() const
{
    return msParagraphText.getLength();
}

sal_Unicode PresenterTextParagraph::GetCharacter (
    const sal_Int32 nGlobalCharacterIndex) const
{
    if (nGlobalCharacterIndex<mnCharacterOffset
        || nGlobalCharacterIndex>=mnCharacterOffset+msParagraphText.getLength())
    {
        return sal_Unicode();
    }
    else
    {
        return msParagraphText[nGlobalCharacterIndex - mnCharacterOffset];
    }
}

const OUString& PresenterTextParagraph::GetText() const
{
    return msParagraphText;
}

TextSegment PresenterTextParagraph::GetTextSegment (
    const sal_Int32 nOffset,
    const sal_Int32 nIndex,
    const sal_Int16 nTextType) const
{
    switch(nTextType)
    {
        case AccessibleTextType::PARAGRAPH:
            return TextSegment(
                msParagraphText,
                mnCharacterOffset,
                mnCharacterOffset+msParagraphText.getLength());

        case AccessibleTextType::SENTENCE:
            if (mxBreakIterator.is())
            {
                const sal_Int32 nStart (mxBreakIterator->beginOfSentence(
                    msParagraphText, nIndex-mnCharacterOffset, lang::Locale()));
                const sal_Int32 nEnd (mxBreakIterator->endOfSentence(
                    msParagraphText, nIndex-mnCharacterOffset, lang::Locale()));
                if (nStart < nEnd)
                    return TextSegment(
                        msParagraphText.copy(nStart, nEnd-nStart),
                        nStart+mnCharacterOffset,
                        nEnd+mnCharacterOffset);
            }
            break;

        case AccessibleTextType::WORD:
            if (mxBreakIterator.is())
                return GetWordTextSegment(nOffset, nIndex);
            break;

        case AccessibleTextType::LINE:
        {
            auto iLine = std::find_if(maLines.begin(), maLines.end(),
                [nIndex](const Line& rLine) { return nIndex < rLine.mnLineEndCharacterIndex; });
            if (iLine != maLines.end())
            {
                return TextSegment(
                    msParagraphText.copy(
                        iLine->mnLineStartCharacterIndex,
                        iLine->mnLineEndCharacterIndex - iLine->mnLineStartCharacterIndex),
                    iLine->mnLineStartCharacterIndex,
                    iLine->mnLineEndCharacterIndex);
            }
        }
        break;

        // Handle GLYPH and ATTRIBUTE_RUN like CHARACTER because we can not
        // do better at the moment.
        case AccessibleTextType::CHARACTER:
        case AccessibleTextType::GLYPH:
        case AccessibleTextType::ATTRIBUTE_RUN:
            return CreateTextSegment(nIndex+nOffset, nIndex+nOffset+1);
    }

    return TextSegment(OUString(), 0,0);
}

TextSegment PresenterTextParagraph::GetWordTextSegment (
    const sal_Int32 nOffset,
    const sal_Int32 nIndex) const
{
    sal_Int32 nCurrentOffset (nOffset);
    sal_Int32 nCurrentIndex (nIndex);

    i18n::Boundary aWordBoundary;
    if (nCurrentOffset == 0)
        aWordBoundary = mxBreakIterator->getWordBoundary(
            msParagraphText,
            nIndex,
            lang::Locale(),
            i18n::WordType::ANYWORD_IGNOREWHITESPACES,
            true);
    else if (nCurrentOffset < 0)
    {
        while (nCurrentOffset<0 && nCurrentIndex>0)
        {
            aWordBoundary = mxBreakIterator->previousWord(
                msParagraphText,
                nCurrentIndex,
                lang::Locale(),
                i18n::WordType::ANYWORD_IGNOREWHITESPACES);
            nCurrentIndex = aWordBoundary.startPos;
            ++nCurrentOffset;
        }
    }
    else
    {
        while (nCurrentOffset>0 && nCurrentIndex<=GetCharacterCount())
        {
            aWordBoundary = mxBreakIterator->nextWord(
                msParagraphText,
                nCurrentIndex,
                lang::Locale(),
                i18n::WordType::ANYWORD_IGNOREWHITESPACES);
            nCurrentIndex = aWordBoundary.endPos;
            --nCurrentOffset;
        }
    }

    return CreateTextSegment(aWordBoundary.startPos, aWordBoundary.endPos);
}

TextSegment PresenterTextParagraph::CreateTextSegment (
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex) const
{
    if (nEndIndex <= nStartIndex)
        return TextSegment(
            OUString(),
            nStartIndex,
            nEndIndex);
    else
        return TextSegment(
            msParagraphText.copy(nStartIndex, nEndIndex-nStartIndex),
            nStartIndex,
            nEndIndex);
}

awt::Rectangle PresenterTextParagraph::GetCharacterBounds (
    sal_Int32 nGlobalCharacterIndex,
    const bool bCaretBox)
{
    // Find the line that contains the requested character and accumulate
    // the previous line heights.
    double nX (mnXOrigin);
    double nY (mnYOrigin + mnVerticalOffset + mnAscent);
    const sal_Int8 nTextDirection (GetTextDirection());
    for (sal_Int32 nLineIndex=0,nLineCount=maLines.size();
         nLineIndex<nLineCount;
         ++nLineIndex, nY+=mnLineHeight)
    {
        Line& rLine (maLines[nLineIndex]);
        // Skip lines before the indexed character.
        if (nGlobalCharacterIndex >= rLine.mnLineEndCharacterIndex)
            // When in the last line then allow the index past the last char.
            if (nLineIndex<nLineCount-1)
                continue;

        rLine.ProvideCellBoxes();

        const sal_Int32 nCellIndex (nGlobalCharacterIndex - rLine.mnLineStartCharacterIndex);

        // The cell bounding box is defined relative to the origin of
        // the current line.  Therefore we have to add the absolute
        // position of the line.
        geometry::RealRectangle2D rCellBox (rLine.maCellBoxes[
            ::std::min(nCellIndex, rLine.maCellBoxes.getLength()-1)]);

        double nLeft = nX + rCellBox.X1;
        double nRight = nX + rCellBox.X2;
        if (nTextDirection == rendering::TextDirection::WEAK_RIGHT_TO_LEFT)
        {
            const double nOldRight (nRight);
            nRight = rLine.mnWidth - nLeft;
            nLeft = rLine.mnWidth - nOldRight;
        }
        double nTop = nY - mnAscent;
        double nBottom;
        if (bCaretBox)
        {
            nBottom = nTop + mnLineHeight;
            if (nCellIndex >= rLine.maCellBoxes.getLength())
                nLeft = nRight-2;
            if (nLeft < nX)
                nLeft = nX;
            nRight = nLeft+2;
        }
        else
        {
            nBottom = nTop + mnAscent + mnDescent;
        }
        const sal_Int32 nX1 = sal_Int32(floor(nLeft));
        const sal_Int32 nY1 = sal_Int32(floor(nTop));
        const sal_Int32 nX2 = sal_Int32(ceil(nRight));
        const sal_Int32 nY2 = sal_Int32(ceil(nBottom));

        return awt::Rectangle(nX1,nY1,nX2-nX1+1,nY2-nY1+1);
    }

    // We are still here.  That means that the given index lies past the
    // last character in the paragraph.
    // Return an empty box that lies past the last character.  Better than nothing.
    return awt::Rectangle(sal_Int32(nX+0.5), sal_Int32(nY+0.5), 0, 0);
}

sal_Int8 PresenterTextParagraph::GetTextDirection() const
{
    // Find first portion that has a non-neutral text direction.
    sal_Int32 nPosition (0);
    sal_Int32 nTextLength (msParagraphText.getLength());
    while (nPosition < nTextLength)
    {
        const sal_Int16 nScriptDirection (
            mxScriptTypeDetector->getScriptDirection(
                msParagraphText, nPosition, i18n::ScriptDirection::NEUTRAL));
        switch (nScriptDirection)
        {
            case i18n::ScriptDirection::NEUTRAL:
                // continue looping.
                break;
            case i18n::ScriptDirection::LEFT_TO_RIGHT:
                return rendering::TextDirection::WEAK_LEFT_TO_RIGHT;

            case i18n::ScriptDirection::RIGHT_TO_LEFT:
                return rendering::TextDirection::WEAK_RIGHT_TO_LEFT;
        }

        nPosition = mxScriptTypeDetector->endOfScriptDirection(
            msParagraphText, nPosition, nScriptDirection);
    }

    // All text in paragraph is neutral.  Fall back on writing mode taken
    // from the XText (which may not be properly initialized.)
    sal_Int8 nTextDirection(rendering::TextDirection::WEAK_LEFT_TO_RIGHT);
    switch(mnWritingMode)
    {
        case text::WritingMode2::LR_TB:
            nTextDirection = rendering::TextDirection::WEAK_LEFT_TO_RIGHT;
            break;

        case text::WritingMode2::RL_TB:
            nTextDirection = rendering::TextDirection::WEAK_RIGHT_TO_LEFT;
            break;

        default:
        case text::WritingMode2::TB_RL:
        case text::WritingMode2::TB_LR:
            // Can not handle this.  Use default and hope for the best.
            break;
    }
    return nTextDirection;
}

bool PresenterTextParagraph::IsTextReferencePointLeft() const
{
    return mnWritingMode != text::WritingMode2::RL_TB;
}

void PresenterTextParagraph::SetupCellArray (
    const PresenterTheme::SharedFontDescriptor& rpFont)
{
    maCells.clear();

    if ( ! rpFont || ! rpFont->mxFont.is())
        return;

    sal_Int32 nPosition (0);
    sal_Int32 nIndex (0);
    const sal_Int32 nTextLength (msParagraphText.getLength());
    const sal_Int8 nTextDirection (GetTextDirection());
    while (nPosition < nTextLength)
    {
        const sal_Int32 nNewPosition (mxBreakIterator->nextCharacters(
            msParagraphText,
            nPosition,
            lang::Locale(),
            i18n::CharacterIteratorMode::SKIPCELL,
            1,
            nIndex));

        rendering::StringContext aContext (msParagraphText, nPosition, nNewPosition-nPosition);
        Reference<rendering::XTextLayout> xLayout (
            rpFont->mxFont->createTextLayout(aContext, nTextDirection, 0));
        css::geometry::RealRectangle2D aCharacterBox (xLayout->queryTextBounds());

        maCells.emplace_back(
            nPosition,
            nNewPosition-nPosition,
            aCharacterBox.X2-aCharacterBox.X1);

        nPosition = nNewPosition;
    }
}

//===== PresenterTextCaret ================================================----

PresenterTextCaret::PresenterTextCaret (
        uno::Reference<uno::XComponentContext> const& xContext,
    const ::std::function<css::awt::Rectangle (const sal_Int32,const sal_Int32)>& rCharacterBoundsAccess,
    const ::std::function<void (const css::awt::Rectangle&)>& rInvalidator)
    : m_xContext(xContext)
    , mnParagraphIndex(-1),
      mnCharacterIndex(-1),
      mnCaretBlinkTaskId(0),
      mbIsCaretVisible(false),
      maCharacterBoundsAccess(rCharacterBoundsAccess),
      maInvalidator(rInvalidator),
      maBroadcaster(),
      maCaretBounds()
{
}

PresenterTextCaret::~PresenterTextCaret()
{
    try
    {
        HideCaret();
    }
    catch (uno::Exception const&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("sdext.presenter", "unexpected exception in ~PresenterTextCaret " << exceptionToString(ex));
    }
}

void PresenterTextCaret::ShowCaret()
{
    if (mnCaretBlinkTaskId == 0)
    {
        mnCaretBlinkTaskId = PresenterTimer::ScheduleRepeatedTask (
            m_xContext,
            [this] (TimeValue const&) { return this->InvertCaret(); },
            CaretBlinkInterval,
            CaretBlinkInterval);
    }
    mbIsCaretVisible = true;
}

void PresenterTextCaret::HideCaret()
{
    if (mnCaretBlinkTaskId != 0)
    {
        PresenterTimer::CancelTask(mnCaretBlinkTaskId);
        mnCaretBlinkTaskId = 0;
    }
    mbIsCaretVisible = false;
    // Reset the caret position.
    mnParagraphIndex = -1;
    mnCharacterIndex = -1;
}


void PresenterTextCaret::SetPosition (
    const sal_Int32 nParagraphIndex,
    const sal_Int32 nCharacterIndex)
{
    if (mnParagraphIndex == nParagraphIndex
        && mnCharacterIndex == nCharacterIndex)
        return;

    if (mnParagraphIndex >= 0)
        maInvalidator(maCaretBounds);

    const sal_Int32 nOldParagraphIndex (mnParagraphIndex);
    const sal_Int32 nOldCharacterIndex (mnCharacterIndex);
    mnParagraphIndex = nParagraphIndex;
    mnCharacterIndex = nCharacterIndex;
    maCaretBounds = maCharacterBoundsAccess(mnParagraphIndex, mnCharacterIndex);
    if (mnParagraphIndex >= 0)
        ShowCaret();
    else
        HideCaret();

    if (mnParagraphIndex >= 0)
        maInvalidator(maCaretBounds);

    if (maBroadcaster)
        maBroadcaster(
            nOldParagraphIndex,
            nOldCharacterIndex,
            mnParagraphIndex,
            mnCharacterIndex);
}


void PresenterTextCaret::SetCaretMotionBroadcaster (
    const ::std::function<void (sal_Int32,sal_Int32,sal_Int32,sal_Int32)>& rBroadcaster)
{
    maBroadcaster = rBroadcaster;
}

const css::awt::Rectangle& PresenterTextCaret::GetBounds() const
{
    return maCaretBounds;
}

void PresenterTextCaret::InvertCaret()
{
    mbIsCaretVisible = !mbIsCaretVisible;
    if (mnParagraphIndex >= 0)
        maInvalidator(maCaretBounds);
}

//===== PresenterTextParagraph::Cell ==========================================

PresenterTextParagraph::Cell::Cell (
    const sal_Int32 nCharacterIndex,
    const sal_Int32 nCharacterCount,
    const double nCellWidth)
    : mnCharacterIndex(nCharacterIndex),
      mnCharacterCount(nCharacterCount),
      mnCellWidth(nCellWidth)
{
}

//===== PresenterTextParagraph::Line ==========================================

PresenterTextParagraph::Line::Line (
    const sal_Int32 nLineStartCharacterIndex,
    const sal_Int32 nLineEndCharacterIndex)
    : mnLineStartCharacterIndex(nLineStartCharacterIndex),
      mnLineEndCharacterIndex(nLineEndCharacterIndex),
      mnLineStartCellIndex(-1), mnLineEndCellIndex(-1),
      mxLayoutedLine(),
      mnBaseLine(0), mnWidth(0),
      maCellBoxes()
{
}

void PresenterTextParagraph::Line::ProvideCellBoxes()
{
    if ( mnLineStartCharacterIndex < mnLineEndCharacterIndex && maCellBoxes.getLength()==0 )
    {
        if (mxLayoutedLine.is())
            maCellBoxes = mxLayoutedLine->queryInkMeasures();
        else
        {
            OSL_ASSERT(mxLayoutedLine.is());
        }
    }
}

void PresenterTextParagraph::Line::ProvideLayoutedLine (
    const OUString& rsParagraphText,
    const PresenterTheme::SharedFontDescriptor& rpFont,
    const sal_Int8 nTextDirection)
{
    if ( ! mxLayoutedLine.is())
    {
        const rendering::StringContext aContext (
            rsParagraphText,
            mnLineStartCharacterIndex,
            mnLineEndCharacterIndex - mnLineStartCharacterIndex);

        mxLayoutedLine = rpFont->mxFont->createTextLayout(
            aContext,
            nTextDirection,
            0);
    }
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

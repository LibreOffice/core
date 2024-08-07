/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ToxTabStopTokenHandler.hxx>

#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/boxitem.hxx>

#include <cntfrm.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <ndtxt.hxx>
#include <pagedesc.hxx>
#include <pagefrm.hxx>
#include <swrect.hxx>
#include <tox.hxx>

namespace sw {

DefaultToxTabStopTokenHandler::DefaultToxTabStopTokenHandler(SwNodeOffset indexOfSectionNode,
        const SwPageDesc& defaultPageDescription,
        bool tabPositionIsRelativeToParagraphIndent,
        TabStopReferencePolicy referencePolicy)
: mIndexOfSectionNode(indexOfSectionNode),
  mDefaultPageDescription(defaultPageDescription),
  mTabPositionIsRelativeToParagraphIndent(tabPositionIsRelativeToParagraphIndent),
  mTabStopReferencePolicy(referencePolicy)
{
}


ToxTabStopTokenHandler::HandledTabStopToken
DefaultToxTabStopTokenHandler::HandleTabStopToken(
        const SwFormToken& aToken, const SwTextNode& targetNode) const
{
    HandledTabStopToken result;

    if (aToken.bWithTab) { // #i21237#
        result.text = "\t";
    }

    // check whether a tab adjustment has been specified.
    if (SvxTabAdjust::End > aToken.eTabAlign) {
        SvxTextLeftMarginItem const& rTextLeftMargin(
            targetNode.SwContentNode::GetAttr(RES_MARGIN_TEXTLEFT));
        tools::Long nTabPosition = aToken.nTabStopPosition;
        if (!mTabPositionIsRelativeToParagraphIndent && rTextLeftMargin.GetTextLeft() != 0)
        {
            nTabPosition -= rTextLeftMargin.GetTextLeft();
        }
        result.tabStop = SvxTabStop(nTabPosition, aToken.eTabAlign, cDfltDecimalChar, aToken.cTabFillChar);
        return result;
    }

    // note: this will be filled later by CalcEndStop()
    result.tabStop = SvxTabStop(0, SvxTabAdjust::Right, cDfltDecimalChar, aToken.cTabFillChar);
    return result;
}

auto DefaultToxTabStopTokenHandler::CalcEndStop(SwTextNode const& rNode,
        SwRootFrame const*const pLayout) const -> tools::Long
{
    tools::Long nRightMargin;
    if (CanUseLayoutRectangle(rNode, pLayout))
    {
        // in case it's in a header, any frame should do
        SwContentFrame const*const pFrame(rNode.getLayoutFrame(pLayout));
        assert(pFrame); // created in SwTOXBaseSection::Update()
        SwRectFnSet const fnRect(pFrame->GetUpper());
        SwRect rect = pFrame->getFramePrintArea();
        if (fnRect.GetWidth(rect) == 0) // typically it's newly created
        {
            if (pFrame->GetUpper()->IsSctFrame())
            {   // this is set in SwSectionFrame::Init()
                rect = pFrame->GetUpper()->GetUpper()->getFramePrintArea();
                assert(fnRect.GetWidth(rect) != 0);
            }
            else if (pFrame->GetUpper()->IsColBodyFrame())
            {
                SwFrame const*const pColFrame(pFrame->GetUpper()->GetUpper());
                assert(pColFrame->IsColumnFrame());
                rect = pColFrame->getFrameArea();
                // getFramePrintArea() is not valid yet, manually subtract...
                // (it can have a border too!)
                SwBorderAttrAccess access(SwFrame::GetCache(), pColFrame);
                SwBorderAttrs const& rAttrs(*access.Get());
                auto const nLeft(rAttrs.CalcLeft(pColFrame));
                auto const nRight(rAttrs.CalcRight(pColFrame));
                fnRect.SetWidth(rect, fnRect.GetWidth(rect) - nLeft - nRight);
                assert(fnRect.GetWidth(rect) != 0);
            }
            else assert(false);
        }
        nRightMargin = fnRect.GetWidth(rect);
    }
    else
    {
        nRightMargin = CalculatePageMarginFromPageDescription(rNode);
    }
    //#i24363# tab stops relative to indent
    if (mTabStopReferencePolicy == TABSTOPS_RELATIVE_TO_INDENT) {
        // left margin of paragraph style
        SvxFirstLineIndentItem const& rFirstLine(
            rNode.GetTextColl()->GetFirstLineIndent());
        SvxTextLeftMarginItem const& rTextLeftMargin(
            rNode.GetTextColl()->GetTextLeftMargin());
        nRightMargin -= rTextLeftMargin.GetLeft(rFirstLine);
        nRightMargin -= rFirstLine.GetTextFirstLineOffset();
    }
    return nRightMargin - 1; // subtract 1 twip to avoid equal for TabOverMargin
}

tools::Long
DefaultToxTabStopTokenHandler::CalculatePageMarginFromPageDescription(const SwTextNode& targetNode) const
{
    SwNodeOffset nPgDescNdIdx = targetNode.GetIndex() + 1;
    const SwPageDesc *pPageDesc = targetNode.FindPageDesc(&nPgDescNdIdx);
    if (!pPageDesc || nPgDescNdIdx < mIndexOfSectionNode) {
        // Use default page description, if none is found or the found one is given by a Node before the
        // table-of-content section.
        pPageDesc = &mDefaultPageDescription;
    }
    const SwFrameFormat& rPgDscFormat = pPageDesc->GetMaster();
    tools::Long result = rPgDscFormat.GetFrameSize().GetWidth() - rPgDscFormat.GetLRSpace().GetLeft()
            - rPgDscFormat.GetLRSpace().GetRight();
    // Also consider borders
    const SvxBoxItem& rBox = rPgDscFormat.GetBox();
    result -= rBox.CalcLineSpace(SvxBoxItemLine::LEFT) + rBox.CalcLineSpace(SvxBoxItemLine::RIGHT);
    return result;
}


/*static*/ bool
DefaultToxTabStopTokenHandler::CanUseLayoutRectangle(const SwTextNode& targetNode, const SwRootFrame *currentLayout)
{
    const SwPageDesc* pageDescription =
            targetNode.SwContentNode::GetAttr(RES_PAGEDESC).GetPageDesc();

    if (!pageDescription) {
        return true;
    }
    const SwFrame* pFrame = targetNode.getLayoutFrame(currentLayout);
    if (!pFrame) {
        return false;
    }
    pFrame = pFrame->FindPageFrame();
    if (!pFrame) {
        return false;
    }
    const SwPageFrame* pageFrame = static_cast<const SwPageFrame*>(pFrame);
    return pageDescription == pageFrame->GetPageDesc();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

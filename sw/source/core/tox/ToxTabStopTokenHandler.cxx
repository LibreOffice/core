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

#include <cntfrm.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <frmfmt.hxx>
#include <ndtxt.hxx>
#include <pagedesc.hxx>
#include <pagefrm.hxx>
#include <swrect.hxx>
#include <tox.hxx>

namespace sw {

DefaultToxTabStopTokenHandler::DefaultToxTabStopTokenHandler(sal_uInt32 indexOfSectionNode,
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
        const SwFormToken& aToken, const SwTextNode& targetNode, const SwRootFrame *currentLayout) const
{
    HandledTabStopToken result;

    if (aToken.bWithTab) { // #i21237#
        result.text = "\t";
    }

    // check whether a tab adjustment has been specified.
    if (SvxTabAdjust::End > aToken.eTabAlign) {
        const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>( targetNode.SwContentNode::GetAttr(RES_LR_SPACE) );

        long nTabPosition = aToken.nTabStopPosition;
        if (!mTabPositionIsRelativeToParagraphIndent && rLR.GetTextLeft()) {
            nTabPosition -= rLR.GetTextLeft();
        }
        result.tabStop = SvxTabStop(nTabPosition, aToken.eTabAlign, cDfltDecimalChar, aToken.cTabFillChar);
        return result;
    }

    SwRect aNdRect;
    if (CanUseLayoutRectangle(targetNode, currentLayout)) {
        aNdRect = targetNode.FindLayoutRect(true);
    }
    long nRightMargin;
    if (aNdRect.IsEmpty()) {
        nRightMargin = CalculatePageMarginFromPageDescription(targetNode);
    } else {
        nRightMargin = aNdRect.Width();
    }
    //#i24363# tab stops relative to indent
    if (mTabStopReferencePolicy == TABSTOPS_RELATIVE_TO_INDENT) {
        // left margin of paragraph style
        const SvxLRSpaceItem& rLRSpace = targetNode.GetTextColl()->GetLRSpace();
        nRightMargin -= rLRSpace.GetLeft();
        nRightMargin -= rLRSpace.GetTextFirstLineOfst();
    }

    result.tabStop = SvxTabStop(nRightMargin, SvxTabAdjust::Right, cDfltDecimalChar, aToken.cTabFillChar);
    return result;
}

long
DefaultToxTabStopTokenHandler::CalculatePageMarginFromPageDescription(const SwTextNode& targetNode) const
{
    size_t nPgDescNdIdx = targetNode.GetIndex() + 1;
    const SwPageDesc *pPageDesc = targetNode.FindPageDesc(&nPgDescNdIdx);
    if (!pPageDesc || nPgDescNdIdx < mIndexOfSectionNode) {
        // Use default page description, if none is found or the found one is given by a Node before the
        // table-of-content section.
        pPageDesc = &mDefaultPageDescription;
    }
    const SwFrameFormat& rPgDscFormat = pPageDesc->GetMaster();
    long result = rPgDscFormat.GetFrameSize().GetWidth() - rPgDscFormat.GetLRSpace().GetLeft()
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
            static_cast<const SwFormatPageDesc&>( targetNode.SwContentNode::GetAttr(RES_PAGEDESC)).GetPageDesc();

    if (!pageDescription) {
        return false;
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

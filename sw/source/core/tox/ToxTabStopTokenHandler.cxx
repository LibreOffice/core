/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ToxTabStopTokenHandler.hxx"

#include "editeng/tstpitem.hxx"
#include "editeng/lrspitem.hxx"

#include "cntfrm.hxx"
#include "fmtfsize.hxx"
#include "fmtpdsc.hxx"
#include "frmfmt.hxx"
#include "ndtxt.hxx"
#include "pagedesc.hxx"
#include "pagefrm.hxx"
#include "swrect.hxx"
#include "tox.hxx"

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
        const SwFormToken& aToken, const SwTxtNode& targetNode, const SwRootFrm *currentLayout) const
{
    HandledTabStopToken result;

    if (aToken.bWithTab) { // #i21237#
        result.text = "\t";
    }

    // check whether a tab adjustment has been specified.
    if (SVX_TAB_ADJUST_END > aToken.eTabAlign) {
        const SvxLRSpaceItem& rLR = (SvxLRSpaceItem&) targetNode.SwCntntNode::GetAttr(RES_LR_SPACE, true);

        long nTabPosition = aToken.nTabStopPosition;
        if (!mTabPositionIsRelativeToParagraphIndent && rLR.GetTxtLeft()) {
            nTabPosition -= rLR.GetTxtLeft();
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
        const SvxLRSpaceItem& rLRSpace = targetNode.GetTxtColl()->GetLRSpace();
        nRightMargin -= rLRSpace.GetLeft();
        nRightMargin -= rLRSpace.GetTxtFirstLineOfst();
    }

    result.tabStop = SvxTabStop(nRightMargin, SVX_TAB_ADJUST_RIGHT, cDfltDecimalChar, aToken.cTabFillChar);
    return result;
}

long
DefaultToxTabStopTokenHandler::CalculatePageMarginFromPageDescription(const SwTxtNode& targetNode) const
{
    sal_uInt32 nPgDescNdIdx = targetNode.GetIndex() + 1;
    const SwPageDesc *pPageDesc = targetNode.FindPageDesc(false, &nPgDescNdIdx);
    if (!pPageDesc || nPgDescNdIdx < mIndexOfSectionNode) {
        // Use default page description, if none is found or the found one is given by a Node before the
        // table-of-content section.
        pPageDesc = &mDefaultPageDescription;
    }
    const SwFrmFmt& rPgDscFmt = pPageDesc->GetMaster();
    long result = rPgDscFmt.GetFrmSize().GetWidth() - rPgDscFmt.GetLRSpace().GetLeft()
            - rPgDscFmt.GetLRSpace().GetRight();
    return result;
}


/*static*/ bool
DefaultToxTabStopTokenHandler::CanUseLayoutRectangle(const SwTxtNode& targetNode, const SwRootFrm *currentLayout)
{
    const SwPageDesc* pageDescription =
            ((SwFmtPageDesc&) targetNode.SwCntntNode::GetAttr(RES_PAGEDESC)).GetPageDesc();

    if (!pageDescription) {
        return false;
    }
    const SwFrm* pFrm = targetNode.getLayoutFrm(currentLayout, 0, 0, true);
    if (!pFrm) {
        return false;
    }
    pFrm = pFrm->FindPageFrm();
    if (!pFrm) {
        return false;
    }
    const SwPageFrm* pageFrm = static_cast<const SwPageFrm*>(pFrm);
    if (pageDescription != pageFrm->GetPageDesc()) {
        return false;
    }
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

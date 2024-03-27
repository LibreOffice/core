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

#include <svgswitchnode.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <unotools/syslocaleoptions.hxx>

namespace svgio::svgreader
{
SvgSwitchNode::SvgSwitchNode(SvgDocument& rDocument, SvgNode* pParent)
    : SvgNode(SVGToken::Switch, rDocument, pParent)
    , maSvgStyleAttributes(*this)
{
}

SvgSwitchNode::~SvgSwitchNode() {}

const SvgStyleAttributes* SvgSwitchNode::getSvgStyleAttributes() const
{
    return checkForCssStyle(maSvgStyleAttributes);
}

void SvgSwitchNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
{
    // call parent
    SvgNode::parseAttribute(aSVGToken, aContent);

    // read style attributes
    maSvgStyleAttributes.parseStyleAttribute(aSVGToken, aContent);

    // parse own
    switch (aSVGToken)
    {
        case SVGToken::Style:
        {
            readLocalCssStyle(aContent);
            break;
        }
        case SVGToken::Transform:
        {
            const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

            if (!aMatrix.isIdentity())
            {
                setTransform(aMatrix);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void SvgSwitchNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                                     bool bReferenced) const
{
    // #i125258# for SVGTokenG decompose children
    const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

    if (pStyle)
    {
        drawinglayer::primitive2d::Primitive2DContainer aContent;

        const auto& rChildren = getChildren();
        const sal_uInt32 nCount(rChildren.size());
        OUString sLanguage(SvtSysLocaleOptions().GetRealUILanguageTag().getLanguage());

        SvgNode* pNodeToDecompose = nullptr;
        for (sal_uInt32 a(0); a < nCount; a++)
        {
            SvgNode* pCandidate = rChildren[a].get();

            if (pCandidate && Display::None != pCandidate->getDisplay())
            {
                std::vector<OUString> aSystemLanguage = pCandidate->getSystemLanguage();
                if (!sLanguage.isEmpty() && !aSystemLanguage.empty())
                {
                    for (const OUString& sSystemLang : aSystemLanguage)
                    {
                        if (sSystemLang == sLanguage)
                        {
                            pNodeToDecompose = pCandidate;
                            break;
                        }
                    }
                }
                else
                {
                    pNodeToDecompose = pCandidate;
                }
            }

            if (pNodeToDecompose)
            {
                pNodeToDecompose->decomposeSvgNode(aContent, bReferenced);
                // break once it's descomposed
                break;
            }
        }

        if (!aContent.empty())
        {
            pStyle->add_postProcess(rTarget, std::move(aContent), getTransform());
        }
    }
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

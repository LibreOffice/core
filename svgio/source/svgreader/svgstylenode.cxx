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

#include <svgio/svgreader/svgstylenode.hxx>
#include <svgio/svgreader/svgdocument.hxx>



namespace svgio
{
    namespace svgreader
    {
        SvgStyleNode::SvgStyleNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenStyle, rDocument, pParent),
            maSvgStyleAttributes(),
            mbTextCss(false)
        {
        }

        SvgStyleNode::~SvgStyleNode()
        {
            while(!maSvgStyleAttributes.empty())
            {
                delete *(maSvgStyleAttributes.end() - 1);
                maSvgStyleAttributes.pop_back();
            }
        }

        void SvgStyleNode::parseAttribute(const OUString& rTokenName, SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGTokenType:
                {
                    if(!aContent.isEmpty())
                    {
                        if(aContent.startsWith("text/css"))
                        {
                            setTextCss(true);
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgStyleNode::addCssStyleSheet(const OUString& aContent)
        {
            const sal_Int32 nLen(aContent.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                OUStringBuffer aTokenValue;

                while(nPos < nLen)
                {
                    const sal_Int32 nInitPos(nPos);
                    skip_char(aContent, ' ', '#', nPos, nLen);
                    copyToLimiter(aContent, '{', nPos, aTokenValue, nLen);
                    const OUString aStyleName = aTokenValue.makeStringAndClear().trim();

                    if(!aStyleName.isEmpty() && nPos < nLen)
                    {
                        skip_char(aContent, ' ', '{', nPos, nLen);
                        copyToLimiter(aContent, '}', nPos, aTokenValue, nLen);
                        skip_char(aContent, ' ', '}', nPos, nLen);
                        const OUString aStyleContent = aTokenValue.makeStringAndClear().trim();

                        if(!aStyleContent.isEmpty())
                        {
                            // create new style
                            SvgStyleAttributes* pNewStyle = new SvgStyleAttributes(*this);
                            maSvgStyleAttributes.push_back(pNewStyle);

                            // fill with content
                            pNewStyle->readStyle(aStyleContent);

                            // register new style at document
                            const_cast< SvgDocument& >(getDocument()).addSvgStyleAttributesToMapper(aStyleName, *pNewStyle);
                        }
                    }

                    if(nInitPos == nPos)
                    {
                        OSL_ENSURE(false, "Could not interpret on current position (!)");
                        nPos++;
                    }
                }
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio


// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

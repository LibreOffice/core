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

#include <svgstylenode.hxx>
#include <svgdocument.hxx>

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

        // #i125258# no parent when we are a CssStyle holder to break potential loops because
        // when using CssStyles we jump uncontrolled inside the node tree hierarchy
        bool SvgStyleNode::supportsParentStyle() const
        {
            if(isTextCss())
            {
                return false;
            }

            // call parent
            return SvgNode::supportsParentStyle();
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

        void SvgStyleNode::addCssStyleSheet(const OUString& aSelectors, const SvgStyleAttributes& rNewStyle)
        {
            // aSelectors: CssStyle selectors, any combination, no comma separations, no spaces at start/end
            // rNewStyle: the already prepared style to register on that name
            if(aSelectors.isEmpty())
                return;

            std::vector< OUString > aSelectorParts;
            const sal_Int32 nLen(aSelectors.getLength());
            sal_Int32 nPos(0);
            OUStringBuffer aToken;

            // split into single tokens (currently only space separator)
            while(nPos < nLen)
            {
                const sal_Int32 nInitPos(nPos);
                copyToLimiter(aSelectors, u' ', nPos, aToken, nLen);
                skip_char(aSelectors, u' ', nPos, nLen);
                const OUString aSelectorPart(aToken.makeStringAndClear().trim());

                if(!aSelectorPart.isEmpty())
                {
                    aSelectorParts.push_back(aSelectorPart);
                }

                if(nInitPos == nPos)
                {
                    OSL_ENSURE(false, "Could not interpret on current position (!)");
                    nPos++;
                }
            }

            if(aSelectorParts.empty())
                return;

            OUStringBuffer aConcatenatedSelector;

            // re-combine without spaces, create a unique name (for now)
            for(size_t a(0); a < aSelectorParts.size(); a++)
            {
                aConcatenatedSelector.append(aSelectorParts[a]);
            }

            // CssStyles in SVG are currently not completely supported; the current idea for
            // supporting the needed minimal set is to register CssStyles associated to a string
            // which is just the space-char cleaned, concatenated Selectors. The part to 'match'
            // these is in fillCssStyleVectorUsingHierarchyAndSelectors. There, the same string is
            // built up using the priorities of local CssStyle, Id, Class and other info combined
            // with the existing hierarchy. This creates a specificity and priority-sorted local
            // list for each node which is then chained using get/setCssStyleParent.
            // The current solution is capable of solving space-separated selectors which can be
            // mixed between Id, Class and type specifiers.
            // When CssStyles need more specific solving, the start point is here; remember the
            // needed infos not in maIdStyleTokenMapperList at the document, but select evtl.
            // more specific infos there in a class capable of handling more complex matchings.
            // Additionally fillCssStyleVector (or the mechanism above that when a linked list of
            // SvgStyleAttributes will not do it) will have to be adapted to make use of it.

            // register new style at document for (evtl. concatenated) stylename
            const_cast< SvgDocument& >(getDocument()).addSvgStyleAttributesToMapper(aConcatenatedSelector.makeStringAndClear(), rNewStyle);
        }

        void SvgStyleNode::addCssStyleSheet(const OUString& aSelectors, const OUString& aContent)
        {
            // aSelectors: possible comma-separated list of CssStyle definitions, no spaces at start/end
            // aContent: the svg style definitions as string
            if(aSelectors.isEmpty() || aContent.isEmpty())
                return;

            // create new style and add to local list (for ownership control)
            SvgStyleAttributes* pNewStyle = new SvgStyleAttributes(*this);
            maSvgStyleAttributes.push_back(pNewStyle);

            // fill with content
            pNewStyle->readCssStyle(aContent);

            // comma-separated split (Css abbreviation for same style for multiple selectors)
            const sal_Int32 nLen(aSelectors.getLength());
            sal_Int32 nPos(0);
            OUStringBuffer aToken;

            while(nPos < nLen)
            {
                const sal_Int32 nInitPos(nPos);
                copyToLimiter(aSelectors, u',', nPos, aToken, nLen);
                skip_char(aSelectors, u' ', u',', nPos, nLen);

                const OUString aSingleName(aToken.makeStringAndClear().trim());

                if(aSingleName.getLength())
                {
                    addCssStyleSheet(aSingleName, *pNewStyle);
                }

                if(nInitPos == nPos)
                {
                    OSL_ENSURE(false, "Could not interpret on current position (!)");
                    nPos++;
                }
            }
        }

        void SvgStyleNode::addCssStyleSheet(const OUString& aSelectorsAndContent)
        {
            const sal_Int32 nLen(aSelectorsAndContent.getLength());

            if(!nLen)
                return;

            sal_Int32 nPos(0);
            OUStringBuffer aToken;

            while(nPos < nLen)
            {
                // read the full selectors (may be multiple, comma-separated)
                const sal_Int32 nInitPos(nPos);
                skip_char(aSelectorsAndContent, u' ', nPos, nLen);
                copyToLimiter(aSelectorsAndContent, u'{', nPos, aToken, nLen);
                skip_char(aSelectorsAndContent, u' ', u'{', nPos, nLen);

                const OUString aSelectors(aToken.makeStringAndClear().trim());
                OUString aContent;

                if(!aSelectors.isEmpty() && nPos < nLen)
                {
                    // isolate content as text, embraced by '{' and '}'
                    copyToLimiter(aSelectorsAndContent, u'}', nPos, aToken, nLen);
                    skip_char(aSelectorsAndContent, u' ', u'}', nPos, nLen);

                    aContent = aToken.makeStringAndClear().trim();
                }

                if(!aSelectors.isEmpty() && !aContent.isEmpty())
                {
                    addCssStyleSheet(aSelectors, aContent);
                }

                if(nInitPos == nPos)
                {
                    OSL_ENSURE(false, "Could not interpret on current position (!)");
                    nPos++;
                }
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

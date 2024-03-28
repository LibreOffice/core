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
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>

namespace svgio::svgreader
{
        SvgStyleNode::SvgStyleNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGToken::Style, rDocument, pParent),
            mbTextCss(true)
        {
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

        void SvgStyleNode::parseAttribute(SVGToken aSVGToken, const OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGToken::Type:
                {
                    if(!aContent.isEmpty())
                    {
                        if(!o3tl::equalsIgnoreAsciiCase(o3tl::trim(aContent), u"text/css"))
                        {
                            setTextCss(false);
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

        void SvgStyleNode::addCssStyleSheet(std::u16string_view aSelectors, const SvgStyleAttributes& rNewStyle)
        {
            // aSelectors: CssStyle selectors, any combination, no comma separations, no spaces at start/end
            // rNewStyle: the already prepared style to register on that name
            SvgStringVector aSelectorParts;

            if(!readSvgStringVector(aSelectors, aSelectorParts, ' '))
                return;

            OUStringBuffer aConcatenatedSelector;

            // re-combine without spaces, create a unique name (for now)
            for(const auto &a : aSelectorParts)
            {
                aConcatenatedSelector.append(a);
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

        void SvgStyleNode::addCssStyleSheet(std::u16string_view aSelectors, std::u16string_view aContent)
        {
            // aSelectors: possible comma-separated list of CssStyle definitions, no spaces at start/end
            // aContent: the svg style definitions as string
            if(aSelectors.empty() || aContent.empty())
                return;

            // comma-separated split (Css abbreviation for same style for multiple selectors)
            const sal_Int32 nLen(aSelectors.size());
            sal_Int32 nPos(0);
            OUStringBuffer aToken;

            while(nPos < nLen)
            {
                const sal_Int32 nInitPos(nPos);
                copyToLimiter(aSelectors, u',', nPos, aToken, nLen);
                skip_char(aSelectors, u' ', u',', nPos, nLen);

                const OUString aSingleName(o3tl::trim(aToken));
                aToken.setLength(0);

                // add the current css class only if wasn't previously added
                auto [aIterator, bIsNew] = maSvgStyleAttributes.try_emplace(aSingleName);
                if (bIsNew)
                {
                    // create new style and add to local list (for ownership control) and
                    // in case it's written to again in future classes to prevent overwrites
                    aIterator->second = std::make_unique<SvgStyleAttributes>(*this);
                }
                const std::unique_ptr<SvgStyleAttributes>& pCurrentStyle = aIterator->second;

                // fill with content
                pCurrentStyle->readCssStyle(aContent);

                if(aSingleName.getLength())
                {
                    addCssStyleSheet(aSingleName, *pCurrentStyle);
                }

                if(nInitPos == nPos)
                {
                    OSL_ENSURE(false, "Could not interpret on current position (!)");
                    nPos++;
                }
            }
        }

        void SvgStyleNode::addCssStyleSheet(std::u16string_view aSelectorsAndContent)
        {
            const sal_Int32 nLen(aSelectorsAndContent.size());

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

                const OUString aSelectors(o3tl::trim(aToken));
                aToken.setLength(0);
                OUString aContent;

                if(!aSelectors.isEmpty() && nPos < nLen)
                {
                    // isolate content as text, embraced by '{' and '}'
                    copyToLimiter(aSelectorsAndContent, u'}', nPos, aToken, nLen);
                    skip_char(aSelectorsAndContent, u' ', u'}', nPos, nLen);

                    aContent = o3tl::trim(aToken);
                    aToken.setLength(0);
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

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

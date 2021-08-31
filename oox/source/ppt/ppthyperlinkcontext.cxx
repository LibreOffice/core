/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/ppt/ppthyperlinkcontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>

using namespace ::oox::core;

namespace oox::ppt
{
PPTHyperLinkContext::PPTHyperLinkContext(ContextHandler2Helper const& rParent, const AttributeList&,
                                         PropertyMap& aProperties)
    : ContextHandler2(rParent)
    , maProperties(aProperties)
{
}

PPTHyperLinkContext::~PPTHyperLinkContext() {}

ContextHandlerRef PPTHyperLinkContext::onCreateContext(::sal_Int32 aElement,
                                                       const AttributeList& rAttribs)
{
    switch (aElement)
    {
        case A_TOKEN(hlinkClick):
            OUString sURL, sHref;
            OUString aRelId = rAttribs.getString(R_TOKEN(id)).get();
            if (!aRelId.isEmpty())
            {
                sHref = getRelations().getExternalTargetFromRelId(aRelId);
                if (!sHref.isEmpty())
                    sURL = getFilter().getAbsoluteUrl(sHref);
                else
                    sURL = getRelations().getInternalTargetFromRelId(aRelId);
            }
            OUString sAction = rAttribs.getString(XML_action).get();
            if (!sAction.isEmpty())
            {
                static const OUStringLiteral sPPAction(u"ppaction://");
                if (sAction.matchIgnoreAsciiCase(sPPAction))
                {
                    OUString aPPAct(sAction.copy(sPPAction.getLength()));
                    sal_Int32 nIndex = aPPAct.indexOf('?');
                    OUString aPPAction(nIndex > 0 ? aPPAct.copy(0, nIndex) : aPPAct);

                    if (aPPAction.match("hlinkshowjump"))
                    {
                        static const OUStringLiteral sShowJumpAction(
                            u"ppaction://hlinkshowjump?jump=");
                        sURL = sAction.copy(sShowJumpAction.getLength());
                    }
                    else if (aPPAction.match("hlinksldjump"))
                    {
                        static const OUStringLiteral sHlinkSlide(u"slide");
                        if (sURL.match(sHlinkSlide))
                        {
                            sal_Int32 nLength = 1;
                            sal_Int32 nIndex = sHlinkSlide.getLength();
                            while ((nIndex + nLength) < sURL.getLength())
                            {
                                sal_Unicode nChar = sURL[nIndex + nLength];
                                if ((nChar < '0') || (nChar > '9'))
                                    break;
                                nLength++;
                            }
                            sal_Int32 nPageNumber = sURL.copy(nIndex, nLength).toInt32();
                            if (nPageNumber)
                                sURL = "#Slide " + OUString::number(nPageNumber);
                        }
                    }
                }
            }
            if (!sURL.isEmpty())
                maProperties.setProperty(PROP_URL, sURL);
            break;
    }
    return this;
}

} // namespace oox::ppt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

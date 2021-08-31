/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_PPT_PPTHYPERLINKCONTEXT_HXX
#define INCLUDED_OOX_PPT_PPTHYPERLINKCONTEXT_HXX

#include <oox/core/contexthandler2.hxx>
#include <oox/helper/propertymap.hxx>
#include <sal/types.h>

namespace oox
{
class AttributeList;
}
namespace oox::ppt
{
class PPTHyperLinkContext final : public ::oox::core::ContextHandler2
{
public:
    PPTHyperLinkContext(::oox::core::ContextHandler2Helper const& rParent,
                        const AttributeList& rAttribs, PropertyMap& aProperties);
    virtual ~PPTHyperLinkContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext(sal_Int32 Element,
                                                           const AttributeList& rAttribs) override;

private:
    PropertyMap& maProperties;
};
} // namespace oox::ppt

#endif // INCLUDED_OOX_PPT_PPTHYPERLINKCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

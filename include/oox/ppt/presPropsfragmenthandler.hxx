/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OOX_PPT_PRESPROPSFRAGMENTHANDLER_HXX
#define INCLUDED_OOX_PPT_PRESPROPSFRAGMENTHANDLER_HXX

#include <oox/core/fragmenthandler2.hxx>

namespace oox
{
namespace core
{
class XmlFilterBase;
}

namespace ppt
{
class PresPropsFragmentHandler final : public core::FragmentHandler2
{
public:
    PresPropsFragmentHandler(core::XmlFilterBase& rFilter, const OUString& rFragmentPath);
    virtual ~PresPropsFragmentHandler() override;
    virtual void finalizeImport() override;
    virtual core::ContextHandlerRef onCreateContext(sal_Int32 aElementToken,
                                                    const AttributeList& rAttribs) override;

private:
    bool m_bLoop = false;
};
} // namespace ppt
} // namespace oox

#endif // INCLUDED_OOX_PPT_PRESPROPSFRAGMENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

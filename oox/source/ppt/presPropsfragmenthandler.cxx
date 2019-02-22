/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>

#include <oox/helper/attributelist.hxx>
#include <oox/ppt/pptimport.hxx>
#include <oox/ppt/presPropsfragmenthandler.hxx>
#include <oox/token/namespaces.hxx>

namespace oox
{
namespace ppt
{
PresPropsFragmentHandler::PresPropsFragmentHandler(core::XmlFilterBase& rFilter,
                                                   const OUString& rFragmentPath)
    : FragmentHandler2(rFilter, rFragmentPath)
{
}

PresPropsFragmentHandler::~PresPropsFragmentHandler() = default;

void PresPropsFragmentHandler::finalizeImport()
{
    css::uno::Reference<css::presentation::XPresentationSupplier> xPresentationSupplier(
        getFilter().getModel(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::beans::XPropertySet> xPresentationProps(
        xPresentationSupplier->getPresentation(), css::uno::UNO_QUERY_THROW);
    xPresentationProps->setPropertyValue("IsEndless", css::uno::Any(m_bLoop));
}

core::ContextHandlerRef PresPropsFragmentHandler::onCreateContext(sal_Int32 aElementToken,
                                                                  const AttributeList& rAttribs)
{
    switch (aElementToken)
    {
        case PPT_TOKEN(presentationPr):
            return this;
        case PPT_TOKEN(showPr):
            m_bLoop = rAttribs.getBool(XML_loop, false);
            return this;
    }
    return this;
}
} // namespace ppt
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

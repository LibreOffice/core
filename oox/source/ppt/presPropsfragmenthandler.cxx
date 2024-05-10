/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/ppt/presPropsfragmenthandler.hxx>
#include <oox/token/namespaces.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace oox::ppt
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
    xPresentationProps->setPropertyValue(u"IsEndless"_ustr, css::uno::Any(m_bLoop));
    xPresentationProps->setPropertyValue(u"IsAutomatic"_ustr, css::uno::Any(!m_bTiming));

    if (!m_sId.isEmpty())
    {
        css::uno::Reference<css::presentation::XCustomPresentationSupplier>
            XCustPresentationSupplier(getFilter().getModel(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::container::XNameContainer> mxCustShows;
        mxCustShows = XCustPresentationSupplier->getCustomPresentations();
        const css::uno::Sequence<OUString> aNameSeq(mxCustShows->getElementNames());
        xPresentationProps->setPropertyValue(u"CustomShow"_ustr,
                                             css::uno::Any(aNameSeq[m_sId.toInt32()]));
    }

    if (!m_sSt.isEmpty())
    {
        Reference<drawing::XDrawPagesSupplier> xDPS(getFilter().getModel(), uno::UNO_QUERY_THROW);
        Reference<drawing::XDrawPages> xDrawPages(xDPS->getDrawPages(), uno::UNO_SET_THROW);
        Reference<drawing::XDrawPage> xDrawPage;
        xDrawPages->getByIndex(m_sSt.toInt32() - 1) >>= xDrawPage;
        Reference<container::XNamed> xNamed(xDrawPage, uno::UNO_QUERY_THROW);
        xPresentationProps->setPropertyValue(u"FirstPage"_ustr, uno::Any(xNamed->getName()));
    }
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
            m_bTiming = rAttribs.getBool(XML_useTimings, true);
            return this;
        case PPT_TOKEN(custShow):
            m_sId = rAttribs.getStringDefaulted(XML_id);
            return this;
        case PPT_TOKEN(sldRg):
            m_sSt = rAttribs.getStringDefaulted(XML_st);
            return this;
    }
    return this;
}
} // namespace oox::ppt

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

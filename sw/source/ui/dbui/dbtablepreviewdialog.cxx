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

#include "dbtablepreviewdialog.hxx"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

class DBTablePreviewFrame
    : public cppu::WeakImplHelper<lang::XEventListener>
{
private:
    css::uno::Reference<css::frame::XFrame2> m_xFrame;

    virtual void SAL_CALL disposing(const lang::EventObject& /*Source*/) override
    {
        m_xFrame.clear();
    }

public:
    DBTablePreviewFrame(css::uno::Reference<css::frame::XFrame2> xFrame)
        : m_xFrame(std::move(xFrame))
    {
    }

    void cleanup()
    {
        if (m_xFrame.is())
        {
            m_xFrame->setComponent(nullptr, nullptr);
            m_xFrame->dispose();
            m_xFrame.clear();
        }
    }
};

SwDBTablePreviewDialog::SwDBTablePreviewDialog(weld::Window* pParent, uno::Sequence< beans::PropertyValue> const & rValues)
    : SfxDialogController(pParent, u"modules/swriter/ui/tablepreviewdialog.ui"_ustr, u"TablePreviewDialog"_ustr)
    , m_xDescriptionFI(m_xBuilder->weld_label(u"description"_ustr))
    , m_xBeamerWIN(m_xBuilder->weld_container(u"beamer"_ustr))
{
    Size aSize(m_xBeamerWIN->get_approximate_digit_width() * 80,
               m_xBeamerWIN->get_text_height() * 18);
    m_xBeamerWIN->set_size_request(aSize.Width(), aSize.Height());

    auto pValue = std::find_if(rValues.begin(), rValues.end(),
        [](const beans::PropertyValue& rValue) { return rValue.Name == "Command"; });
    if (pValue != rValues.end())
    {
        OUString sDescription = m_xDescriptionFI->get_label();
        OUString sTemp;
        pValue->Value >>= sTemp;
        m_xDescriptionFI->set_label(sDescription.replaceFirst("%1", sTemp));
    }

    css::uno::Reference<css::frame::XFrame2> xFrame;
    try
    {
        // create a frame wrapper for myself
        xFrame = frame::Frame::create( comphelper::getProcessComponentContext() );
        xFrame->initialize(m_xBeamerWIN->CreateChildFrame());
    }
    catch (uno::Exception const &)
    {
        xFrame.clear();
    }
    if (!xFrame.is())
        return;

    m_xFrameListener.set(new DBTablePreviewFrame(xFrame));
    xFrame->addEventListener(m_xFrameListener);

    util::URL aURL;
    aURL.Complete = ".component:DB/DataSourceBrowser";
    uno::Reference<frame::XDispatch> xD = xFrame->queryDispatch(aURL, u""_ustr,
            css::frame::FrameSearchFlag::CHILDREN | css::frame::FrameSearchFlag::CREATE);
    if (xD.is())
    {
        xD->dispatch(aURL, rValues);
        m_xBeamerWIN->show();
    }
}

SwDBTablePreviewDialog::~SwDBTablePreviewDialog()
{
    if (m_xFrameListener)
    {
        m_xFrameListener->cleanup();
        m_xFrameListener.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

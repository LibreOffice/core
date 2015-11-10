/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_BUTTONSTATUSLISTENER_HXX
#define INCLUDED_VCL_BUTTONSTATUSLISTENER_HXX

#include <cppuhelper/implbase.hxx>
#include <vcl/button.hxx>

#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>


class VCL_DLLPUBLIC ButtonStatusListener : public cppu::WeakImplHelper < css::frame::XStatusListener>
{
public:
    ButtonStatusListener(Button* button, const rtl::OUString& aCommand);

private:
    VclPtr<Button> mButton; /** The button on which actions are performed */

    /** Dispatcher. Need to keep a reference to it as long as this StatusListener exists. */
    css::uno::Reference<css::frame::XDispatch> mxDispatch;
    css::util::URL maCommandURL;

public:
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent)
            throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& /*Source*/)
            throw( css::uno::RuntimeException, std::exception ) override;

    void dispose();
};

#endif // INCLUDED_VCL_BUTTONSTATUSLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
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

#include <com/sun/star/frame/XStatusListener.hpp>

class VCL_DLLPUBLIC ButtonStatusListener : public cppu::WeakImplHelper < css::frame::XStatusListener>
{
    public:
        ButtonStatusListener(Button* button);
        virtual ~ButtonStatusListener();

        VclPtr<Button> mButton; /** The button on which actions are performed */


    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent)
            throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& /*Source*/)
            throw( css::uno::RuntimeException, std::exception ) override {};
};

#endif // INCLUDED_VCL_BUTTONSTATUSLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
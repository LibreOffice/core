/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_VCLSTATUSLISTENER_HXX
#define INCLUDED_VCL_VCLSTATUSLISTENER_HXX

#include <cppuhelper/implbase.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/vclptr.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

template <class T> class VclStatusListener : public cppu::WeakImplHelper < css::frame::XStatusListener>
{
public:
    VclStatusListener<T>(T* widget, const OUString& aCommand);

private:
    VclPtr<T> mWidget; /** The widget on which actions are performed */

    /** Dispatcher. Need to keep a reference to it as long as this StatusListener exists. */
    css::uno::Reference<css::frame::XDispatch> mxDispatch;
    css::util::URL maCommandURL;
    css::uno::Reference<css::frame::XFrame> mxFrame;

public:
    void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    void SAL_CALL disposing(const css::lang::EventObject& /*Source*/) override;

    const css::uno::Reference<css::frame::XFrame>& getFrame() { return mxFrame; }

    void startListening();

    void dispose();
};

template<class T>
VclStatusListener<T>::VclStatusListener(T* widget, const OUString& aCommand) {
    mWidget = widget;

    css::uno::Reference<css::uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference<css::frame::XDesktop2> xDesktop = css::frame::Desktop::create(xContext);

    css::uno::Reference<css::frame::XFrame> xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        xFrame = xDesktop;

    mxFrame = xFrame;

    maCommandURL.Complete = aCommand;
    css::uno::Reference<css::util::XURLTransformer> xParser = css::util::URLTransformer::create(xContext);
    xParser->parseStrict(maCommandURL);
}

template<class T>
void VclStatusListener<T>::startListening()
{
    if (mxDispatch.is())
        mxDispatch->removeStatusListener(this, maCommandURL);

    css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(mxFrame, css::uno::UNO_QUERY);
    if (!xDispatchProvider.is())
        return;

    mxDispatch = xDispatchProvider->queryDispatch(maCommandURL, "", 0);
    if (mxDispatch.is())
        mxDispatch->addStatusListener(this, maCommandURL);
}

template<class T>
void VclStatusListener<T>::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    mWidget->statusChanged(rEvent);
}

template<class T>
void VclStatusListener<T>::disposing(const css::lang::EventObject& /*Source*/)
{
    mxDispatch.clear();
}

template<class T>
void VclStatusListener<T>::dispose()
{
    if (mxDispatch.is()) {
        mxDispatch->removeStatusListener(this, maCommandURL);
        mxDispatch.clear();
    }
    mxFrame.clear();
    mWidget.clear();
}


#endif // INCLUDED_VCL_VCLSTATUSLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

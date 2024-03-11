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
#ifndef INCLUDED_DBACCESS_DATAVIEW_HXX
#define INCLUDED_DBACCESS_DATAVIEW_HXX

#include <memory>

#include <com/sun/star/uno/Reference.hxx>
#include <dbaccess/dbaccessdllapi.h>
#include <rtl/ref.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

namespace com::sun::star {
    namespace frame { class XFrame; }
    namespace uno { class XComponentContext; }
}

namespace svt {
    class AcceleratorExecute;
}

class NotifyEvent;
namespace tools { class Rectangle; }

namespace dbaui
{
    class IController;
    class UNLESS_MERGELIBS_MORE(DBACCESS_DLLPUBLIC) ODataView :    public vcl::Window
    {
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;  // the service factory to work with

    protected:
        rtl::Reference<IController> m_xController;  // the controller where we reside in
        ::std::unique_ptr< ::svt::AcceleratorExecute> m_pAccel;

    public:
        ODataView(  vcl::Window* pParent,
                    IController& _rController,
                    const css::uno::Reference< css::uno::XComponentContext >& ,
                    WinBits nStyle = 0 );
        virtual ~ODataView() override;
        virtual void dispose() override;

        /// late construction
        virtual void Construct();
        // initialize will be called when after the controller finished his initialize method
        virtual void initialize(){}
        // window overridables
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
        virtual void StateChanged( StateChangedType nStateChange ) override;

        IController& getCommandController() const { return *m_xController; }

        const css::uno::Reference< css::uno::XComponentContext >& getORB() const { return m_xContext;}

        // the default implementation simply calls resizeAll( GetSizePixel() )
        virtual void Resize() override;

        void attachFrame(const css::uno::Reference< css::frame::XFrame >& _xFrame);
    protected:
        // window overridables
        virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& _rRect ) override;

        /// re-arrange all controls, including the toolbox, it's separator, and the "real view"
        virtual void resizeAll( const tools::Rectangle& _rPlayground );

        // re-arrange the controls belonging to the document itself
        virtual void resizeDocumentView( tools::Rectangle& _rPlayground );
    };
}
#endif // INCLUDED_DBACCESS_DATAVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

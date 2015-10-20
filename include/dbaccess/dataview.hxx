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

#include <dbaccess/dbaccessdllapi.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <svtools/acceleratorexecute.hxx>
#include <sal/macros.h>
#include <vcl/fixed.hxx>

#include <memory>

class FixedLine;
namespace dbaui
{
    class IController;
    class DBACCESS_DLLPUBLIC ODataView :    public vcl::Window
    {
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;  // the service factory to work with

    protected:
        rtl::Reference<IController> m_xController;  // the controller in where we resides in
        VclPtr<FixedLine>   m_aSeparator;
        ::std::unique_ptr< ::svt::AcceleratorExecute> m_pAccel;

    public:
        ODataView(  vcl::Window* pParent,
                    IController& _rController,
                    const css::uno::Reference< css::uno::XComponentContext >& ,
                    WinBits nStyle = 0 );
        virtual ~ODataView();
        virtual void dispose() override;

        /// late construction
        virtual void Construct();
        // initialize will be called when after the controller finished his initialize method
        virtual void initialize(){}
        // window overridables
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
        virtual void StateChanged( StateChangedType nStateChange ) override;
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

        inline IController& getCommandController() const { return *m_xController.get(); }

        css::uno::Reference< css::uno::XComponentContext > getORB() { return m_xContext;}

        // the default implementation simply calls resizeAll( GetSizePixel() )
        virtual void Resize() override;

        void attachFrame(const css::uno::Reference< css::frame::XFrame >& _xFrame);
    protected:
        // window overridables
        virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& _rRect ) override;

        /// re-arrange all controls, including the toolbox, it's separator, and the "real view"
        virtual void resizeAll( const Rectangle& _rPlayground );

        // re-arrange the controls belonging to the document itself
        virtual void resizeDocumentView( Rectangle& _rPlayground );
    };
}
#endif // INCLUDED_DBACCESS_DATAVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

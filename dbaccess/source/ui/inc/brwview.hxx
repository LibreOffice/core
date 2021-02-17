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

#pragma once

#include <vcl/window.hxx>

#include <dbaccess/dataview.hxx>
#include <unotools/eventlisteneradapter.hxx>
#include "dbtreelistbox.hxx"

namespace com::sun::star::awt {
    class XControl;
    class XControlContainer;
    class XControlModel;
}

class Splitter;

namespace dbaui
{
    class SbaGridControl;

    class UnoDataBrowserView final : public ODataView, public ::utl::OEventListenerAdapter
    {
        css::uno::Reference< css::awt::XControl >                 m_xGrid;            // our grid's UNO representation
        css::uno::Reference< css::awt::XControlContainer >        m_xMe;              // our own UNO representation
        VclPtr<InterimDBTreeListBox>   m_pTreeView;
        VclPtr<Splitter>               m_pSplitter;
        mutable VclPtr<SbaGridControl> m_pVclControl;  // our grid's VCL representation

        DECL_LINK( SplitHdl, Splitter*, void );
    // attribute access
    public:
        const css::uno::Reference< css::awt::XControl >&  getGridControl() const  { return m_xGrid; }
        SbaGridControl*         getVclControl() const;

        UnoDataBrowserView( vcl::Window* pParent,
                            IController& _rController,
                            const css::uno::Reference< css::uno::XComponentContext >& );
        virtual ~UnoDataBrowserView() override;
        virtual void dispose() override;

        /// late construction
        void Construct(const css::uno::Reference< css::awt::XControlModel >& xModel);

        /** as columns may be hidden there is a difference between a columns model pos and its view pos
            so we you may use these translation function
        */
        sal_uInt16 View2ModelPos(sal_uInt16 nPos) const;
        /// for the same reason the view column count isn't the same as the model column count

        void setSplitter(Splitter* pSplitter);
        void setTreeView(InterimDBTreeListBox* pTreeView);

        void    showStatus( const OUString& _rStatus );
        void    hideStatus();

        const css::uno::Reference< css::awt::XControlContainer >& getContainer() const { return m_xMe; }

    private:
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
        virtual void GetFocus() override;
        virtual void resizeDocumentView(tools::Rectangle& rRect) override;
        virtual void _disposing( const css::lang::EventObject& _rSource ) override;

        using ODataView::Construct;
    };

    class BrowserViewStatusDisplay final
    {
        VclPtr<UnoDataBrowserView>     m_pView;

    public:
        BrowserViewStatusDisplay( UnoDataBrowserView* _pView, const OUString& _rStatus );
        ~BrowserViewStatusDisplay( );
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

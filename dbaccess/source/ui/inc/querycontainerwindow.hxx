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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYCONTAINERWINDOW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYCONTAINERWINDOW_HXX

#include <vcl/window.hxx>
#include <vcl/split.hxx>
#include <dbaccess/dataview.hxx>
#include <com/sun/star/frame/XFrame2.hpp>
#include "QueryViewSwitch.hxx"
#include <vcl/dockwin.hxx>

namespace dbaui
{

    // OBeamer
    // tempoaray class until the beamer is implemented
    class OBeamer : public DockingWindow
    {
    public:
        OBeamer(vcl::Window* _pParent) : DockingWindow(_pParent,0){}
    };

    // OQueryContainerWindow
    class OQueryContainerWindow : public ODataView
    {
        OQueryViewSwitch*   m_pViewSwitch;
        VclPtr<OBeamer>     m_pBeamer;
        VclPtr<Splitter>    m_pSplitter;
        css::uno::Reference< css::frame::XFrame2 > m_xBeamer;

        DECL_LINK_TYPED( SplitHdl, Splitter*, void );
    public:
        OQueryContainerWindow(vcl::Window* pParent, OQueryController& _rController,const css::uno::Reference< css::uno::XComponentContext >&);
        virtual ~OQueryContainerWindow();
        virtual void dispose() override;

        virtual void Construct() override;

        virtual bool        PreNotify( NotifyEvent& rNEvt ) override;

        // show the beamer
        void    showPreview(const css::uno::Reference< css::frame::XFrame >& _xFrame);
            // called when the beamer has been disposed
        void    disposingPreview();

        const css::uno::Reference< css::frame::XFrame2 >&
                getPreviewFrame() const { return m_xBeamer; }

        OQueryDesignView*   getDesignView() { return m_pViewSwitch->getDesignView(); }

        bool isCutAllowed()     { return m_pViewSwitch->isCutAllowed(); }
        bool isPasteAllowed()   { return m_pViewSwitch->isPasteAllowed(); }
        bool isCopyAllowed()    { return m_pViewSwitch->isCopyAllowed(); }
        void copy()                 { m_pViewSwitch->copy(); }
        void cut()                  { m_pViewSwitch->cut(); }
        void paste()                { m_pViewSwitch->paste(); }

        void clear()                                                        { m_pViewSwitch->clear(); }
        bool isSlotEnabled( sal_Int32 _nSlotId )                        { return m_pViewSwitch->isSlotEnabled( _nSlotId ); }
        void     setSlotEnabled( sal_Int32 _nSlotId, bool _bEnable )    { m_pViewSwitch->setSlotEnabled( _nSlotId, _bEnable ); }
        void     setNoneVisbleRow(sal_Int32 _nRows)                         { m_pViewSwitch->setNoneVisbleRow( _nRows); }

        bool checkStatement()                                           { return m_pViewSwitch->checkStatement( ); }
        OUString getStatement()                                      { return m_pViewSwitch->getStatement( ); }
        void setStatement( const OUString& _rsStatement )            { m_pViewSwitch->setStatement( _rsStatement ); }

        void    initialize() override                                                { m_pViewSwitch->initialize(); }
        void    SaveUIConfig()                                              { m_pViewSwitch->SaveUIConfig(); }
        void    reset( ::dbtools::SQLExceptionInfo* _pErrorInfo )           { m_pViewSwitch->reset( _pErrorInfo ); }

        bool    switchView( ::dbtools::SQLExceptionInfo* _pErrorInfo );
        void    forceInitialView();

        virtual void GetFocus() override;

    protected:
        // re-arrange the controls belonging to the document itself
        virtual void resizeAll( const Rectangle& _rPlayground ) override;

        // arrange derived classes controls in the rectangle given
        virtual void resizeDocumentView(Rectangle& _rPlayground) override;
    };
    // end of temp classes

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYCONTAINERWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

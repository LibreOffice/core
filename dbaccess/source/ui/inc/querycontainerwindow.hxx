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

#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#define DBAUI_QUERYCONTAINERWINDOW_HXX

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
        OBeamer(Window* _pParent) : DockingWindow(_pParent,0){}
    };

    // OQueryContainerWindow
    class OQueryContainerWindow : public ODataView
    {
        OQueryViewSwitch*   m_pViewSwitch;
        OBeamer*            m_pBeamer;
        Splitter*           m_pSplitter;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame2 > m_xBeamer;

        DECL_LINK( SplitHdl, void* );
    public:
        OQueryContainerWindow(Window* pParent, OQueryController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);
        ~OQueryContainerWindow();

        virtual void Construct();

        virtual long        PreNotify( NotifyEvent& rNEvt );

        // show the beamer
        void    showPreview(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);
            // called when the beamer has been disposed
        void    disposingPreview();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame2 >
                getPreviewFrame() const { return m_xBeamer; }

        OQueryDesignView*   getDesignView() { return m_pViewSwitch->getDesignView(); }

        sal_Bool isCutAllowed()     { return m_pViewSwitch->isCutAllowed(); }
        sal_Bool isPasteAllowed()   { return m_pViewSwitch->isPasteAllowed(); }
        sal_Bool isCopyAllowed()    { return m_pViewSwitch->isCopyAllowed(); }
        void copy()                 { m_pViewSwitch->copy(); }
        void cut()                  { m_pViewSwitch->cut(); }
        void paste()                { m_pViewSwitch->paste(); }

        void clear()                                                        { m_pViewSwitch->clear(); }
        sal_Bool isSlotEnabled( sal_Int32 _nSlotId )                        { return m_pViewSwitch->isSlotEnabled( _nSlotId ); }
        void     setSlotEnabled( sal_Int32 _nSlotId, sal_Bool _bEnable )    { m_pViewSwitch->setSlotEnabled( _nSlotId, _bEnable ); }
        void     setNoneVisbleRow(sal_Int32 _nRows)                         { m_pViewSwitch->setNoneVisbleRow( _nRows); }

        void setReadOnly( sal_Bool _bReadOnly )                             { m_pViewSwitch->setReadOnly( _bReadOnly ); }

        sal_Bool checkStatement()                                           { return m_pViewSwitch->checkStatement( ); }
        OUString getStatement()                                      { return m_pViewSwitch->getStatement( ); }
        void setStatement( const OUString& _rsStatement )            { m_pViewSwitch->setStatement( _rsStatement ); }

        void    initialize()                                                { m_pViewSwitch->initialize(); }
        void    SaveUIConfig()                                              { m_pViewSwitch->SaveUIConfig(); }
        bool    reset( ::dbtools::SQLExceptionInfo* _pErrorInfo )           { return m_pViewSwitch->reset( _pErrorInfo ); }

        bool    switchView( ::dbtools::SQLExceptionInfo* _pErrorInfo );
        void    forceInitialView();

        virtual void GetFocus();

    protected:
        // re-arrange the controls belonging to the document itself
        virtual void resizeAll( const Rectangle& _rPlayground );

        // arrange dericed classes controls in the rectangle given
        virtual void resizeDocumentView(Rectangle& _rPlayground);
    };
    // end of temp classes

}   // namespace dbaui

#endif // DBAUI_QUERYCONTAINERWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#define DBAUI_QUERYCONTAINERWINDOW_HXX

#include <vcl/window.hxx>
#include <vcl/split.hxx>
#include "dataview.hxx"
#include <com/sun/star/frame/XFrame.hpp>
#include "QueryViewSwitch.hxx"
#include <vcl/dockwin.hxx>

class FixedLine;
//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= OBeamer
    //=====================================================================
    // tempoaray class until the beamer is implemented
    class OBeamer : public DockingWindow
    {
    public:
        OBeamer(Window* _pParent) : DockingWindow(_pParent,0){}
    };

    //=====================================================================
    //= OQueryContainerWindow
    //=====================================================================
    class OQueryContainerWindow : public ODataView
    {
        OQueryViewSwitch*   m_pViewSwitch;
        OBeamer*            m_pBeamer;
        Splitter*           m_pSplitter;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xBeamer;

        DECL_LINK( SplitHdl, void* );
    public:
        OQueryContainerWindow(Window* pParent, OQueryController& _rController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        ~OQueryContainerWindow();

        virtual void Construct();

        virtual long        PreNotify( NotifyEvent& rNEvt );

        // show the beamer
        void    showPreview(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);
            // called when the beamer has been disposed
        void    disposingPreview();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
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
        ::rtl::OUString getStatement()                                      { return m_pViewSwitch->getStatement( ); }
        void setStatement( const ::rtl::OUString& _rsStatement )            { m_pViewSwitch->setStatement( _rsStatement ); }

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

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_QUERYCONTAINERWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

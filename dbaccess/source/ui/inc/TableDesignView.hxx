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
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#define DBAUI_TABLEDESIGNVIEW_HXX

#include <dbaccess/dataview.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <vcl/split.hxx>
#include "IClipBoardTest.hxx"

namespace dbaui
{
    class OTableController;
    class OTableFieldDescWin;
    class OTableEditorCtrl;
    class OTableBorderWindow : public Window
    {
        Splitter                            m_aHorzSplitter;
        OTableFieldDescWin*                 m_pFieldDescWin;
        OTableEditorCtrl*                   m_pEditorCtrl;

        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
        void ArrangeChildren( long nSplitPos ,Rectangle& rRect);
        DECL_LINK( SplitHdl, Splitter* );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OTableBorderWindow(Window* pParent);
        ~OTableBorderWindow();
        // window overloads
        virtual void Resize();
        virtual void GetFocus();

        OTableEditorCtrl*       GetEditorCtrl() const { return m_pEditorCtrl; }
        OTableFieldDescWin*     GetDescWin()    const { return m_pFieldDescWin; }
    };
    class OTableDesignView : public ODataView
                            ,public IClipboardTest
    {
        enum ChildFocusState
        {
            DESCRIPTION,
            EDITOR,
            NONE
        };
    private:
        ::com::sun::star::lang::Locale      m_aLocale;
        OTableBorderWindow*                 m_pWin;
        OTableController&                   m_rController;
        ChildFocusState                     m_eChildFocus;

        IClipboardTest* getActiveChild() const;
    protected:

        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);

    public:
        OTableDesignView(   Window* pParent,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&,
                            OTableController& _rController);
        virtual ~OTableDesignView();

        // window overloads
        virtual long            PreNotify( NotifyEvent& rNEvt );
        virtual void            GetFocus();

        OTableEditorCtrl*       GetEditorCtrl() const { return m_pWin ? m_pWin->GetEditorCtrl() : NULL; }
        OTableFieldDescWin*     GetDescWin()    const { return m_pWin ? m_pWin->GetDescWin() : NULL; }
        OTableController&       getController() const { return m_rController; }

        ::com::sun::star::lang::Locale      getLocale() const { return m_aLocale;}

        // IClipboardTest
        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }
        virtual void copy();
        virtual void cut();
        virtual void paste();

        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);

        virtual void initialize();
        void reSync(); // resync window data with realdata
    };
}
#endif // DBAUI_TABLEDESIGNVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#define DBAUI_TABLEDESIGNVIEW_HXX

#include "dataview.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <vcl/split.hxx>
#include "IClipBoardTest.hxx"

namespace dbaui
{
    class OTableController;
    class OTableFieldDescWin;
    class OTableEditorCtrl;
    //==================================================================
    class OTableBorderWindow : public Window
    {
        Splitter                            m_aHorzSplitter;
        OTableFieldDescWin*                 m_pFieldDescWin;
        OTableEditorCtrl*                   m_pEditorCtrl;

        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
        void ArrangeChilds( long nSplitPos ,Rectangle& rRect);
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
    //==================================================================
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
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
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

        DECL_LINK( SwitchHdl, Accelerator* );
    };
}
#endif // DBAUI_TABLEDESIGNVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEDESIGNVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEDESIGNVIEW_HXX

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
    class OTableBorderWindow : public vcl::Window
    {
        VclPtr<Splitter>                    m_aHorzSplitter;
        VclPtr<OTableFieldDescWin>          m_pFieldDescWin;
        VclPtr<OTableEditorCtrl>            m_pEditorCtrl;

        void ImplInitSettings();
        DECL_LINK_TYPED( SplitHdl, Splitter*, void );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    public:
        OTableBorderWindow(vcl::Window* pParent);
        virtual ~OTableBorderWindow();
        // Window overrides
        virtual void dispose() override;
        virtual void Resize() override;
        virtual void GetFocus() override;

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
        css::lang::Locale                   m_aLocale;
        VclPtr<OTableBorderWindow>          m_pWin;
        OTableController&                   m_rController;
        ChildFocusState                     m_eChildFocus;

        IClipboardTest* getActiveChild() const;
    protected:

        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect) override;

    public:
        OTableDesignView(   vcl::Window* pParent,
                            const css::uno::Reference< css::uno::XComponentContext >&,
                            OTableController& _rController);
        virtual ~OTableDesignView();
        virtual void dispose() override;

        // Window overrides
        virtual bool            PreNotify( NotifyEvent& rNEvt ) override;
        virtual void            GetFocus() override;

        OTableEditorCtrl*       GetEditorCtrl() const { return m_pWin ? m_pWin->GetEditorCtrl() : nullptr; }
        OTableFieldDescWin*     GetDescWin()    const { return m_pWin ? m_pWin->GetDescWin() : nullptr; }
        OTableController&       getController() const { return m_rController; }

        const css::lang::Locale& getLocale() const { return m_aLocale;}

        // IClipboardTest
        virtual bool isCutAllowed() override;
        virtual bool isCopyAllowed() override;
        virtual bool isPasteAllowed() override;
        virtual void copy() override;
        virtual void cut() override;
        virtual void paste() override;

        // set the view readonly or not
        void setReadOnly(bool _bReadOnly);

        virtual void initialize() override;
        void reSync(); // resync window data with realdata
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEDESIGNVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

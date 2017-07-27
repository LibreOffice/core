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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_DATAEDITOR_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_DATAEDITOR_HXX

#include <svl/lstner.hxx>
#include <vcl/toolbox.hxx>
#include <comphelper/stl_types.hxx>
#include <vcl/dialog.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
    }
}}}

namespace chart
{

class DataBrowser;

class DataEditor : public ModalDialog
{
public:
    DataEditor( vcl::Window* pParent,
                const css::uno::Reference< css::chart2::XChartDocument > & xChartDoc,
                const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~DataEditor() override;
    virtual void dispose() override;

    // Dialog
    virtual bool Close() override;

    void SetReadOnly( bool bReadOnly );
    bool ApplyChangesToModel();

private:
    sal_uInt16 TBI_DATA_INSERT_ROW;
    sal_uInt16 TBI_DATA_INSERT_COL;
    sal_uInt16 TBI_DATA_INSERT_TEXT_COL;
    sal_uInt16 TBI_DATA_DELETE_ROW;
    sal_uInt16 TBI_DATA_DELETE_COL;
    sal_uInt16 TBI_DATA_MOVE_LEFT_COL;
    sal_uInt16 TBI_DATA_MOVE_RIGHT_COL;
    sal_uInt16 TBI_DATA_MOVE_UP_ROW;
    sal_uInt16 TBI_DATA_MOVE_DOWN_ROW;

    bool                           m_bReadOnly;
    VclPtr<DataBrowser>            m_xBrwData;
    VclPtr<ToolBox>                m_pTbxData;
    css::uno::Reference< css::chart2::XChartDocument > m_xChartDoc;
    css::uno::Reference< css::uno::XComponentContext >
        m_xContext;

    /// handles actions of the toolbox
    DECL_LINK( ToolboxHdl, ToolBox*, void );
    /// is called, if the cursor of the table has moved
    DECL_LINK( BrowserCursorMovedHdl, DataBrowser*, void);
    /// this is called if MiscOptions change, esp. High-Contrast mode
    DECL_LINK( MiscHdl, LinkParamNone*, void );

    /** notifySystemWindow adds or remove the given window pToRegister at the Systemwindow found when search pWindow.
        @param  pWindow
            The window which is used to search for the SystemWindow.
        @param  pToRegister
            The window which should be added or removed on the TaskPaneList.
        @param  rMemFunc
            The member function which should be called at the SystemWindow when found.
            Possible values are:
            ::comphelper::mem_fun(&TaskPaneList::AddWindow)
            ::comphelper::mem_fun(&TaskPaneList::RemoveWindow)

        @note this code is taken from dbaccess/source/ui/inc/UITools.hxx
    */
    static void notifySystemWindow(vcl::Window const * pWindow,
                            vcl::Window* pToRegister,
                            const ::comphelper::mem_fun1_t<TaskPaneList, vcl::Window*>& rMemFunc);
};

} // namespace chart

#endif // INCLUDED_CHART2_SOURCE_CONTROLLER_INC_DLG_DATAEDITOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

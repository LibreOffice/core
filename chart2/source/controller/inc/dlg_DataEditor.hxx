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

#ifndef _SCH_DGRDATA_HXX
#define _SCH_DGRDATA_HXX

#include <svl/lstner.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/toolbox.hxx>
#include <comphelper/stl_types.hxx>
#include <vcl/dialog.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

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
    DataEditor( Window* pParent,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XChartDocument > & xChartDoc,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataEditor();

    // Window
    virtual void Resize();

    // Dialog
    virtual sal_Bool Close();

    void SetReadOnly( bool bReadOnly );
    bool ApplyChangesToModel();

private:
    bool                            m_bReadOnly;
    ::std::auto_ptr< DataBrowser >  m_apBrwData;
    ToolBox                         m_aTbxData;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDoc;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    ImageList       m_aToolboxImageList;

    /// handles actions of the toolbox
    DECL_LINK( ToolboxHdl, void* );
    /// is called, if the cursor of the table has moved
    DECL_LINK( BrowserCursorMovedHdl, void*);
    /// this is called if MiscOptions change, esp. High-Contrast mode
    DECL_LINK( MiscHdl, void* );
    /// is called when the contents of the edit cell changes
    DECL_LINK( CellModified, void* );
    /// is called when the width of any column in the edit browsebox has changed
    DECL_LINK( BrowserColumnResized, void* );
    /// is called when the browser view was scrolled, @todo: only call on horizontal scroll
    DECL_LINK( BrowserContentScrolled, void* );

    void UpdateData();
    void ApplyImageList();
    /// moved and resizes the series name control etc. to fit the dimensions of the edit browsebox
//     void ImplAdjustHeaderControls( bool bRefreshFromModel );

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
    void notifySystemWindow( Window* pWindow,
                             Window* pToRegister,
                             ::comphelper::mem_fun1_t< TaskPaneList, Window* > rMemFunc );

    void AdaptBrowseBoxSize();
};

} // namespace chart

#endif  // _SCH_DGRDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

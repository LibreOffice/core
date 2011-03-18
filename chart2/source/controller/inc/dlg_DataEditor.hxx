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

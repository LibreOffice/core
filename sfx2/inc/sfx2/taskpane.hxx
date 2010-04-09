/*************************************************************************
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

#ifndef SFX_TASKPANE_HXX
#define SFX_TASKPANE_HXX

#include "sfx2/childwin.hxx"
#include "sfx2/titledockwin.hxx"

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

    //==================================================================================================================
    //= TaskPaneWrapper
    //==================================================================================================================
    class TaskPaneWrapper : public SfxChildWindow
    {
    public:
        TaskPaneWrapper(
            Window* i_pParent,
            USHORT i_nId,
            SfxBindings* i_pBindings,
            SfxChildWinInfo* i_pInfo
        );

        SFX_DECL_CHILDWINDOW( TaskPaneWrapper );
    };

    //==================================================================================================================
    //= TaskPane
    //==================================================================================================================
    class ModuleTaskPane_Impl;
    /** SFX-less version of a module dependent task pane, filled with tool panels as specified in the respective
        module's configuration
    */
    class ModuleTaskPane : public Window
    {
    public:
        /** creates a new instance
            @param i_rParentWindow
                the parent window
            @param i_rModuleIdentifier
                a string describing the module which the task pane should act for. The tool panels registered
                for this module will be displayed in the task pane.
        */
        ModuleTaskPane( Window& i_rParentWindow, const ::rtl::OUString& i_rModuleIdentifier );
        ~ModuleTaskPane();

        /** determines whether a given module has any registered tool panels
        */
        static bool ModuleHasToolPanels( const ::rtl::OUString& i_rModuleIdentifier );
        /** determines whether a given module has any registered tool panels
        */
        static bool ModuleHasToolPanels( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rDocumentFrame );

        // Window overridables
        virtual void Resize();
        virtual void GetFocus();

    private:
        ::boost::scoped_ptr< ModuleTaskPane_Impl >  m_pImpl;
    };

    //==================================================================================================================
    //= TaskPaneDockingWindow
    //==================================================================================================================
    class TaskPaneDockingWindow : public TitledDockingWindow
    {
    public:
        TaskPaneDockingWindow( SfxBindings* i_pBindings, TaskPaneWrapper& i_rWrapper,
            Window* i_pParent, WinBits i_nBits );

    protected:
        // Window overridables
        virtual void        GetFocus();
        virtual long        Notify( NotifyEvent& i_rNotifyEvent );

        // TitledDockingWindow overridables
        virtual void onLayoutDone();

    private:
        ModuleTaskPane  m_aTaskPane;
    };

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................

#endif // SFX_TASKPANE_HXX

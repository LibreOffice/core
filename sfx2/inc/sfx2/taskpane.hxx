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

#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>

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
    class TaskPane_Impl;
    class TaskPane : public SfxDockingWindow
    {
    public:
                            TaskPane(
                                SfxBindings* i_pBindings,
                                TaskPaneWrapper& i_rWrapper,
                                Window* i_pParent,
                                WinBits i_nBits
                            );

        virtual void        GetFocus();
        virtual void        Resize();

    private:
        ::boost::scoped_ptr< TaskPane_Impl >    m_pImpl;
    };

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................

#endif // SFX_TASKPANE_HXX

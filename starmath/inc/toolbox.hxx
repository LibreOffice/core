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

#ifndef TOOLBOX_HXX
#define TOOLBOX_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <vcl/toolbox.hxx>

#include "smmod.hxx"
#include "config.hxx"
#include "toolbox.hrc"

class SmToolBoxWindow : public SfxFloatingWindow
{

protected:
    ToolBox     aToolBoxCat;
    FixedLine   aToolBoxCat_Delim;  // to visualy seperate the catalog part
    ToolBox    *pToolBoxCmd;
    ToolBox    *vToolBoxCategories[NUM_TBX_CATEGORIES];
    ImageList  *aImageLists [NUM_TBX_CATEGORIES + 1];   /* regular */
    sal_uInt16      nActiveCategoryRID;

    virtual sal_Bool    Close();
    virtual void    GetFocus();

    void            ApplyImageLists( sal_uInt16 nCategoryRID );

    DECL_LINK( CategoryClickHdl, ToolBox* );
    DECL_LINK( CmdSelectHdl, ToolBox* );

    SmViewShell * GetView();
    const ImageList * GetImageList( sal_uInt16 nResId );

public:
    SmToolBoxWindow(SfxBindings    *pBindings,
                    SfxChildWindow *pChildWindow,
                    Window         *pParent);
    ~SmToolBoxWindow();

    // Window
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent &rEvt );

    void        AdjustPosSize( bool bSetPos );
    void        SetCategory(sal_uInt16 nCategory);
};

/**************************************************************************/

class SmToolBoxWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW(SmToolBoxWrapper);

protected:
    SmToolBoxWrapper(Window *pParentWindow,
                     sal_uInt16, SfxBindings*, SfxChildWinInfo*);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

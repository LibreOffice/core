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

#ifndef INCLUDED_STARMATH_INC_TOOLBOX_HXX
#define INCLUDED_STARMATH_INC_TOOLBOX_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <vcl/toolbox.hxx>

#include "smmod.hxx"
#include "config.hxx"
#include "toolbox.hrc"

class SmToolBoxWindow : public SfxFloatingWindow
{

protected:
    VclPtr<ToolBox>    m_pToolBoxCat;
    sal_uInt16  m_nUnbinopsId;
    sal_uInt16  m_nRelationsId;
    sal_uInt16  m_nSetoperationsId;
    sal_uInt16  m_nFunctionsId;
    sal_uInt16  m_nOperatorsId;
    sal_uInt16  m_nAttributesId;
    sal_uInt16  m_nBracketsId;
    sal_uInt16  m_nFormatId;
    sal_uInt16  m_nMiscId;
    VclPtr<ToolBox>    pToolBoxCmd;
    VclPtr<ToolBox>    vToolBoxCategories[NUM_TBX_CATEGORIES];
    ImageList  *aImageLists [NUM_TBX_CATEGORIES + 1];   /* regular */
    sal_uInt16      nActiveCategoryRID;

    virtual bool    Close() SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;

    void            ApplyImageLists( sal_uInt16 nCategoryRID );

    DECL_LINK( CategoryClickHdl, ToolBox* );
    DECL_LINK( CmdSelectHdl, ToolBox* );

    SmViewShell * GetView();
    const ImageList * GetImageList( sal_uInt16 nResId );

    sal_uInt16 MapToolbarIdToCategory(sal_uInt16 nId) const;

public:
    SmToolBoxWindow(SfxBindings    *pBindings,
                    SfxChildWindow *pChildWindow,
                    Window         *pParent);
    virtual ~SmToolBoxWindow();
    virtual void dispose() SAL_OVERRIDE;

    // Window
    virtual void    StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent &rEvt ) SAL_OVERRIDE;

    void        AdjustPos();
    void        SetCategory(sal_uInt16 nCategory);
};

/**************************************************************************/

class SmToolBoxWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(SmToolBoxWrapper);

protected:
    SmToolBoxWrapper(vcl::Window *pParentWindow,
                     sal_uInt16, SfxBindings*, SfxChildWinInfo*);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

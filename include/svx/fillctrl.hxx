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

#ifndef INCLUDED_SVX_FILLCTRL_HXX
#define INCLUDED_SVX_FILLCTRL_HXX

#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/svxdllapi.h>

class XFillStyleItem;
class XFillColorItem;
class XFillGradientItem;
class XFillHatchItem;
class XFillBitmapItem;
class FillControl;
class SvxFillTypeBox;
class SvxFillAttrBox;
class ListBox;

/*************************************************************************
|*
|* Class for surface attributes (controls and controller)
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxFillToolBoxControl: public SfxToolBoxControl
{
private:
    XFillStyleItem*     pStyleItem;
    XFillColorItem*     pColorItem;
    XFillGradientItem*  pGradientItem;
    XFillHatchItem*     pHatchItem;
    XFillBitmapItem*    pBitmapItem;

    FillControl*        pFillControl;
    SvxFillTypeBox*     pFillTypeLB;
    SvxFillAttrBox*     pFillAttrLB;

    bool                bUpdate;
    sal_uInt16          eLastXFS;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFillToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxFillToolBoxControl();

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState ) SAL_OVERRIDE;
    void                Update( const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent ) SAL_OVERRIDE;
};



class FillControl : public Window
{
private:
    friend class SvxFillToolBoxControl;

    SvxFillTypeBox* pLbFillType;
    SvxFillAttrBox* pLbFillAttr;
    Size            aLogicalFillSize;
    Size            aLogicalAttrSize;
    Timer           aDelayTimer;

    DECL_LINK( DelayHdl, void * );
    DECL_LINK( SelectFillTypeHdl, ListBox * );
    DECL_LINK( SelectFillAttrHdl, ListBox * );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
public:
    FillControl( Window* pParent, WinBits nStyle = 0 );
    virtual ~FillControl();

    virtual void Resize() SAL_OVERRIDE;
};

#endif // INCLUDED_SVX_FILLCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

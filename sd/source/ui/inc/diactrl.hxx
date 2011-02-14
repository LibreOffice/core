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

#ifndef _SD_DIACTRL_HXX
#define _SD_DIACTRL_HXX

#include "dlgctrls.hxx"
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <svx/itemwin.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>

//========================================================================
// SdPagesField:

class SdPagesField : public SvxMetricField
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
protected:
    virtual void    Modify();

public:
                    SdPagesField( Window* pParent,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                  WinBits nBits = WB_BORDER | WB_SPIN | WB_REPEAT );
                    ~SdPagesField();

    void            UpdatePagesField( const SfxUInt16Item* pItem );
};

//========================================================================
// SdTbxCtlDiaPages:
//========================================================================

class SdTbxCtlDiaPages : public SfxToolBoxControl
{
public:
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

    SFX_DECL_TOOLBOX_CONTROL();

    SdTbxCtlDiaPages( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SdTbxCtlDiaPages();
};

#endif // _SD_DIACTRL_HXX




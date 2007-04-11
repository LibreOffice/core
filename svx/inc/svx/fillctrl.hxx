/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillctrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:49:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FILLCTRL_HXX
#define _FILLCTRL_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
|* Klassen fuer Flaechenattribute (Controls und Controller)
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

    BOOL                bUpdate;
    BOOL                bIgnoreStatusUpdate;
    USHORT              eLastXFS;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFillToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFillToolBoxControl();

    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    void                Update( const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );
    void                IgnoreStatusUpdate( sal_Bool bSet );
};

//========================================================================

class FillControl : public Window
{
private:
    friend class SvxFillToolBoxControl;

    SvxFillTypeBox* pLbFillType;
    SvxFillAttrBox* pLbFillAttr;
    Size            aLogicalFillSize;
    Size            aLogicalAttrSize;
    Timer           aDelayTimer;

//#if 0 // _SOLAR__PRIVATE
    DECL_LINK( DelayHdl, Timer * );
    DECL_LINK( SelectFillTypeHdl, ListBox * );
    DECL_LINK( SelectFillAttrHdl, ListBox * );
//#endif
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
public:
    FillControl( Window* pParent, WinBits nStyle = 0 );
    ~FillControl();

    virtual void Resize();
};

#endif      // _FILLCTRL_HXX


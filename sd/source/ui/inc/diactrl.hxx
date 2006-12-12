/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diactrl.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:41:36 $
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

#ifndef _SD_DIACTRL_HXX
#define _SD_DIACTRL_HXX

#ifndef SD_DLGCTRLS_HXX
#include "dlgctrls.hxx"
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_ITEMWIN_HXX //autogen
#include <svx/itemwin.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

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
    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

    SFX_DECL_TOOLBOX_CONTROL();

    SdTbxCtlDiaPages( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SdTbxCtlDiaPages();
};

#endif // _SD_DIACTRL_HXX




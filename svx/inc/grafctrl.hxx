/*************************************************************************
 *
 *  $RCSfile: grafctrl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-24 17:49:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_GRAFCTRL_HXX
#define _SVX_GRAFCTRL_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

// ----------------
// - TbxImageItem -
// ----------------

class TbxImageItem : public SfxUInt16Item
{
public:
                            TYPEINFO();
                            TbxImageItem( USHORT nWhich = 0, UINT16 nImage = 0 );

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
};

// -------------------------------
// - SvxGrafFilterToolBoxControl -
// -------------------------------

class SvxGrafFilterToolBoxControl : public SfxToolBoxControl
{
public:

                                SFX_DECL_TOOLBOX_CONTROL();

                                SvxGrafFilterToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
                                ~SvxGrafFilterToolBoxControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

// -------------------------
// - SvxGrafToolBoxControl -
// -------------------------

class SvxGrafToolBoxControl : public SfxToolBoxControl, public SfxListener
{
public:

                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
                        ~SvxGrafToolBoxControl();

    virtual void        StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );
    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                    const SfxHint& rHint, const TypeId& rHintType );
};

// ----------------------------
// - SvxGrafRedToolBoxControl -
// ----------------------------

class SvxGrafRedToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafRedToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
};

// ------------------------------
// - SvxGrafGreenToolBoxControl -
// ------------------------------

class SvxGrafGreenToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafGreenToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
};

// -----------------------------
// - SvxGrafBlueToolBoxControl -
// -----------------------------

class SvxGrafBlueToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafBlueToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
};

// ----------------------------------
// - SvxGrafLuminanceToolBoxControl -
// ----------------------------------

class SvxGrafLuminanceToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafLuminanceToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
};

// ---------------------------------
// - SvxGrafContrastToolBoxControl -
// ---------------------------------

class SvxGrafContrastToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafContrastToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
};

// ------------------------------
// - SvxGrafGammaToolBoxControl -
// ------------------------------

class SvxGrafGammaToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafGammaToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
};

// -------------------------------------
// - SvxGrafTransparenceToolBoxControl -
// -------------------------------------

class SvxGrafTransparenceToolBoxControl : public SvxGrafToolBoxControl
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafTransparenceToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
};

// -----------------------------
// - SvxGrafModeToolBoxControl -
// -----------------------------

class SvxGrafModeToolBoxControl : public SfxToolBoxControl, public SfxListener
{
public:
                        SFX_DECL_TOOLBOX_CONTROL();
                        SvxGrafModeToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
                        ~SvxGrafModeToolBoxControl();

    virtual void        StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );
    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                    const SfxHint& rHint, const TypeId& rHintType );
};


#endif // _SVX_GRAFCTRL_HXX

/*************************************************************************
 *
 *  $RCSfile: diactrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:12:50 $
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

#ifndef _SD_DIACTRL_HXX
#define _SD_DIACTRL_HXX

#pragma hdrstop

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


/*************************************************************************
|*
|* Controls (Windows) fuer Diaeffekte
|*
\************************************************************************/

//========================================================================
// DiaTimeControl:

class DiaTimeControl : public TimeField
{
private:

    SfxBindings&    rBindings;

protected:
    virtual void    Up();
    virtual void    Down();

public:
                DiaTimeControl( Window* pParent, SfxBindings& rBindings, WinBits nStyle = 0 );
                DiaTimeControl( Window* pParent, SfxBindings& rBindings, ResId nRId );
                ~DiaTimeControl();

    DECL_LINK( ModifyDiaTimeHdl, void * );
    DECL_LINK( GetFocusHdl, void * );
};

#ifdef _SD_DIACTRL_CXX

//========================================================================
// DiaEffectControl:

class DiaEffectControl : public Window
{
private:
    SfxBindings&    rBindings;
    FixedText       aFtDescr;
public:
    FadeEffectLB    aLbEffect;

public:
                DiaEffectControl( Window* pParent, SfxBindings& rBindings, WinBits nStyle = 0 );
                ~DiaEffectControl();

    DECL_LINK( SelectDiaEffectHdl, void * );
};

//========================================================================
// DiaSpeedControl:

class DiaSpeedControl : public Window
{
private:
    SfxBindings&    rBindings;

public:
    ListBox         aLbSpeed;

public:
                DiaSpeedControl( Window* pParent, SfxBindings& rBindings, WinBits nStyle = 0 );
                ~DiaSpeedControl();

    DECL_LINK( SelectDiaSpeedHdl, void * );
};

//========================================================================
// DiaAutoControl:

class DiaAutoControl : public Window
{
private:
    SfxBindings&    rBindings;
    FixedText       aFtDescr;
public:
    ListBox         aLbAuto;

public:
                DiaAutoControl( Window* pParent, SfxBindings& rBindings, WinBits nStyle = 0 );
                ~DiaAutoControl();

    DECL_LINK( SelectDiaAutoHdl, void * );
};

//========================================================================
// SdPagesField:

class SdPagesField : public SvxMetricField
{
private:
    SfxBindings&    rBindings;
protected:
    virtual void    Modify();

public:
                    SdPagesField( Window* pParent, SfxBindings& rBindings,
                        WinBits nBits = WB_BORDER | WB_SPIN | WB_REPEAT );
                    ~SdPagesField();

    void            Update( const SfxUInt16Item* pItem );
};


/*************************************************************************
|*
|* Toolbox-Controller fuer Diaeffekte
|*
\************************************************************************/

class SdTbxCtlDiaEffect: public SfxToolBoxControl
{
public:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxCtlDiaEffect( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
            ~SdTbxCtlDiaEffect() {}
};

class SdTbxCtlDiaSpeed: public SfxToolBoxControl
{
public:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxCtlDiaSpeed( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
            ~SdTbxCtlDiaSpeed() {}
};

class SdTbxCtlDiaAuto: public SfxToolBoxControl
{
public:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxCtlDiaAuto( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
            ~SdTbxCtlDiaAuto() {}
};

class SdTbxCtlDiaTime: public SfxToolBoxControl
{
public:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxCtlDiaTime( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
            ~SdTbxCtlDiaTime() {}
};

//========================================================================
// SdTbxCtlDiaPages:
//========================================================================

class SdTbxCtlDiaPages : public SfxToolBoxControl
{
protected:
    SfxBindings&        rBindings;

public:
    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

    SFX_DECL_TOOLBOX_CONTROL();

    SdTbxCtlDiaPages( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
    ~SdTbxCtlDiaPages();
};

#endif // _SD_DIACTRL_CXX

#endif // _SD_TBXCTRLS_HXX




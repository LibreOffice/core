/*************************************************************************
 *
 *  $RCSfile: linectrl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:57 $
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
#ifndef _SVX_LINECTRL_HXX
#define _SVX_LINECTRL_HXX


#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif
class XLineStyleItem;
class XLineDashItem;
class SvxLineBox;
class SvxMetricField;
class SvxColorBox;
class XLineEndList;

//========================================================================
// SvxLineStyleController:
//========================================================================

class SvxLineStyleToolBoxControl : public SfxToolBoxControl, public SfxListener
{
private:
    XLineStyleItem*     pStyleItem;
    XLineDashItem*      pDashItem;

    SfxStatusForwarder  aDashForwarder;
    BOOL                bUpdate;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxLineStyleToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
    ~SvxLineStyleToolBoxControl();

    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType );
    virtual Window*     CreateItemWindow( Window *pParent );
};

//========================================================================
// SvxLineWidthController:
//========================================================================

class SvxLineWidthToolBoxControl : public SfxToolBoxControl, public SfxListener
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxLineWidthToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
    ~SvxLineWidthToolBoxControl();

    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType );
    virtual Window*     CreateItemWindow( Window *pParent );
};

//========================================================================
// SvxLineColorController:
//========================================================================

class SvxLineColorToolBoxControl : public SfxToolBoxControl, public SfxListener
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxLineColorToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
    ~SvxLineColorToolBoxControl();

    virtual void        StateChanged( USHORT nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType );
    virtual Window*     CreateItemWindow( Window *pParent );
};

//========================================================================
// class SvxLineEndWindow
//========================================================================
class SvxLineEndWindow : public SfxPopupWindow, public SfxListener
{
private:
    XLineEndList*   pLineEndList;
    ValueSet        aLineEndSet;
    USHORT          nCols;
    USHORT          nLines;
    ULONG           nLineEndWidth;
    Size            aBmpSize;
    BOOL            bPopupMode;

    DECL_LINK( SelectHdl, void * );
    void            FillValueSet();
    void            SetSize();

protected:
    virtual void    Resizing( Size& rSize );
    virtual void    Resize();
    virtual BOOL    Close();
    virtual void    PopupModeEnd();

public:
    SvxLineEndWindow( USHORT nId, const String& rWndTitle,
                      SfxBindings& rBindings );
    ~SvxLineEndWindow();

    void            StartSelection();

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );
    virtual SfxPopupWindow* Clone() const;
};

//========================================================================
// class SvxColorToolBoxControl
//========================================================================

class SvxLineEndToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxLineEndToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~SvxLineEndToolBoxControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};



#endif


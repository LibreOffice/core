/*************************************************************************
 *
 *  $RCSfile: extrusioncontrols.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-04-02 13:55:22 $
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
#ifndef _SVX_EXTRUSION_CONTROLS_HXX
#define _SVX_EXTRUSION_CONTROLS_HXX

#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

class SfxBindings;
class ToolbarMenu;
class SfxStatusForwarder;
class SvxTbxButtonColorUpdater_Impl;

//========================================================================

namespace svx
{

class ExtrusionDirectionWindow : public SfxPopupWindow
{
private:
    ToolbarMenu*        mpMenu;
    ValueSet*           mpDirectionSet;

    SfxStatusForwarder* mpDirectionForewarder;
    SfxStatusForwarder* mpProjectionForewarder;

    Image       maImgDirection[9];
    Image       maImgDirectionH[9];

    Image       maImgPerspective;
    Image       maImgPerspectiveH;
    Image       maImgParallel;
    Image       maImgParallelH;

    bool        mbPopupMode;

    DECL_LINK( SelectHdl, void * );
    void            FillValueSet();

    void implSetDirection( sal_Int32 nSkew, bool bEnabled = true );
    void implSetProjection( sal_Int32 nProjection, bool bEnabled = true );

protected:
    virtual BOOL    Close();
    virtual void    PopupModeEnd();

    /** This function is called when the window gets the focus.  It grabs
        the focus to the line ends value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);

public:
    ExtrusionDirectionWindow( USHORT nId, SfxBindings& rBindings );
    ~ExtrusionDirectionWindow();

    void            StartSelection();

    virtual SfxPopupWindow* Clone() const;

    virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

//========================================================================

class ExtrusionDirectionControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    ExtrusionDirectionControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~ExtrusionDirectionControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//========================================================================

class ExtrusionDepthWindow : public SfxPopupWindow
{
private:
    ToolbarMenu* mpMenu;

    Image maImgDepth0;
    Image maImgDepth1;
    Image maImgDepth2;
    Image maImgDepth3;
    Image maImgDepth4;
    Image maImgDepthInfinity;

    Image maImgDepth0h;
    Image maImgDepth1h;
    Image maImgDepth2h;
    Image maImgDepth3h;
    Image maImgDepth4h;
    Image maImgDepthInfinityh;

    SfxStatusForwarder* mpDepthForewarder;
    SfxStatusForwarder* mpMetricForewarder;

    bool        mbPopupMode;
    FieldUnit   meUnit;
    double      mfDepth;
    bool        mbEnabled;
    bool        mbInExecute;

    DECL_LINK( SelectHdl, void * );

    void    implFillStrings( FieldUnit eUnit );
    void    implSetDepth( double fDepth, bool bEnabled );

protected:
    virtual BOOL    Close();
    virtual void    PopupModeEnd();

    /** This function is called when the window gets the focus.  It grabs
        the focus to the line ends value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);

public:
    ExtrusionDepthWindow( USHORT nId, SfxBindings& rBindings );
    ~ExtrusionDepthWindow();

    void            StartSelection();

    virtual SfxPopupWindow* Clone() const;

    virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

//========================================================================

class ExtrusionDepthControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    ExtrusionDepthControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~ExtrusionDepthControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//========================================================================

class ExtrusionLightingWindow : public SfxPopupWindow
{
private:
    ToolbarMenu* mpMenu;
    ValueSet*    mpLightingSet;

    Image maImgLightingOff[9];
    Image maImgLightingOn[9];
    Image maImgLightingPreview[9];

    Image maImgLightingOffh[9];
    Image maImgLightingOnh[9];
    Image maImgLightingPreviewh[9];

    Image maImgBright;
    Image maImgNormal;
    Image maImgDim;
    Image maImgBrighth;
    Image maImgNormalh;
    Image maImgDimh;

    SfxStatusForwarder* mpLightingDirectionForewarder;
    SfxStatusForwarder* mpLightingIntensityForewarder;

    bool    mbPopupMode;
    int     mnLevel;
    bool    mbLevelEnabled;
    int     mnDirection;
    bool    mbDirectionEnabled;

    void    implSetIntensity( int nLevel, bool bEnabled );
    void    implSetDirection( int nDirection, bool bEnabled );

    DECL_LINK( SelectHdl, void * );

protected:
    virtual BOOL    Close();
    virtual void    PopupModeEnd();

    /** This function is called when the window gets the focus.  It grabs
        the focus to the line ends value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);

public:
    ExtrusionLightingWindow( USHORT nId, SfxBindings& rBindings );
    ~ExtrusionLightingWindow();

    void            StartSelection();

    virtual SfxPopupWindow* Clone() const;

    virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

//========================================================================

class ExtrusionLightingControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    ExtrusionLightingControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~ExtrusionLightingControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//========================================================================

class ExtrusionSurfaceWindow : public SfxPopupWindow
{
private:
    ToolbarMenu* mpMenu;

    Image maImgSurface1;
    Image maImgSurface2;
    Image maImgSurface3;
    Image maImgSurface4;
    Image maImgSurface1h;
    Image maImgSurface2h;
    Image maImgSurface3h;
    Image maImgSurface4h;

    SfxStatusForwarder* mpSurfaceForewarder;

    bool        mbPopupMode;

    DECL_LINK( SelectHdl, void * );

    void    implSetSurface( int nSurface, bool bEnabled );

protected:
    virtual BOOL    Close();
    virtual void    PopupModeEnd();

    /** This function is called when the window gets the focus.  It grabs
        the focus to the line ends value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);

public:
    ExtrusionSurfaceWindow( USHORT nId, SfxBindings& rBindings );
    ~ExtrusionSurfaceWindow();

    void            StartSelection();

    virtual SfxPopupWindow* Clone() const;

    virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

//========================================================================

class ExtrusionSurfaceControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    ExtrusionSurfaceControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~ExtrusionSurfaceControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//========================================================================

class ExtrusionColorControl : public SfxToolBoxControl
{
private:
    SvxTbxButtonColorUpdater_Impl*  mpBtnUpdater;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    ExtrusionColorControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~ExtrusionColorControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//========================================================================

class ExtrusionDepthDialog : public ModalDialog
{
    FixedText           maFLDepth;
    MetricField         maMtrDepth;
    OKButton            maOKButton;
    CancelButton        maCancelButton;
    HelpButton          maHelpButton;

    FieldUnit           meDefaultUnit;

public:
    ExtrusionDepthDialog( Window* pParent, double fDepth, FieldUnit eDefaultUnit );
    ~ExtrusionDepthDialog();

    double getDepth() const;
};

}
#endif


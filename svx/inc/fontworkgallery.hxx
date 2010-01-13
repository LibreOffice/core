/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fontworkgallery.hxx,v $
 * $Revision: 1.11 $
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


#ifndef _SVX_FONTWORK_GALLERY_DIALOG_HXX
#define _SVX_FONTWORK_GALLERY_DIALOG_HXX

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/valueset.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include "svx/svxdllapi.h"
#include "svx/toolbarmenu.hxx"
#include <vector>

class FmFormModel;
class SdrView;
class Window;
class SdrTextObj;
class SdrObject;
class SdrModel;

class SfxBindings;
class SfxStatusForwarder;

//------------------------------------------------------------------------

namespace svx
{

class FontWorkAlignmentWindow : public ToolbarMenu
{
    using FloatingWindow::StateChanged;
private:
    Image maImgAlgin1;
    Image maImgAlgin2;
    Image maImgAlgin3;
    Image maImgAlgin4;
    Image maImgAlgin5;
    Image maImgAlgin1h;
    Image maImgAlgin2h;
    Image maImgAlgin3h;
    Image maImgAlgin4h;
    Image maImgAlgin5h;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

    bool        mbPopupMode;

    DECL_LINK( SelectHdl, void * );

    void    implSetAlignment( int nAlignmentMode, bool bEnabled );
    void    implInit();

protected:
    virtual BOOL    Close();
    virtual void    PopupModeEnd();

    /** This function is called when the window gets the focus.  It grabs
        the focus to the line ends value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);

public:
    FontWorkAlignmentWindow( USHORT nId,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                             Window* pParentWindow );
    ~FontWorkAlignmentWindow();

    void            StartSelection();

    virtual SfxPopupWindow* Clone() const;

    virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

class SVX_DLLPUBLIC FontWorkAlignmentControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    FontWorkAlignmentControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~FontWorkAlignmentControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//------------------------------------------------------------------------

class FontWorkCharacterSpacingWindow : public ToolbarMenu
{
    using FloatingWindow::StateChanged;
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

    bool        mbPopupMode;

    DECL_LINK( SelectHdl, void * );

    void    implSetCharacterSpacing( sal_Int32 nCharacterSpacing, bool bEnabled );
    void    implSetKernCharacterPairs( sal_Bool bKernOnOff, bool bEnabled );
    void    implInit();

protected:
    virtual BOOL    Close();
    virtual void    PopupModeEnd();

    /** This function is called when the window gets the focus.  It grabs
        the focus to the line ends value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);

public:
    FontWorkCharacterSpacingWindow( USHORT nId,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
    FontWorkCharacterSpacingWindow( USHORT nId,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                            Window* pParentWindow );
    ~FontWorkCharacterSpacingWindow();

    void            StartSelection();

    virtual SfxPopupWindow* Clone() const;

    virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

class SVX_DLLPUBLIC FontWorkCharacterSpacingControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    FontWorkCharacterSpacingControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~FontWorkCharacterSpacingControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//------------------------------------------------------------------------

class SVX_DLLPUBLIC FontWorkShapeTypeControl : public SfxToolBoxControl
{
    using SfxToolBoxControl::Select;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    FontWorkShapeTypeControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~FontWorkShapeTypeControl();

    virtual void                Select( BOOL bMod1 = FALSE );
    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//------------------------------------------------------------------------

class FontworkCharacterSpacingDialog : public ModalDialog
{
    FixedText           maFLScale;
    MetricField         maMtrScale;
    OKButton            maOKButton;
    CancelButton        maCancelButton;
    HelpButton          maHelpButton;

public:
    FontworkCharacterSpacingDialog( Window* pParent, sal_Int32 nScale );
    ~FontworkCharacterSpacingDialog();

    sal_Int32 getScale() const;
};


class SVX_DLLPUBLIC FontWorkGalleryDialog : public ModalDialog
{
    ValueSet            maCtlFavorites;
    FixedLine           maFLFavorites;
    OKButton            maOKButton;
    CancelButton        maCancelButton;
    HelpButton          maHelpButton;

    sal_uInt16          mnThemeId;

    SdrView*            mpSdrView;
    FmFormModel*        mpModel;

    String              maStrClickToAddText;

    DECL_LINK( DoubleClickFavoriteHdl, void * );
    DECL_LINK( ClickOKHdl, void * );
    DECL_LINK( ClickTextDirectionHdl, ImageButton * );

    SdrObject**         mppSdrObject;
    SdrModel*           mpDestModel;

    void            initfavorites(sal_uInt16 nThemeId, std::vector< Bitmap * >& rFavorites);
    void            insertSelectedFontwork();
    void            changeText( SdrTextObj* pObj );
    void            fillFavorites( sal_uInt16 nThemeId, std::vector< Bitmap * >& rFavorites );

    std::vector< Bitmap * > maFavoritesHorizontal;

public:
    FontWorkGalleryDialog( SdrView* pView, Window* pParent, sal_uInt16 nSID );
    ~FontWorkGalleryDialog();

    // SJ: if the SdrObject** is set, the SdrObject is not inserted into the page when executing the dialog
    void SetSdrObjectRef( SdrObject**, SdrModel* pModel );
};

}

#endif

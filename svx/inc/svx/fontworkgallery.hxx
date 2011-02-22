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


#ifndef _SVX_FONTWORK_GALLERY_DIALOG_HXX
#define _SVX_FONTWORK_GALLERY_DIALOG_HXX

#include "svx/svxdllapi.h"

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>

#include <svtools/valueset.hxx>

#include <sfx2/tbxctrl.hxx>

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

class SVX_DLLPUBLIC FontWorkShapeTypeControl : public SfxToolBoxControl
{
    using SfxToolBoxControl::Select;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    FontWorkShapeTypeControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~FontWorkShapeTypeControl();

    virtual void                Select( sal_Bool bMod1 = sal_False );
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
    FixedLine           maFLFavorites;
    ValueSet            maCtlFavorites;
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

/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




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

    void            initFavorites(sal_uInt16 nThemeId);
    void            insertSelectedFontwork();
    void            changeText( SdrTextObj* pObj );
    void            fillFavorites(sal_uInt16 nThemeId);

    std::vector< BitmapEx> maFavoritesHorizontal;

public:
    FontWorkGalleryDialog( SdrView* pView, Window* pParent, sal_uInt16 nSID );
    ~FontWorkGalleryDialog();

    // SJ: if the SdrObject** is set, the SdrObject is not inserted into the page when executing the dialog
    void SetSdrObjectRef( SdrObject**, SdrModel* pModel );
};

}

#endif

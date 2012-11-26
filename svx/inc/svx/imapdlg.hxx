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




#ifndef _IMAPDLG_HXX_
#define _IMAPDLG_HXX_

#include <svtools/inettbc.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/basedlgs.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#include "svx/svxdllapi.h"


#ifndef _GOMISC_HXX
class ImageMap;
#endif


/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Float
|*
\************************************************************************/

class Graphic;
class TargetList;

class SVX_DLLPUBLIC SvxIMapDlgChildWindow : public SfxChildWindow
{
 public:

    SvxIMapDlgChildWindow( Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW( SvxIMapDlgChildWindow );

    static void UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap = NULL,
                               const TargetList* pTargetList = NULL, void* pEditingObj = NULL );
};


#ifndef _REDUCED_IMAPDLG_HXX_
#define _REDUCED_IMAPDLG_HXX_

/*************************************************************************
|*
|*
|*
\************************************************************************/

class SvxIMapDlg;

class SvxIMapDlgItem : public SfxControllerItem
{
    SvxIMapDlg& rIMap;


protected:

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                               const SfxPoolItem* pState );


public:
    SvxIMapDlgItem( sal_uInt16 nId, SvxIMapDlg& rIMapDlg, SfxBindings& rBindings );
};


/*************************************************************************
|*
|*
|*
\************************************************************************/

class IMapOwnData;
class IMapWindow;

class SVX_DLLPUBLIC SvxIMapDlg : public SfxModelessDialog // SfxFloatingWindow
{
    friend class IMapOwnData;
    friend class IMapWindow;
    using Window::Update;

    ToolBox             aTbxIMapDlg1;
    FixedText           aFtURL;
    SvtURLBox           maURLBox;
    FixedText           aFtText;
    Edit                aEdtText;
    FixedText           maFtTarget;
    ComboBox            maCbbTarget;
    StatusBar           aStbStatus;
    ImageList           maImageList;
    ImageList           maImageListH;

    Size                aLastSize;
    IMapWindow*         pIMapWnd;
    IMapOwnData*        pOwnData;
    void*               pCheckObj;
    SvxIMapDlgItem      aIMapItem;

    virtual void        Resize();
    virtual sal_Bool        Close();

#ifdef _IMAPDLG_PRIVATE

                        DECL_LINK( TbxClickHdl, ToolBox* );
                        DECL_LINK( InfoHdl, IMapWindow* );
                        DECL_LINK( MousePosHdl, IMapWindow* );
                        DECL_LINK( GraphSizeHdl, IMapWindow* );
                        DECL_LINK( URLModifyHdl, void* );
                        DECL_LINK( URLLoseFocusHdl, void* );
                        DECL_LINK( UpdateHdl, Timer* );
                        DECL_LINK( TbxUpdateHdl, Timer* );
                        DECL_LINK( StateHdl, IMapWindow* );
                        DECL_LINK( MiscHdl, void* );

    void                DoOpen();
    sal_Bool                DoSave();

#endif


public:

                        SvxIMapDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                    Window* pParent, const ResId& rResId );
                        ~SvxIMapDlg();

    void                SetExecState( sal_Bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    void                SetImageMap( const ImageMap& rImageMap );
    const ImageMap&     GetImageMap() const;

    void                SetTargetList( const TargetList& rTargetList );
    const TargetList&   GetTargetList() const;

    void                Update( const Graphic& rGraphic, const ImageMap* pImageMap = NULL,
                                const TargetList* pTargetList = NULL, void* pEditingObj = NULL );

    virtual void        KeyInput( const KeyEvent& rKEvt );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    void ApplyImageList();
};


/*************************************************************************
|*
|* Defines
|*
\************************************************************************/

#define SVXIMAPDLG() ( (SvxIMapDlg*) ( SfxViewFrame::Current()->GetChildWindow(         \
                        SvxIMapDlgChildWindow::GetChildWindowId() )->   \
                        GetWindow() ) )


#endif // _REDUCED_IMAPDLG_HXX_
#endif // _IMAPDLG_HXX_



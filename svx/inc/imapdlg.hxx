/*************************************************************************
 *
 *  $RCSfile: imapdlg.hxx,v $
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


#ifndef _IMAPDLG_HXX_
#define _IMAPDLG_HXX_

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

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

class SvxIMapDlgChildWindow : public SfxChildWindow
{
 public:

    SvxIMapDlgChildWindow( Window*, USHORT, SfxBindings*, SfxChildWinInfo* );

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

    virtual void StateChanged( USHORT nSID, SfxItemState eState,
                               const SfxPoolItem* pState );


public:

    SvxIMapDlgItem( USHORT nId, SvxIMapDlg& rIMapDlg, SfxBindings& rBindings );
};


/*************************************************************************
|*
|*
|*
\************************************************************************/

class IMapOwnData;

class SvxIMapDlg : public SfxFloatingWindow
{
    friend class IMapOwnData;
    friend class IMapWindow;

    Size                aLastSize;
    IMapWindow*         pIMapWnd;
    IMapOwnData*        pOwnData;
    void*               pCheckObj;
    SvxIMapDlgItem      aIMapItem;

    ToolBox             aTbxIMapDlg1;
    StatusBar           aStbStatus;
    FixedText           aFtURL;
    FixedText           aFtText;
    ComboBox            aCbbURL;
    Edit                aEdtText;

    virtual void        Resize();
    virtual BOOL        Close();

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

    void                DoOpen();
    BOOL                DoSave();

#endif


public:

                        SvxIMapDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                    Window* pParent, const ResId& rResId );
                        ~SvxIMapDlg();

    void                SetExecState( BOOL bEnable );

    void                SetGraphic( const Graphic& rGraphic );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    void                SetImageMap( const ImageMap& rImageMap );
    const ImageMap&     GetImageMap() const;

    void                SetTargetList( const TargetList& rTargetList );
    const TargetList&   GetTargetList() const;

    void                Update( const Graphic& rGraphic, const ImageMap* pImageMap = NULL,
                                const TargetList* pTargetList = NULL, void* pEditingObj = NULL );
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



/*************************************************************************
 *
 *  $RCSfile: imapwnd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-15 17:21:24 $
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
#ifndef _IMAPWND_HXX
#define _IMAPWND_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX //autogen
#include <svtools/imapobj.hxx>
#endif
#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif

#include "graphctl.hxx"

/*************************************************************************
|*
|*
|*
\************************************************************************/

struct NotifyInfo
{
    String  aMarkURL;
    String  aMarkDescription;
    String  aMarkTarget;
    BOOL    bNewObj;
    BOOL    bOneMarked;
    BOOL    bActivated;
};


struct NotifyPosSize
{
    Size    aPictureSize;
    Point   aMousePos;
    BOOL    bPictureSize;
    BOOL    bMousePos;
};


/*************************************************************************
|*
|*
|*
\************************************************************************/

#define SVD_IMAP_USERDATA   0x0001

const UINT32 IMapInventor = UINT32('I') * 0x00000001+
                            UINT32('M') * 0x00000100+
                            UINT32('A') * 0x00010000+
                            UINT32('P') * 0x01000000;


class IMapUserData : public SdrObjUserData
{
    IMapObject* pObj;

public:

                            IMapUserData() :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                pObj            ( NULL ) {}

                            IMapUserData( IMapObject* pIMapObj ) :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                pObj            ( pIMapObj ) {}

                            IMapUserData( const IMapUserData& rIMapUserData ) :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                pObj            ( rIMapUserData.pObj ) {}

                            ~IMapUserData() { delete pObj; }

    virtual SdrObjUserData* Clone( SdrObject *pObj ) const { return new IMapUserData( *this ); }

    void                    SetObject( IMapObject* pIMapObj ) { pObj = pIMapObj; }
    IMapObject*             GetObject() const { return pObj; }
    void                    ReplaceObject( IMapObject* pNewIMapObject ) { delete pObj; pObj = pNewIMapObject; }
};


/*************************************************************************
|*
|*
|*
\************************************************************************/

class URLDlg : public ModalDialog
{
    GroupBox            aGrpURL;
    Edit                aEdtURL;
    Edit                aEdtURLDescription;
    ComboBox            aCbbTargets;
    Edit                aEdtName;
    FixedText           aFtURL1;
    FixedText           aFtURLDescription;
    FixedText           aFtTarget;
    FixedText           aFtName;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;

public:

                        URLDlg( Window* pWindow,
                                const String& rURL, const String& rDescription,
                                const String& rTarget, const String& rName,
                                TargetList& rTargetList );

    String              GetURL() const { return aEdtURL.GetText(); }
    String              GetDescription() const { return aEdtURLDescription.GetText(); }
    String              GetTarget() const { return aCbbTargets.GetText(); }
    String              GetName() const { return aEdtName.GetText(); }
};


/*************************************************************************
|*
|*
|*
\************************************************************************/

class IMapWindow : public GraphCtrl, public DropTargetHelper
{
    NotifyInfo          aInfo;
    ImageMap            aIMap;
    TargetList          aTargetList;
    Link                aInfoLink;
    SfxItemPool*        pIMapPool;
    SfxItemInfo*        pItemInfo;

                        DECL_LINK( MenuSelectHdl, Menu* );
                        DECL_LINK( MouseTimerHdl, Timer* );

protected:

    // GraphCtrl
    virtual void        MouseButtonUp(const MouseEvent& rMEvt);
    virtual void        Command(const CommandEvent& rCEvt);
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        SdrObjCreated( const SdrObject& rObj );
    virtual void        SdrObjChanged( const SdrObject& rObj );
    virtual void        MarkListHasChanged();
    virtual void        InitSdrModel();

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    void                CreateImageMap();
    void                ReplaceImageMap( const ImageMap& rNewImageMap, BOOL bScaleToGraphic );

    void                ClearTargetList();

    SdrObject*          CreateObj( const IMapObject* pIMapObj );
    IMapObject*         GetIMapObj( const SdrObject* pSdrObj ) const;
    SdrObject*          GetSdrObj( const IMapObject* pIMapObj ) const;
    SdrObject*          GetHitSdrObj( const Point& rPosPixel ) const;

    void                UpdateInfo( BOOL bNewObj );

public:

                        IMapWindow( Window* pParent, WinBits nWinBits = 0 );
                        IMapWindow( Window* pParent, const ResId& rResId );
                        ~IMapWindow();

    BOOL                ReplaceActualIMapInfo( const NotifyInfo& rNewInfo );

    void                SetImageMap( const ImageMap& rImageMap );
    const ImageMap&     GetImageMap();

    void                SetCurrentObjState( BOOL bActive );
    void                DoMacroAssign();
    void                DoPropertyDialog();

    void                SetInfoLink( const Link& rLink ) { aInfoLink = rLink; }
    const Link&         GetInfoLink() const { return aInfoLink; }

    void                SetTargetList( TargetList& rTargetList );
    const TargetList&   GetTargetList() const { return aTargetList; }

    const NotifyInfo&   GetInfo() const { return aInfo; }
};


#endif


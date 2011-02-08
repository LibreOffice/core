/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _IMAPWND_HXX
#define _IMAPWND_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <svl/itempool.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/transfer.hxx>
#include <svtools/imap.hxx>
#include <sfx2/frame.hxx>
#include <svx/graphctl.hxx>

#include <com/sun/star/frame/XFrame.hpp>

/*************************************************************************
|*
|*
|*
\************************************************************************/

struct NotifyInfo
{
    String  aMarkURL;
    String  aMarkAltText;
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


typedef boost::shared_ptr< IMapObject > IMapObjectPtr;

class IMapUserData : public SdrObjUserData
{
    // #i98386# use boost::shared_ptr here due to cloning possibilities
    IMapObjectPtr           mpObj;

public:

                            IMapUserData() :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                mpObj           ( ) {}

                            IMapUserData( const IMapObjectPtr& rIMapObj ) :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                mpObj           ( rIMapObj ) {}

                            IMapUserData( const IMapUserData& rIMapUserData ) :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                mpObj           ( rIMapUserData.mpObj ) {}

                            ~IMapUserData() { }

    virtual SdrObjUserData* Clone( SdrObject * ) const { return new IMapUserData( *this ); }

    void                    SetObject( const IMapObjectPtr& rIMapObj ) { mpObj = rIMapObj; }
    const IMapObjectPtr     GetObject() const { return mpObj; }
    void                    ReplaceObject( const IMapObjectPtr& pNewIMapObject ) { mpObj = pNewIMapObject; }
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
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                        mxDocumentFrame;

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

                        IMapWindow( Window* pParent, const ResId& rResId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame );
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

    void                CreateDefaultObject();
    void                SelectFirstObject();
    void                StartPolyEdit();

    virtual void        KeyInput( const KeyEvent& rKEvt );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

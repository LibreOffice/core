/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

struct NotifyInfo
{
    OUString    aMarkURL;
    OUString    aMarkAltText;
    OUString    aMarkTarget;
    sal_Bool    bNewObj;
    sal_Bool    bOneMarked;
    sal_Bool    bActivated;
};


struct NotifyPosSize
{
    Size    aPictureSize;
    Point   aMousePos;
    sal_Bool    bPictureSize;
    sal_Bool    bMousePos;
};

#define SVD_IMAP_USERDATA   0x0001

const sal_uInt32 IMapInventor = sal_uInt32('I') * 0x00000001+
                            sal_uInt32('M') * 0x00000100+
                            sal_uInt32('A') * 0x00010000+
                            sal_uInt32('P') * 0x01000000;


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
    void                ReplaceImageMap( const ImageMap& rNewImageMap, sal_Bool bScaleToGraphic );

    void                ClearTargetList();

    SdrObject*          CreateObj( const IMapObject* pIMapObj );
    IMapObject*         GetIMapObj( const SdrObject* pSdrObj ) const;
    SdrObject*          GetHitSdrObj( const Point& rPosPixel ) const;

    void                UpdateInfo( sal_Bool bNewObj );

public:

                        IMapWindow( Window* pParent, const ResId& rResId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame );
                        ~IMapWindow();

    sal_Bool                ReplaceActualIMapInfo( const NotifyInfo& rNewInfo );

    void                SetImageMap( const ImageMap& rImageMap );
    const ImageMap&     GetImageMap();

    void                SetCurrentObjState( sal_Bool bActive );
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

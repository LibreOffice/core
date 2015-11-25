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
#ifndef INCLUDED_SVX_SOURCE_DIALOG_IMAPWND_HXX
#define INCLUDED_SVX_SOURCE_DIALOG_IMAPWND_HXX

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
    bool    bNewObj;
    bool    bOneMarked;
    bool    bActivated;
};


#define SVD_IMAP_USERDATA   0x0001

const sal_uInt32 IMapInventor = sal_uInt32('I') * 0x00000001+
                            sal_uInt32('M') * 0x00000100+
                            sal_uInt32('A') * 0x00010000+
                            sal_uInt32('P') * 0x01000000;


typedef std::shared_ptr< IMapObject > IMapObjectPtr;

class IMapUserData : public SdrObjUserData
{
    // #i98386# use std::shared_ptr here due to cloning possibilities
    IMapObjectPtr           mpObj;

public:

                            IMapUserData() :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                mpObj           ( ) {}

                   explicit IMapUserData( const IMapObjectPtr& rIMapObj ) :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                mpObj           ( rIMapObj ) {}

                            IMapUserData( const IMapUserData& rIMapUserData ) :
                                SdrObjUserData  ( IMapInventor, SVD_IMAP_USERDATA, 0 ),
                                mpObj           ( rIMapUserData.mpObj ) {}

                            virtual ~IMapUserData() { }

    virtual SdrObjUserData* Clone( SdrObject * ) const override { return new IMapUserData( *this ); }

    const IMapObjectPtr     GetObject() const { return mpObj; }
    void                    ReplaceObject( const IMapObjectPtr& pNewIMapObject ) { mpObj = pNewIMapObject; }
};

class IMapWindow : public GraphCtrl, public DropTargetHelper
{
    NotifyInfo          aInfo;
    ImageMap            aIMap;
    TargetList          aTargetList;
    Link<IMapWindow&,void> aInfoLink;
    SfxItemPool*        pIMapPool;
    SfxItemInfo*        pItemInfo;
    css::uno::Reference< css::frame::XFrame >
                        mxDocumentFrame;

                        DECL_LINK_TYPED( MenuSelectHdl, Menu*, bool );

protected:

    // GraphCtrl
    virtual void        MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual Size        GetOptimalSize() const override;
    virtual void        Command(const CommandEvent& rCEvt) override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void        SdrObjCreated( const SdrObject& rObj ) override;
    virtual void        SdrObjChanged( const SdrObject& rObj ) override;
    virtual void        MarkListHasChanged() override;
    virtual void        InitSdrModel() override;

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    void                ReplaceImageMap( const ImageMap& rNewImageMap, bool bScaleToGraphic );

    SdrObject*          CreateObj( const IMapObject* pIMapObj );
    static IMapObject*  GetIMapObj( const SdrObject* pSdrObj );
    SdrObject*          GetHitSdrObj( const Point& rPosPixel ) const;

    void                UpdateInfo( bool bNewObj );

public:

                        IMapWindow( vcl::Window* pParent, WinBits nBits, const css::uno::Reference< css::frame::XFrame >& rxDocumentFrame );
                        virtual ~IMapWindow();
    virtual void        dispose() override;

    bool                ReplaceActualIMapInfo( const NotifyInfo& rNewInfo );

    void                SetImageMap( const ImageMap& rImageMap );
    const ImageMap&     GetImageMap();

    void                SetCurrentObjState( bool bActive );
    void                DoMacroAssign();
    void                DoPropertyDialog();

    void                SetInfoLink( const Link<IMapWindow&,void>& rLink ) { aInfoLink = rLink; }

    void                SetTargetList( TargetList& rTargetList );

    const NotifyInfo&   GetInfo() const { return aInfo; }

    void                CreateDefaultObject();
    void                SelectFirstObject();
    void                StartPolyEdit();

    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

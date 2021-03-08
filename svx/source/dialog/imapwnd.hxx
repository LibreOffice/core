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

#include <vcl/imapobj.hxx>
#include <vcl/transfer.hxx>
#include <vcl/imap.hxx>
#include <sfx2/frame.hxx>
#include <svx/graphctl.hxx>
#include <svl/itempool.hxx>

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

typedef std::shared_ptr< IMapObject > IMapObjectPtr;

class IMapUserData : public SdrObjUserData
{
    // #i98386# use std::shared_ptr here due to cloning possibilities
    IMapObjectPtr           mpObj;

public:

                   explicit IMapUserData( const IMapObjectPtr& rIMapObj ) :
                                SdrObjUserData  ( SdrInventor::IMap, SVD_IMAP_USERDATA ),
                                mpObj           ( rIMapObj ) {}

                            IMapUserData( const IMapUserData& rIMapUserData ) :
                                SdrObjUserData  ( SdrInventor::IMap, SVD_IMAP_USERDATA ),
                                mpObj           ( rIMapUserData.mpObj ) {}

    virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject * ) const override { return std::unique_ptr<SdrObjUserData>(new IMapUserData( *this )); }

    const IMapObjectPtr&    GetObject() const { return mpObj; }
    void                    ReplaceObject( const IMapObjectPtr& pNewIMapObject ) { mpObj = pNewIMapObject; }
};

class IMapWindow;

class IMapDropTargetHelper final : public DropTargetHelper
{
    IMapWindow& m_rImapWindow;
public:
    IMapDropTargetHelper(IMapWindow& rImapWindow);

    // DropTargetHelper
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
};

class IMapWindow final : public GraphCtrl
{
    NotifyInfo          aInfo;
    ImageMap            aIMap;
    TargetList          aTargetList;
    Link<IMapWindow&,void> aInfoLink;
    SfxItemPool*        pIMapPool;
    SfxItemInfo         maItemInfos[1] = {};
    css::uno::Reference< css::frame::XFrame >
                        mxDocumentFrame;
    std::unique_ptr<IMapDropTargetHelper> mxDropTargetHelper;
    std::unique_ptr<weld::Menu> mxPopupMenu;

    void                MenuSelectHdl(const OString& rId);

    // GraphCtrl
    virtual bool        MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void        SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual bool        Command(const CommandEvent& rCEvt) override;
    virtual OUString    RequestHelp(tools::Rectangle& rHelpArea) override;
    virtual void        SdrObjCreated( const SdrObject& rObj ) override;
    virtual void        SdrObjChanged( const SdrObject& rObj ) override;
    virtual void        MarkListHasChanged() override;
    virtual void        InitSdrModel() override;

    void                ReplaceImageMap( const ImageMap& rNewImageMap );

    rtl::Reference<SdrObject> CreateObj( const IMapObject* pIMapObj );
    static IMapObject*  GetIMapObj( const SdrObject* pSdrObj );
    SdrObject*          GetHitSdrObj( const Point& rPosPixel ) const;

    void                UpdateInfo( bool bNewObj );

public:

    IMapWindow(const css::uno::Reference< css::frame::XFrame >& rxDocumentFrame,
               weld::Dialog* pDialog);
    virtual ~IMapWindow() override;

    sal_Int8            AcceptDrop( const AcceptDropEvent& rEvt );
    sal_Int8            ExecuteDrop( const ExecuteDropEvent& rEvt );

    void                ReplaceActualIMapInfo( const NotifyInfo& rNewInfo );

    void                SetImageMap( const ImageMap& rImageMap );
    const ImageMap&     GetImageMap();

    void                SetCurrentObjState( bool bActive );
    void                DoMacroAssign();
    void                DoPropertyDialog();

    void                SetInfoLink( const Link<IMapWindow&,void>& rLink ) { aInfoLink = rLink; }

    void                SetTargetList( TargetList& rTargetList );

    const NotifyInfo&   GetInfo() const { return aInfo; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

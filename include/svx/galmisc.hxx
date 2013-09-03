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

#ifndef _SVX_GALMISC_HXX_
#define _SVX_GALMISC_HXX_

#include <sot/storage.hxx>
#include <tools/urlobj.hxx>
#include <svtools/imap.hxx>
#include <svl/hint.hxx>
#include <svtools/transfer.hxx>
#include <svx/svdobj.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/XProgressMonitor.hpp>
#include "svx/svxdllapi.h"
#include <tools/date.hxx>
#include <tools/time.hxx>

class GalleryTheme;

struct ExchangeData
{
    GalleryTheme*   pTheme;
    OUString        aEditedTitle;
    Date            aThemeChangeDate;
    Time            aThemeChangeTime;

    ExchangeData() :
        aThemeChangeDate( Date::EMPTY ),
        aThemeChangeTime( Time::EMPTY )
        {}
};

enum SgaObjKind
{
    SGA_OBJ_NONE = 0,   // abstract object
    SGA_OBJ_BMP = 1,    // bitmap object
    SGA_OBJ_SOUND = 2,  // sound object
    SGA_OBJ_VIDEO = 3,  // video object
    SGA_OBJ_ANIM = 4,   // animation object
    SGA_OBJ_SVDRAW = 5, // Svdraw object
    SGA_OBJ_INET = 6    // graphics from the internet
};

#define IV_IMAPINFO             (sal_uInt32('S')*0x00000001+sal_uInt32('D')*0x00000100+sal_uInt32('U')*0x00010000+sal_uInt32('D')*0x01000000)
#define ID_IMAPINFO             2

#define USERDATA_HDL()          (LINK(this,SgaUserDataFactory,MakeUserData))

#define GAL_RES(nId)            ResId(nId, *GetGalleryResMgr())
#define GAL_RESSTR(nId)         GAL_RES(nId).toString()
#define STREAMBUF_SIZE          16384L

#define SGA_IMPORT_NONE         0x0000
#define SGA_IMPORT_FILE         0x0001
#define SGA_IMPORT_INET         0x0002

#define GALLERY_PROGRESS_RANGE  10000

#define GALLERY_FG_COLOR        Application::GetSettings().GetStyleSettings().GetWindowTextColor()
#define GALLERY_BG_COLOR        Application::GetSettings().GetStyleSettings().GetWindowColor()
#define GALLERY_DLG_COLOR       Application::GetSettings().GetStyleSettings().GetDialogColor()

class ResMgr;
class SvStream;
class Graphic;
class FmFormModel;
class ImageMap;
class Gallery;

SVX_DLLPUBLIC ResMgr*           GetGalleryResMgr();
sal_uInt16          GalleryGraphicImport( const INetURLObject& rURL, Graphic& rGraphic, OUString& rFilterName, sal_Bool bShowProgress = sal_False );
sal_Bool            GallerySvDrawImport( SvStream& rIStm, SdrModel& rModel );
sal_Bool            CreateIMapGraphic( const FmFormModel& rModel, Graphic& rGraphic, ImageMap& rImageMap );
SVX_DLLPUBLIC OUString            GetReducedString( const INetURLObject& rURL, sal_Int32 nMaxLen );
String          GetSvDrawStreamNameFromURL( const INetURLObject& rSvDrawObjURL );

sal_Bool            FileExists( const INetURLObject& rURL );
sal_Bool            CreateDir(  const INetURLObject& rURL );
sal_Bool            CopyFile(  const INetURLObject& rSrcURL, const INetURLObject& rDstURL );
sal_Bool            KillFile( const INetURLObject& rURL );
BitmapEx        GalleryResGetBitmapEx( sal_uInt32 nId );

class SgaIMapInfo : public SdrObjUserData, public SfxListener
{
    ImageMap                aImageMap;

public:
                            SgaIMapInfo() : SdrObjUserData( IV_IMAPINFO, ID_IMAPINFO, 0 ) {};

                            SgaIMapInfo( const ImageMap& rImageMap) :
                                SdrObjUserData( IV_IMAPINFO, ID_IMAPINFO, 0 ),
                                aImageMap( rImageMap ) {};

    virtual                 ~SgaIMapInfo() {};

    virtual SdrObjUserData* Clone( SdrObject* ) const
                            {
                                SgaIMapInfo* pInfo = new SgaIMapInfo;
                                pInfo->aImageMap = aImageMap;
                                return pInfo;
                            }

    const ImageMap&         GetImageMap() const { return aImageMap; }
};

class SgaUserDataFactory
{
public:
        SgaUserDataFactory() { SdrObjFactory::InsertMakeUserDataHdl( USERDATA_HDL() ); }
        ~SgaUserDataFactory() { SdrObjFactory::RemoveMakeUserDataHdl( USERDATA_HDL() ); }

        DECL_LINK( MakeUserData, SdrObjFactory* );
};

class GraphicFilter;

class SVX_DLLPUBLIC GalleryProgress
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XProgressBar > mxProgressBar;
    GraphicFilter*                                                          mpFilter;

    public:

                                    GalleryProgress( GraphicFilter* pFilter = NULL );
                                    ~GalleryProgress();

    void                            Update( sal_uIntPtr nVal, sal_uIntPtr nMaxVal );
};

class Gallery;
class GalleryTheme;
class GraphicObject;

class GalleryTransferable : public TransferableHelper
{
friend class GalleryTheme;
using TransferableHelper::CopyToClipboard;

private:

    GalleryTheme*                   mpTheme;
    SgaObjKind                      meObjectKind;
    sal_uInt32                      mnObjectPos;
    SotStorageStreamRef             mxModelStream;
    GraphicObject*                  mpGraphicObject;
    ImageMap*                       mpImageMap;
    INetURLObject*                  mpURL;

protected:

                                    GalleryTransferable( GalleryTheme* pTheme, sal_uIntPtr nObjectPos, bool bLazy );
                                    ~GalleryTransferable();

    void                            InitData( bool bLazy );

    // TransferableHelper
    virtual void                    AddSupportedFormats();
    virtual sal_Bool                GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool                WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void                    DragFinished( sal_Int8 nDropAction );
    virtual void                    ObjectReleased();

    void                            CopyToClipboard( Window* pWindow );
    void                            StartDrag( Window* pWindow, sal_Int8 nDragSourceActions,
                                               sal_Int32 nDragPointer = DND_POINTER_NONE,
                                               sal_Int32 nDragImage = DND_IMAGE_NONE );
};

#define GALLERY_HINT_NONE               0x00000000
#define GALLERY_HINT_CLOSE_THEME        0x00000001
#define GALLERY_HINT_THEME_REMOVED      0x00000002
#define GALLERY_HINT_THEME_RENAMED      0x00000004
#define GALLERY_HINT_THEME_CREATED      0x00000008
#define GALLERY_HINT_THEME_UPDATEVIEW   0x00000010
#define GALLERY_HINT_CLOSE_OBJECT       0x00000020
#define GALLERY_HINT_OBJECT_REMOVED     0x00000040

class GalleryHint : public SfxHint
{
private:

    sal_uIntPtr         mnType;
    OUString       maThemeName;
    OUString       maStringData;
    sal_uIntPtr         mnData1;
    sal_uIntPtr         mnData2;

public:

                    GalleryHint( sal_uIntPtr nType, const OUString& rThemeName, sal_uIntPtr nData1 = 0UL, sal_uIntPtr nData2 = 0UL ) :
                        mnType( nType ), maThemeName( rThemeName ), mnData1( nData1 ), mnData2( nData2 ) {}

                    GalleryHint( sal_uIntPtr nType, const OUString& rThemeName, const OUString& rStringData, sal_uIntPtr nData1 = 0UL, sal_uIntPtr nData2 = 0UL ) :
                        mnType( nType ), maThemeName( rThemeName ), maStringData( rStringData ), mnData1( nData1 ), mnData2( nData2 ) {}

    sal_uIntPtr          GetType() const { return mnType; }
    const OUString& GetThemeName() const { return maThemeName; }
    const OUString& GetStringData() const { return maStringData; }
    sal_uIntPtr          GetData1() const { return mnData1; }
    sal_uIntPtr          GetData2() const { return mnData2; }
};

    #endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

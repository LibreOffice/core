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

#ifndef INCLUDED_SVX_GALMISC_HXX
#define INCLUDED_SVX_GALMISC_HXX

#include <sot/formats.hxx>
#include <tools/urlobj.hxx>
#include <svtools/imap.hxx>
#include <svl/hint.hxx>
#include <svtools/transfer.hxx>
#include <svx/svdobj.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/XProgressMonitor.hpp>
#include <svx/svxdllapi.h>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/resid.hxx>

class GalleryTheme;
class SotStorageStream;

struct ExchangeData
{
    GalleryTheme*   pTheme;
    OUString        aEditedTitle;
    Date            aThemeChangeDate;
    tools::Time     aThemeChangeTime;

    ExchangeData()
        : pTheme(nullptr)
        , aThemeChangeDate( Date::EMPTY )
        , aThemeChangeTime( tools::Time::EMPTY )
        {
        }
};

enum class SgaObjKind
{
    NONE = 0,   // abstract object
    Bitmap = 1,    // bitmap object
    Sound = 2,  // sound object
    Animation = 4,   // animation object
    SvDraw = 5, // Svdraw object
    Inet = 6    // graphics from the internet
};

#define ID_IMAPINFO             2

#define STREAMBUF_SIZE          16384L

enum class GalleryGraphicImportRet
{
    IMPORT_NONE, IMPORT_FILE
};

#define GALLERY_PROGRESS_RANGE  10000

#define GALLERY_FG_COLOR        Application::GetSettings().GetStyleSettings().GetWindowTextColor()
#define GALLERY_BG_COLOR        Application::GetSettings().GetStyleSettings().GetWindowColor()
#define GALLERY_DLG_COLOR       Application::GetSettings().GetStyleSettings().GetDialogColor()

class SvStream;
class Graphic;
class FmFormModel;
class ImageMap;
class Gallery;

GalleryGraphicImportRet  GalleryGraphicImport( const INetURLObject& rURL, Graphic& rGraphic, OUString& rFilterName, bool bShowProgress = false );
bool                GallerySvDrawImport( SvStream& rIStm, SdrModel& rModel );
bool                CreateIMapGraphic( const FmFormModel& rModel, Graphic& rGraphic, ImageMap& rImageMap );
SVX_DLLPUBLIC OUString
                    GetReducedString( const INetURLObject& rURL, sal_Int32 nMaxLen );
OUString            GetSvDrawStreamNameFromURL( const INetURLObject& rSvDrawObjURL );

bool                FileExists( const INetURLObject& rURL );
bool                CreateDir(  const INetURLObject& rURL );
bool                CopyFile(  const INetURLObject& rSrcURL, const INetURLObject& rDstURL );
bool                KillFile( const INetURLObject& rURL );
BitmapEx            GalleryResGetBitmapEx(const OUString& rId);

class SgaIMapInfo : public SdrObjUserData, public SfxListener
{
    ImageMap                aImageMap;

public:
                            SgaIMapInfo() : SdrObjUserData( SdrInventor::SgaImap, ID_IMAPINFO ) {};

                            SgaIMapInfo( const ImageMap& rImageMap) :
                                SdrObjUserData( SdrInventor::SgaImap, ID_IMAPINFO ),
                                aImageMap( rImageMap ) {};

    virtual SdrObjUserData* Clone( SdrObject* ) const override
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
    SgaUserDataFactory() { SdrObjFactory::InsertMakeUserDataHdl( LINK(this,SgaUserDataFactory,MakeUserData) ); }
    ~SgaUserDataFactory() { SdrObjFactory::RemoveMakeUserDataHdl( LINK(this,SgaUserDataFactory,MakeUserData) ); }

    DECL_STATIC_LINK( SgaUserDataFactory, MakeUserData, SdrObjUserDataCreatorParams, SdrObjUserData* );
};

class GraphicFilter;

class SVX_DLLPUBLIC GalleryProgress
{
    css::uno::Reference< css::awt::XProgressBar > mxProgressBar;
    GraphicFilter*                                                          mpFilter;

    public:

                                    GalleryProgress( GraphicFilter* pFilter = nullptr );
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
    tools::SvRef<SotStorageStream>             mxModelStream;
    GraphicObject*                  mpGraphicObject;
    ImageMap*                       mpImageMap;
    INetURLObject*                  mpURL;

protected:

                                    GalleryTransferable( GalleryTheme* pTheme, sal_uIntPtr nObjectPos, bool bLazy );
                                    virtual ~GalleryTransferable() override;

    void                            InitData( bool bLazy );

    // TransferableHelper
    virtual void                    AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool                    WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, SotClipboardFormatId nUserObjectId, const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual void                    DragFinished( sal_Int8 nDropAction ) override;
    virtual void                    ObjectReleased() override;

    void                            StartDrag( vcl::Window* pWindow, sal_Int8 nDragSourceActions );
};

enum class GalleryHintType
{
    CLOSE_THEME,
    THEME_REMOVED,
    THEME_RENAMED,
    THEME_CREATED,
    THEME_UPDATEVIEW,
    CLOSE_OBJECT,
    OBJECT_REMOVED,
};

class GalleryHint : public SfxHint
{
private:

    GalleryHintType     mnType;
    OUString            maThemeName;
    OUString            maStringData;
    sal_uIntPtr         mnData1;

public:

                     GalleryHint( GalleryHintType nType, const OUString& rThemeName, sal_uIntPtr nData1 = 0UL ) :
                        mnType( nType ), maThemeName( rThemeName ), mnData1( nData1 ) {}

                     GalleryHint( GalleryHintType nType, const OUString& rThemeName, const OUString& rStringData, sal_uIntPtr nData1 = 0UL ) :
                        mnType( nType ), maThemeName( rThemeName ), maStringData( rStringData ), mnData1( nData1 ) {}

    GalleryHintType  GetType() const { return mnType; }
    const OUString&  GetThemeName() const { return maThemeName; }
    const OUString&  GetStringData() const { return maStringData; }
    sal_uIntPtr      GetData1() const { return mnData1; }
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

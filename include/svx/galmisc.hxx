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

#include <vcl/imap.hxx>
#include <svl/hint.hxx>
#include <vcl/transfer.hxx>
#include <svx/svdobj.hxx>
#include <com/sun/star/uno/Reference.h>
#include <svx/svxdllapi.h>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <memory>

namespace com::sun::star::awt { class XProgressBar; }

class INetURLObject;
class GalleryTheme;
class SotStorageStream;
class SotTempStream;

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

GalleryGraphicImportRet  GalleryGraphicImport( const INetURLObject& rURL, Graphic& rGraphic, OUString& rFilterName );
bool                GallerySvDrawImport( SvStream& rIStm, SdrModel& rModel );
bool                CreateIMapGraphic( const FmFormModel& rModel, Graphic& rGraphic, ImageMap& rImageMap );
SVXCORE_DLLPUBLIC OUString
                    GetReducedString( const INetURLObject& rURL, sal_Int32 nMaxLen );
OUString            GetSvDrawStreamNameFromURL( const INetURLObject& rSvDrawObjURL );

bool                FileExists( const INetURLObject& rURL );
bool                CreateDir(  const INetURLObject& rURL );
bool                CopyFile(  const INetURLObject& rSrcURL, const INetURLObject& rDstURL );
bool                KillFile( const INetURLObject& rURL );

class SgaIMapInfo final : public SdrObjUserData, public SfxListener
{
    ImageMap                aImageMap;

public:
                            SgaIMapInfo() : SdrObjUserData( SdrInventor::SgaImap, ID_IMAPINFO ) {};

                            SgaIMapInfo( const ImageMap& rImageMap) :
                                SdrObjUserData( SdrInventor::SgaImap, ID_IMAPINFO ),
                                aImageMap( rImageMap ) {};

    virtual std::unique_ptr<SdrObjUserData> Clone( SdrObject* ) const override
                            {
                                SgaIMapInfo* pInfo = new SgaIMapInfo;
                                pInfo->aImageMap = aImageMap;
                                return std::unique_ptr<SdrObjUserData>(pInfo);
                            }

    const ImageMap&         GetImageMap() const { return aImageMap; }
};

class GraphicFilter;

class SVXCORE_DLLPUBLIC GalleryProgress
{
    css::uno::Reference< css::awt::XProgressBar > mxProgressBar;

    public:

                                    GalleryProgress( const GraphicFilter* pFilter = nullptr );
                                    ~GalleryProgress();

    void                            Update( sal_Int32 nVal, sal_Int32 nMaxVal );
};

class GalleryTheme;
class GraphicObject;

class GalleryTransferable final : public TransferDataContainer
{
friend class GalleryTheme;
using TransferableHelper::CopyToClipboard;

    GalleryTheme*                   mpTheme;
    SgaObjKind                      meObjectKind;
    sal_uInt32                      mnObjectPos;
    tools::SvRef<SotTempStream>  mxModelStream;
    std::unique_ptr<GraphicObject>  mpGraphicObject;
    std::unique_ptr<INetURLObject>  mpURL;

    void                            InitData( bool bLazy );

public:
                                    GalleryTransferable( GalleryTheme* pTheme, sal_uInt32 nObjectPos, bool bLazy );
                                    virtual ~GalleryTransferable() override;

    void                            SelectObject(sal_uInt32 nObjectPos);

    // TransferableHelper
    virtual void                    AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool                    WriteObject( tools::SvRef<SotTempStream>& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual void                    DragFinished( sal_Int8 nDropAction ) override;
    virtual void                    ObjectReleased() override;

    bool                            StartDrag();
};

enum class GalleryHintType
{
    CLOSE_THEME,
    THEME_REMOVED,
    THEME_RENAMED,
    THEME_CREATED,
    THEME_UPDATEVIEW,
    CLOSE_OBJECT
};

class GalleryHint final : public SfxHint
{
private:

    GalleryHintType     mnType;
    OUString            maThemeName;
    OUString            maStringData;
    void*               mnData1;

public:

                     GalleryHint( GalleryHintType nType, const OUString& rThemeName, void* nData1 = nullptr ) :
                        mnType( nType ), maThemeName( rThemeName ), mnData1( nData1 ) {}

                     GalleryHint( GalleryHintType nType, const OUString& rThemeName, const OUString& rStringData ) :
                        mnType( nType ), maThemeName( rThemeName ), maStringData( rStringData ), mnData1( nullptr ) {}

    GalleryHintType  GetType() const { return mnType; }
    const OUString&  GetThemeName() const { return maThemeName; }
    const OUString&  GetStringData() const { return maStringData; }
    void*            GetData1() const { return mnData1; }
};

struct GalleryObject;

INetURLObject ImplGetURL(const GalleryObject* pObject);

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_SVX_GALTHEME_HXX
#define INCLUDED_SVX_GALTHEME_HXX

#include <svx/svxdllapi.h>

#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/salctype.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/lstner.hxx>
#include <svtools/transfer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/galmisc.hxx>
#include <vector>

class SotStorage;
class SotStorageStream;

// - SgaObjKind -


struct GalleryObject
{
    INetURLObject   aURL;
    sal_uInt32      nOffset;
    SgaObjKind      eObjKind;
    bool            mbDelete;

    //UI visualization buffering
    BitmapEx        maPreviewBitmapEx;
    Size            maPreparedSize;
    OUString        maTitle;
    OUString        maPath;
};

typedef ::std::vector< GalleryObject* > GalleryObjectList;

class GalleryThemeEntry;
class SgaObject;
class FmFormModel;
class ListBox;


// - GalDragParams -


struct GalDragParams
{
    vcl::Region  aDragRegion;
    sal_uIntPtr nDragObjPos;
    OUString aThemeName;
    OUString aFileName;
    SgaObjKind  eObjKind;
};


// - GalleryTheme -


class Gallery;
class GalleryProgress;
namespace unogallery
{
    class GalleryTheme;
    class GalleryItem;
}

class SVX_DLLPUBLIC GalleryTheme : public SfxBroadcaster
{
    friend class Gallery;
    friend class GalleryThemeCacheEntry;
    friend class ::unogallery::GalleryTheme;
    friend class ::unogallery::GalleryItem;

private:

    GalleryObjectList           aObjectList;
    OUString                    m_aDestDir;
    bool                        m_bDestDirRelative;
    tools::SvRef<SotStorage>               aSvDrawStorageRef;
    Gallery*                    pParent;
    GalleryThemeEntry*          pThm;
    sal_uIntPtr                 mnThemeLockCount;
    sal_uIntPtr                 mnBroadcasterLockCount;
    sal_uIntPtr                 nDragPos;
    bool                        bDragging;
    bool                        bAbortActualize;

    SAL_DLLPRIVATE void         ImplCreateSvDrawStorage();
    SgaObject*                  ImplReadSgaObject( GalleryObject* pEntry );
    SAL_DLLPRIVATE bool         ImplWriteSgaObject( const SgaObject& rObj, size_t nPos, GalleryObject* pExistentEntry );
    SAL_DLLPRIVATE void         ImplWrite();
    SAL_DLLPRIVATE const GalleryObject* ImplGetGalleryObject( size_t nPos ) const
                                { return ( nPos < aObjectList.size() ) ? aObjectList[ nPos ] : NULL; }
    const GalleryObject*        ImplGetGalleryObject( const INetURLObject& rURL );

    SAL_DLLPRIVATE size_t       ImplGetGalleryObjectPos( const GalleryObject* pObj ) const
                                {
                                    for ( size_t i = 0, n = aObjectList.size(); i < n; ++i )
                                        if ( pObj == aObjectList[ i ] )
                                            return i;
                                    return size_t(-1);
                                }
    SAL_DLLPRIVATE INetURLObject ImplGetURL( const GalleryObject* pObject ) const;
    SAL_DLLPRIVATE INetURLObject ImplCreateUniqueURL( SgaObjKind eObjKind, ConvertDataFormat nFormat = ConvertDataFormat::Unknown );
    SAL_DLLPRIVATE void         ImplSetModified( bool bModified );
    SAL_DLLPRIVATE void         ImplBroadcast( sal_uIntPtr nUpdatePos );

    SAL_DLLPRIVATE              GalleryTheme();
    SAL_DLLPRIVATE              GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry );
    SAL_DLLPRIVATE              virtual ~GalleryTheme();

    GalleryTheme(GalleryTheme const &) = delete;
    void operator =(GalleryTheme const &) = delete;

public:

    SAL_DLLPRIVATE static GalleryThemeEntry* CreateThemeEntry( const INetURLObject& rURL, bool bReadOnly );

    SAL_DLLPRIVATE size_t       GetObjectCount() const { return aObjectList.size(); }

    SgaObject*                  AcquireObject( size_t nPos );
    static void                 ReleaseObject( SgaObject* pObj );

    bool                        InsertObject( const SgaObject& rObj, sal_uIntPtr nPos = CONTAINER_APPEND );
    bool                        RemoveObject( size_t nPos );
    SAL_DLLPRIVATE bool         ChangeObjectPos( size_t nOldPos, size_t nNewPos );

    const OUString&             GetName() const;
    SAL_DLLPRIVATE const OUString& GetRealName() const;

    // used for building gallery themes during compilation:
    SAL_DLLPRIVATE void         SetDestDir(const OUString& rDestDir, bool bRelative = true)
                                { m_aDestDir = rDestDir; m_bDestDirRelative = bRelative; }

    SAL_DLLPRIVATE const INetURLObject& GetThmURL() const;
    const INetURLObject&        GetSdgURL() const;
    SAL_DLLPRIVATE const INetURLObject& GetSdvURL() const;

    sal_uInt32                  GetId() const;
    SAL_DLLPRIVATE void         SetId( sal_uInt32 nNewId, bool bResetThemeName );

    SAL_DLLPRIVATE void         SetDragging( bool bSet ) { bDragging = bSet; }
    SAL_DLLPRIVATE bool         IsDragging() const { return bDragging; }

    SAL_DLLPRIVATE void         LockTheme() { ++mnThemeLockCount; }
    SAL_DLLPRIVATE bool         UnlockTheme();

    SAL_DLLPRIVATE void         LockBroadcaster() { mnBroadcasterLockCount++; }
    void                        UnlockBroadcaster( sal_uIntPtr nUpdatePos = 0 );
    SAL_DLLPRIVATE bool         IsBroadcasterLocked() const { return mnBroadcasterLockCount > 0; }

    SAL_DLLPRIVATE void         SetDragPos( sal_uIntPtr nPos ) { nDragPos = nPos; }
    SAL_DLLPRIVATE sal_uIntPtr  GetDragPos() const { return nDragPos; }

    SAL_DLLPRIVATE bool         IsThemeNameFromResource() const;

    bool                        IsReadOnly() const;
    bool                        IsDefault() const;
    SAL_DLLPRIVATE bool         IsModified() const;

    void                        Actualize( const Link<const INetURLObject&, void>& rActualizeLink, GalleryProgress* pProgress = NULL );
    SAL_DLLPRIVATE void         AbortActualize() { bAbortActualize = true; }

    SAL_DLLPRIVATE Gallery*     GetParent() const { return pParent; }
    SAL_DLLPRIVATE tools::SvRef<SotStorage> GetSvDrawStorage() const;

public:

    SAL_DLLPRIVATE SgaObjKind   GetObjectKind( sal_uIntPtr nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->eObjKind;
                                }


    SAL_DLLPRIVATE const INetURLObject& GetObjectURL( sal_uIntPtr nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->aURL;
                                }

    SAL_DLLPRIVATE bool         GetThumb( sal_uIntPtr nPos, BitmapEx& rBmp, bool bProgress = false );

    bool                        GetGraphic( sal_uIntPtr nPos, Graphic& rGraphic, bool bProgress = false );
    bool                        InsertGraphic( const Graphic& rGraphic, sal_uIntPtr nInsertPos = CONTAINER_APPEND );

    bool                        GetModel( sal_uIntPtr nPos, SdrModel& rModel, bool bProgress = false );
    bool                        InsertModel( const FmFormModel& rModel, sal_uIntPtr nInsertPos = CONTAINER_APPEND );

    SAL_DLLPRIVATE bool         GetModelStream( sal_uIntPtr nPos, tools::SvRef<SotStorageStream>& rModelStreamRef, bool bProgress = false );
    SAL_DLLPRIVATE bool         InsertModelStream( const tools::SvRef<SotStorageStream>& rModelStream, sal_uIntPtr nInsertPos = CONTAINER_APPEND );

    SAL_DLLPRIVATE bool         GetURL( sal_uIntPtr nPos, INetURLObject& rURL, bool bProgress = false );
    bool                        InsertURL( const INetURLObject& rURL, sal_uIntPtr nInsertPos = CONTAINER_APPEND );
    SAL_DLLPRIVATE bool         InsertFileOrDirURL( const INetURLObject& rFileOrDirURL, sal_uIntPtr nInsertPos = CONTAINER_APPEND );

    SAL_DLLPRIVATE bool         InsertTransferable( const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable, sal_uIntPtr nInsertPos );

    SAL_DLLPRIVATE void         CopyToClipboard( vcl::Window* pWindow, sal_uIntPtr nPos );
    SAL_DLLPRIVATE void         StartDrag( vcl::Window* pWindow, sal_uIntPtr nPos );

public:

    SAL_DLLPRIVATE SvStream&    WriteData( SvStream& rOut ) const;
    SAL_DLLPRIVATE SvStream&    ReadData( SvStream& rIn );
    static void                 InsertAllThemes( ListBox& rListBox );

    // for buffering PreviewBitmaps and strings for object and path
    SAL_DLLPRIVATE void GetPreviewBitmapExAndStrings(sal_uIntPtr nPos, BitmapEx& rBitmapEx, Size& rSize, OUString& rTitle, OUString& rPath) const;
    SAL_DLLPRIVATE void SetPreviewBitmapExAndStrings(sal_uIntPtr nPos, const BitmapEx& rBitmapEx, const Size& rSize, const OUString& rTitle, const OUString& rPath);
};

SvStream& WriteGalleryTheme( SvStream& rOut, const GalleryTheme& rTheme );
SvStream& ReadGalleryTheme( SvStream& rIn, GalleryTheme& rTheme );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
    tools::SvRef<SotStorage>    aSvDrawStorageRef;
    Gallery*                    pParent;
    GalleryThemeEntry*          pThm;
    sal_uInt32                  mnThemeLockCount;
    sal_uInt32                  mnBroadcasterLockCount;
    sal_Int32                   nDragPos;
    bool                        bDragging;
    bool                        bAbortActualize;

    SAL_DLLPRIVATE void         ImplCreateSvDrawStorage();
    SgaObject*                  ImplReadSgaObject( GalleryObject const * pEntry );
    SAL_DLLPRIVATE bool         ImplWriteSgaObject( const SgaObject& rObj, sal_Int32 nPos, GalleryObject* pExistentEntry );
    SAL_DLLPRIVATE void         ImplWrite();
    SAL_DLLPRIVATE const GalleryObject* ImplGetGalleryObject( sal_Int32 nPos ) const
                                { return ( nPos < sal_Int32(aObjectList.size()) ) ? aObjectList[ nPos ] : nullptr; }
    const GalleryObject*        ImplGetGalleryObject( const INetURLObject& rURL );

    SAL_DLLPRIVATE sal_Int32       ImplGetGalleryObjectPos( const GalleryObject* pObj ) const
                                {
                                    for ( size_t i = 0, n = aObjectList.size(); i < n; ++i )
                                        if ( pObj == aObjectList[ i ] )
                                            return i;
                                    return -1;
                                }
    SAL_DLLPRIVATE static INetURLObject ImplGetURL( const GalleryObject* pObject );
    SAL_DLLPRIVATE INetURLObject ImplCreateUniqueURL( SgaObjKind eObjKind, ConvertDataFormat nFormat = ConvertDataFormat::Unknown );
    SAL_DLLPRIVATE void         ImplSetModified( bool bModified );
    SAL_DLLPRIVATE void         ImplBroadcast( sal_Int32 nUpdatePos );

    SAL_DLLPRIVATE              GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry );

    GalleryTheme(GalleryTheme const &) = delete;
    void operator =(GalleryTheme const &) = delete;

public:

    SAL_DLLPRIVATE              virtual ~GalleryTheme() override;

    SAL_DLLPRIVATE static GalleryThemeEntry* CreateThemeEntry( const INetURLObject& rURL, bool bReadOnly );

    SAL_DLLPRIVATE sal_Int32    GetObjectCount() const { return aObjectList.size(); }

    SgaObject*                  AcquireObject( sal_Int32 nPos );
    static void                 ReleaseObject( SgaObject* pObj );

    bool                        InsertObject( const SgaObject& rObj, sal_Int32 nPos = SAL_MAX_INT32 );
    bool                        RemoveObject( sal_Int32 nPos );
    SAL_DLLPRIVATE bool         ChangeObjectPos( sal_Int32 nOldPos, sal_Int32 nNewPos );

    const OUString&             GetName() const;

    // used for building gallery themes during compilation:
    SAL_DLLPRIVATE void         SetDestDir(const OUString& rDestDir, bool bRelative)
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
    void                        UnlockBroadcaster();
    SAL_DLLPRIVATE bool         IsBroadcasterLocked() const { return mnBroadcasterLockCount > 0; }

    SAL_DLLPRIVATE void         SetDragPos( sal_Int32 nPos ) { nDragPos = nPos; }
    SAL_DLLPRIVATE sal_Int32    GetDragPos() const { return nDragPos; }

    SAL_DLLPRIVATE bool         IsThemeNameFromResource() const;

    bool                        IsReadOnly() const;
    bool                        IsDefault() const;

    void                        Actualize( const Link<const INetURLObject&, void>& rActualizeLink, GalleryProgress* pProgress = nullptr );
    SAL_DLLPRIVATE void         AbortActualize() { bAbortActualize = true; }

    SAL_DLLPRIVATE Gallery*     GetParent() const { return pParent; }
    SAL_DLLPRIVATE const tools::SvRef<SotStorage>& GetSvDrawStorage() const;

public:

    SAL_DLLPRIVATE SgaObjKind   GetObjectKind( sal_Int32 nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->eObjKind;
                                }


    SAL_DLLPRIVATE const INetURLObject& GetObjectURL( sal_Int32 nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->aURL;
                                }

    SAL_DLLPRIVATE bool         GetThumb( sal_Int32 nPos, BitmapEx& rBmp );

    bool                        GetGraphic( sal_Int32 nPos, Graphic& rGraphic, bool bProgress = false );
    bool                        InsertGraphic( const Graphic& rGraphic, sal_Int32 nInsertPos );

    bool                        GetModel( sal_Int32 nPos, SdrModel& rModel );
    bool                        InsertModel( const FmFormModel& rModel, sal_Int32 nInsertPos );

    SAL_DLLPRIVATE bool         GetModelStream( sal_Int32 nPos, tools::SvRef<SotStorageStream> const & rModelStreamRef );
    SAL_DLLPRIVATE bool         InsertModelStream( const tools::SvRef<SotStorageStream>& rModelStream, sal_Int32 nInsertPos );

    SAL_DLLPRIVATE bool         GetURL( sal_Int32 nPos, INetURLObject& rURL );
    bool                        InsertURL( const INetURLObject& rURL, sal_Int32 nInsertPos = SAL_MAX_INT32 );
    SAL_DLLPRIVATE bool         InsertFileOrDirURL( const INetURLObject& rFileOrDirURL, sal_Int32 nInsertPos );

    SAL_DLLPRIVATE bool         InsertTransferable( const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable, sal_Int32 nInsertPos );

    SAL_DLLPRIVATE void         CopyToClipboard( vcl::Window* pWindow, sal_Int32 nPos );
    SAL_DLLPRIVATE void         StartDrag( vcl::Window* pWindow, sal_Int32 nPos );

public:

    SAL_DLLPRIVATE SvStream&    WriteData( SvStream& rOut ) const;
    SAL_DLLPRIVATE SvStream&    ReadData( SvStream& rIn );
    static void                 InsertAllThemes( ListBox& rListBox );

    // for buffering PreviewBitmaps and strings for object and path
    SAL_DLLPRIVATE void GetPreviewBitmapExAndStrings(sal_Int32 nPos, BitmapEx& rBitmapEx, Size& rSize, OUString& rTitle, OUString& rPath) const;
    SAL_DLLPRIVATE void SetPreviewBitmapExAndStrings(sal_Int32 nPos, const BitmapEx& rBitmapEx, const Size& rSize, const OUString& rTitle, const OUString& rPath);
};

SvStream& WriteGalleryTheme( SvStream& rOut, const GalleryTheme& rTheme );
SvStream& ReadGalleryTheme( SvStream& rIn, GalleryTheme& rTheme );

#endif // INCLUDED_SVX_GALTHEME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

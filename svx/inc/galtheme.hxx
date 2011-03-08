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

#ifndef _SVX_GALTHEME_HXX_
#define _SVX_GALTHEME_HXX_

#include "svx/svxdllapi.h"

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/salctype.hxx>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <svtools/transfer.hxx>
#include <sot/storage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/galmisc.hxx>

// -----------------
// - GalleryObject -
// -----------------
// --------------
// - SgaObjKind -
// --------------

struct GalleryObject
{
    INetURLObject   aURL;
    sal_uInt32      nOffset;
    SgaObjKind      eObjKind;
    BOOL            bDummy;
};

DECLARE_LIST( GalleryObjectList, GalleryObject* )

class GalleryThemeEntry;
class SgaObject;
class FmFormModel;
class ListBox;

// -----------------
// - GalDragParams -
// -----------------

struct GalDragParams
{
    Region      aDragRegion;
    ULONG       nDragObjPos;
    String      aThemeName;
    String      aFileName;
    SgaObjKind  eObjKind;
};

// ----------------
// - GalleryTheme -
// ----------------

class Gallery;
class GalleryProgress;
namespace unogallery
{
    class GalleryTheme;
    class GalleryItem;
}

class GalleryTheme : public SfxBroadcaster
{
    friend class Gallery;
    friend class GalleryThemeCacheEntry;
    friend class ::unogallery::GalleryTheme;
    friend class ::unogallery::GalleryItem;

private:

    GalleryObjectList           aObjectList;
    String                      aImportName;
    String                      m_aDestDir;
    SotStorageRef               aSvDrawStorageRef;
    Gallery*                    pParent;
    GalleryThemeEntry*          pThm;
    ULONG                       mnThemeLockCount;
    ULONG                       mnBroadcasterLockCount;
    ULONG                       nDragPos;
    BOOL                        bDragging;
    BOOL                        bAbortActualize;

    void                        ImplCreateSvDrawStorage();
    SVX_DLLPUBLIC SgaObject*                    ImplReadSgaObject( GalleryObject* pEntry );
    BOOL                        ImplWriteSgaObject( const SgaObject& rObj, ULONG nPos, GalleryObject* pExistentEntry );
    void                        ImplRead();
    void                        ImplWrite();
    const GalleryObject*        ImplGetGalleryObject( ULONG nPos ) const { return aObjectList.GetObject( nPos ); }
    SVX_DLLPUBLIC const GalleryObject*      ImplGetGalleryObject( const INetURLObject& rURL );
    ULONG                       ImplGetGalleryObjectPos( const GalleryObject* pObj ) const { return aObjectList.GetPos( pObj ); }
    INetURLObject               ImplGetURL( const GalleryObject* pObject ) const;
    INetURLObject               ImplCreateUniqueURL( SgaObjKind eObjKind, ULONG nFormat = CVT_UNKNOWN );
    void                        ImplSetModified( BOOL bModified );
    void                        ImplBroadcast( ULONG nUpdatePos );

                                GalleryTheme();
                                GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry );
                                ~GalleryTheme();

public:

    static GalleryThemeEntry*   CreateThemeEntry( const INetURLObject& rURL, BOOL bReadOnly );

    ULONG                   GetObjectCount() const { return aObjectList.Count(); }

    SVX_DLLPUBLIC SgaObject*                    AcquireObject( ULONG nPos );
    SVX_DLLPUBLIC void                      ReleaseObject( SgaObject* pObj );

    SVX_DLLPUBLIC BOOL                      InsertObject( const SgaObject& rObj, ULONG nPos = LIST_APPEND );
    SVX_DLLPUBLIC BOOL                      RemoveObject( ULONG nPos );
    BOOL                        ChangeObjectPos( ULONG nOldPos, ULONG nNewPos );

    SVX_DLLPUBLIC const String& GetName() const;
    const String&               GetRealName() const;
    const String&               GetImportName() const { return aImportName; }
    void                        SetImportName(const String& rImportName) { aImportName = rImportName; }

    const String&               GetDestDir() const { return m_aDestDir; }
    void                        SetDestDir(const String& rDestDir) { m_aDestDir = rDestDir; }

    const INetURLObject&        GetThmURL() const;
    SVX_DLLPUBLIC const INetURLObject&      GetSdgURL() const;
    const INetURLObject&        GetSdvURL() const;

    SVX_DLLPUBLIC UINT32        GetId() const;
    void                        SetId( UINT32 nNewId, BOOL bResetThemeName );

    void                        SetDragging( BOOL bSet ) { bDragging = bSet; }
    BOOL                        IsDragging() const { return bDragging; }

    void                        LockTheme() { ++mnThemeLockCount; }
    BOOL                        UnlockTheme();

    void                        LockBroadcaster() { mnBroadcasterLockCount++; }
    SVX_DLLPUBLIC void          UnlockBroadcaster( ULONG nUpdatePos = 0 );
    BOOL                        IsBroadcasterLocked() const { return mnBroadcasterLockCount > 0; }

    void                        SetDragPos( ULONG nPos ) { nDragPos = nPos; }
    ULONG                       GetDragPos() const { return nDragPos; }

    BOOL                        IsThemeNameFromResource() const;

    SVX_DLLPUBLIC BOOL          IsImported() const;
    SVX_DLLPUBLIC BOOL          IsReadOnly() const;
    SVX_DLLPUBLIC BOOL          IsDefault() const;
    BOOL                        IsModified() const;

    SVX_DLLPUBLIC void                      Actualize( const Link& rActualizeLink, GalleryProgress* pProgress = NULL );
    void                        AbortActualize() { bAbortActualize = TRUE; }

    Gallery*                    GetParent() const { return pParent; }
    SotStorageRef               GetSvDrawStorage() const { return aSvDrawStorageRef; }

public:

    SgaObjKind                  GetObjectKind( ULONG nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->eObjKind;
                                }


    const INetURLObject&        GetObjectURL( ULONG nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->aURL;
                                }

    BOOL                        GetThumb( ULONG nPos, Bitmap& rBmp, BOOL bProgress = FALSE );

    SVX_DLLPUBLIC BOOL                      GetGraphic( ULONG nPos, Graphic& rGraphic, BOOL bProgress = FALSE );
    SVX_DLLPUBLIC BOOL                      InsertGraphic( const Graphic& rGraphic, ULONG nInsertPos = LIST_APPEND );

    SVX_DLLPUBLIC BOOL                      GetModel( ULONG nPos, SdrModel& rModel, BOOL bProgress = FALSE );
    SVX_DLLPUBLIC BOOL                      InsertModel( const FmFormModel& rModel, ULONG nInsertPos = LIST_APPEND );

    BOOL                        GetModelStream( ULONG nPos, SotStorageStreamRef& rModelStreamRef, BOOL bProgress = FALSE );
    BOOL                        InsertModelStream( const SotStorageStreamRef& rModelStream, ULONG nInsertPos = LIST_APPEND );

    BOOL                        GetURL( ULONG nPos, INetURLObject& rURL, BOOL bProgress = FALSE );
    SVX_DLLPUBLIC BOOL                      InsertURL( const INetURLObject& rURL, ULONG nInsertPos = LIST_APPEND );
    BOOL                        InsertFileOrDirURL( const INetURLObject& rFileOrDirURL, ULONG nInsertPos = LIST_APPEND );

    BOOL                        InsertTransferable( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable, ULONG nInsertPos );

    void                        CopyToClipboard( Window* pWindow, ULONG nPos );
    void                        StartDrag( Window* pWindow, ULONG nPos );

public:

    SvStream&                   WriteData( SvStream& rOut ) const;
    SvStream&                   ReadData( SvStream& rIn );
    static SVX_DLLPUBLIC void   InsertAllThemes( ListBox& rListBox );
};

SvStream& operator<<( SvStream& rOut, const GalleryTheme& rTheme );
SvStream& operator>>( SvStream& rIn, GalleryTheme& rTheme );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

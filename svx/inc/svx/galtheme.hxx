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
#include <vector>

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
    sal_Bool            bDummy;
};

typedef ::std::vector< GalleryObject* > GalleryObjectList;

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
    sal_uIntPtr     nDragObjPos;
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
    sal_uIntPtr                       mnThemeLockCount;
    sal_uIntPtr                     mnBroadcasterLockCount;
    sal_uIntPtr                     nDragPos;
    sal_Bool                        bDragging;
    sal_Bool                        bAbortActualize;

    void                        ImplCreateSvDrawStorage();
    SVX_DLLPUBLIC SgaObject*                    ImplReadSgaObject( GalleryObject* pEntry );
    BOOL                        ImplWriteSgaObject( const SgaObject& rObj, size_t nPos, GalleryObject* pExistentEntry );
    void                        ImplRead();
    void                        ImplWrite();
    const GalleryObject*        ImplGetGalleryObject( size_t nPos ) const
                                { return ( nPos < aObjectList.size() ) ? aObjectList[ nPos ] : NULL; }
    SVX_DLLPUBLIC const GalleryObject*      ImplGetGalleryObject( const INetURLObject& rURL );

    size_t                      ImplGetGalleryObjectPos( const GalleryObject* pObj ) const
                                {
                                    for ( size_t i = 0, n = aObjectList.size(); i < n; ++i )
                                        if ( pObj == aObjectList[ i ] )
                                            return i;
                                    return size_t(-1);
                                }
    INetURLObject               ImplGetURL( const GalleryObject* pObject ) const;
    INetURLObject               ImplCreateUniqueURL( SgaObjKind eObjKind, sal_uIntPtr nFormat = CVT_UNKNOWN );
    void                        ImplSetModified( sal_Bool bModified );
    void                        ImplBroadcast( sal_uIntPtr nUpdatePos );

                                GalleryTheme();
                                GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry );
                                ~GalleryTheme();

public:

    static GalleryThemeEntry*   CreateThemeEntry( const INetURLObject& rURL, sal_Bool bReadOnly );

    size_t                      GetObjectCount() const { return aObjectList.size(); }

    SVX_DLLPUBLIC SgaObject*    AcquireObject( size_t nPos );
    SVX_DLLPUBLIC void                      ReleaseObject( SgaObject* pObj );

    SVX_DLLPUBLIC sal_Bool                      InsertObject( const SgaObject& rObj, sal_uIntPtr nPos = LIST_APPEND );
    SVX_DLLPUBLIC BOOL          RemoveObject( size_t nPos );
    BOOL                        ChangeObjectPos( size_t nOldPos, size_t nNewPos );

    SVX_DLLPUBLIC const String& GetName() const;
    const String&               GetRealName() const;
    const String&               GetImportName() const { return aImportName; }
    void                        SetImportName(const String& rImportName) { aImportName = rImportName; }

    const String&               GetDestDir() const { return m_aDestDir; }
    void                        SetDestDir(const String& rDestDir) { m_aDestDir = rDestDir; }

    const INetURLObject&        GetThmURL() const;
    SVX_DLLPUBLIC const INetURLObject&      GetSdgURL() const;
    const INetURLObject&        GetSdvURL() const;

    SVX_DLLPUBLIC sal_uInt32        GetId() const;
    void                        SetId( sal_uInt32 nNewId, sal_Bool bResetThemeName );

    void                        SetDragging( sal_Bool bSet ) { bDragging = bSet; }
    sal_Bool                        IsDragging() const { return bDragging; }

    void                        LockTheme() { ++mnThemeLockCount; }
    sal_Bool                        UnlockTheme();

    void                        LockBroadcaster() { mnBroadcasterLockCount++; }
    SVX_DLLPUBLIC void          UnlockBroadcaster( sal_uIntPtr nUpdatePos = 0 );
    sal_Bool                        IsBroadcasterLocked() const { return mnBroadcasterLockCount > 0; }

    void                        SetDragPos( sal_uIntPtr nPos ) { nDragPos = nPos; }
    sal_uIntPtr                     GetDragPos() const { return nDragPos; }

    sal_Bool                        IsThemeNameFromResource() const;

    SVX_DLLPUBLIC sal_Bool          IsImported() const;
    SVX_DLLPUBLIC sal_Bool          IsReadOnly() const;
    SVX_DLLPUBLIC sal_Bool          IsDefault() const;
    sal_Bool                        IsModified() const;

    SVX_DLLPUBLIC void                      Actualize( const Link& rActualizeLink, GalleryProgress* pProgress = NULL );
    void                        AbortActualize() { bAbortActualize = sal_True; }

    Gallery*                    GetParent() const { return pParent; }
    SotStorageRef               GetSvDrawStorage() const { return aSvDrawStorageRef; }

public:

    SgaObjKind                  GetObjectKind( sal_uIntPtr nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->eObjKind;
                                }


    const INetURLObject&        GetObjectURL( sal_uIntPtr nPos ) const
                                {
                                    DBG_ASSERT( nPos < GetObjectCount(), "Position out of range" );
                                    return ImplGetGalleryObject( nPos )->aURL;
                                }

    sal_Bool                        GetThumb( sal_uIntPtr nPos, Bitmap& rBmp, sal_Bool bProgress = sal_False );

    SVX_DLLPUBLIC sal_Bool                      GetGraphic( sal_uIntPtr nPos, Graphic& rGraphic, sal_Bool bProgress = sal_False );
    SVX_DLLPUBLIC sal_Bool                      InsertGraphic( const Graphic& rGraphic, sal_uIntPtr nInsertPos = LIST_APPEND );

    SVX_DLLPUBLIC sal_Bool                      GetModel( sal_uIntPtr nPos, SdrModel& rModel, sal_Bool bProgress = sal_False );
    SVX_DLLPUBLIC sal_Bool                      InsertModel( const FmFormModel& rModel, sal_uIntPtr nInsertPos = LIST_APPEND );

    sal_Bool                        GetModelStream( sal_uIntPtr nPos, SotStorageStreamRef& rModelStreamRef, sal_Bool bProgress = sal_False );
    sal_Bool                        InsertModelStream( const SotStorageStreamRef& rModelStream, sal_uIntPtr nInsertPos = LIST_APPEND );

    sal_Bool                        GetURL( sal_uIntPtr nPos, INetURLObject& rURL, sal_Bool bProgress = sal_False );
    SVX_DLLPUBLIC sal_Bool                      InsertURL( const INetURLObject& rURL, sal_uIntPtr nInsertPos = LIST_APPEND );
    sal_Bool                        InsertFileOrDirURL( const INetURLObject& rFileOrDirURL, sal_uIntPtr nInsertPos = LIST_APPEND );

    sal_Bool                        InsertTransferable( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable, sal_uIntPtr nInsertPos );

    void                        CopyToClipboard( Window* pWindow, sal_uIntPtr nPos );
    void                        StartDrag( Window* pWindow, sal_uIntPtr nPos );

public:

    SvStream&                   WriteData( SvStream& rOut ) const;
    SvStream&                   ReadData( SvStream& rIn );
    static SVX_DLLPUBLIC void   InsertAllThemes( ListBox& rListBox );
};

SvStream& operator<<( SvStream& rOut, const GalleryTheme& rTheme );
SvStream& operator>>( SvStream& rIn, GalleryTheme& rTheme );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

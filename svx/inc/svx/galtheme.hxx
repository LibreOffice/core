/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
//#include <svx/galobj.hxx>
#include <svx/galmisc.hxx>
//#include <svx/gallery1.hxx>

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
    bool            mbDelete;

    //UI visualization buffering
    BitmapEx        maPreviewBitmapEx;
    Size            maPreparedSize;
    String          maTitle;
    String          maPath;
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
    sal_Bool                        ImplWriteSgaObject( const SgaObject& rObj, sal_uIntPtr nPos, GalleryObject* pExistentEntry );
    void                        ImplRead();
    void                        ImplWrite();
    const GalleryObject*        ImplGetGalleryObject( sal_uIntPtr nPos ) const { return aObjectList.GetObject( nPos ); }
    SVX_DLLPUBLIC const GalleryObject*      ImplGetGalleryObject( const INetURLObject& rURL );
    sal_uIntPtr                     ImplGetGalleryObjectPos( const GalleryObject* pObj ) const { return aObjectList.GetPos( pObj ); }
    INetURLObject               ImplGetURL( const GalleryObject* pObject ) const;
    INetURLObject               ImplCreateUniqueURL( SgaObjKind eObjKind, sal_uIntPtr nFormat = CVT_UNKNOWN );
    void                        ImplSetModified( sal_Bool bModified );
    void                        ImplBroadcast( sal_uIntPtr nUpdatePos );

                                GalleryTheme();
                                GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry );
                                ~GalleryTheme();

public:

    static GalleryThemeEntry*   CreateThemeEntry( const INetURLObject& rURL, sal_Bool bReadOnly );

    sal_uIntPtr                 GetObjectCount() const { return aObjectList.Count(); }

    SVX_DLLPUBLIC SgaObject*                    AcquireObject( sal_uIntPtr nPos );
    SVX_DLLPUBLIC void                      ReleaseObject( SgaObject* pObj );

    SVX_DLLPUBLIC sal_Bool                      InsertObject( const SgaObject& rObj, sal_uIntPtr nPos = LIST_APPEND );
    SVX_DLLPUBLIC sal_Bool                      RemoveObject( sal_uIntPtr nPos );
    sal_Bool                        ChangeObjectPos( sal_uIntPtr nOldPos, sal_uIntPtr nNewPos );

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

    sal_Bool                        GetThumb( sal_uIntPtr nPos, BitmapEx& rBmp, sal_Bool bProgress = sal_False );

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

    // for buffering PreviewBitmaps and strings for object and path
    void GetPreviewBitmapExAndStrings(sal_uIntPtr nPos, BitmapEx& rBitmapEx, Size& rSize, String& rTitle, String& rPath) const;
    void SetPreviewBitmapExAndStrings(sal_uIntPtr nPos, const BitmapEx& rBitmapEx, const Size& rSize, const String& rTitle, const String& rPath);
};

SvStream& operator<<( SvStream& rOut, const GalleryTheme& rTheme );
SvStream& operator>>( SvStream& rIn, GalleryTheme& rTheme );

#endif

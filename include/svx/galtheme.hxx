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
#include <svx/galleryobjectcollection.hxx>
#include <svx/gallerybinaryengine.hxx>

#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <vcl/bitmapex.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svx/galmisc.hxx>
#include <memory>
#include <vector>

namespace weld { class ComboBox; }

class SotStorageStream;

class GalleryThemeEntry;
class SgaObject;
class FmFormModel;
class Gallery;

namespace unogallery
{
    class GalleryTheme;
    class GalleryItem;
}

class SVXCORE_DLLPUBLIC GalleryTheme : public SfxBroadcaster
{
    friend class Gallery;
    friend class GalleryThemeCacheEntry;
    friend class ::unogallery::GalleryTheme;
    friend class ::unogallery::GalleryItem;

private:

    std::unique_ptr<GalleryBinaryEngine>     mpGalleryStorageEngine;
    GalleryObjectCollection     maGalleryObjectCollection;
    Gallery*                    pParent;
    GalleryThemeEntry*          pThm;
    sal_uInt32                  mnThemeLockCount;
    sal_uInt32                  mnBroadcasterLockCount;
    sal_uInt32                  nDragPos;
    bool                        bDragging;
    bool                        bAbortActualize;

    std::unique_ptr<GalleryBinaryEngine> createGalleryStorageEngine(bool bReadOnly);
    const std::unique_ptr<GalleryBinaryEngine>& getGalleryStorageEngine() const { return mpGalleryStorageEngine; }

    SAL_DLLPRIVATE void         ImplSetModified( bool bModified );
    SAL_DLLPRIVATE void         ImplBroadcast(sal_uInt32 nUpdatePos);

    SAL_DLLPRIVATE              GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry );

    GalleryTheme(GalleryTheme const &) = delete;
    void operator =(GalleryTheme const &) = delete;

public:

    SAL_DLLPRIVATE              virtual ~GalleryTheme() override;

    SAL_DLLPRIVATE sal_uInt32   GetObjectCount() const { return maGalleryObjectCollection.size(); }

    std::unique_ptr<SgaObject>  AcquireObject(sal_uInt32 nPos);

    bool                        InsertObject(const SgaObject& rObj, sal_uInt32 nPos = SAL_MAX_UINT32);
    void                        RemoveObject(sal_uInt32 nPos);
    bool                        ChangeObjectPos(sal_uInt32 nOldPos, sal_uInt32 nNewPos);

    const OUString&             GetName() const;

    // used for building gallery themes during compilation:
    void                        SetDestDir(const OUString& rDestDir, bool bRelative);

    sal_uInt32                  GetId() const;
    SAL_DLLPRIVATE void         SetId( sal_uInt32 nNewId, bool bResetThemeName );

    SAL_DLLPRIVATE void         SetDragging( bool bSet ) { bDragging = bSet; }
    SAL_DLLPRIVATE bool         IsDragging() const { return bDragging; }

    SAL_DLLPRIVATE void         LockTheme() { ++mnThemeLockCount; }
    SAL_DLLPRIVATE bool         UnlockTheme();

    SAL_DLLPRIVATE void         LockBroadcaster() { mnBroadcasterLockCount++; }
    void                        UnlockBroadcaster();
    SAL_DLLPRIVATE bool         IsBroadcasterLocked() const { return mnBroadcasterLockCount > 0; }

    SAL_DLLPRIVATE void         SetDragPos(sal_uInt32 nPos) { nDragPos = nPos; }
    SAL_DLLPRIVATE sal_uInt32   GetDragPos() const { return nDragPos; }

    bool                        IsReadOnly() const;
    bool                        IsDefault() const;

    void                        Actualize( const Link<const INetURLObject&, void>& rActualizeLink, GalleryProgress* pProgress = nullptr );
    SAL_DLLPRIVATE void         AbortActualize() { bAbortActualize = true; }

    SAL_DLLPRIVATE Gallery*     GetParent() const { return pParent; }

public:

    SAL_DLLPRIVATE SgaObjKind GetObjectKind(sal_uInt32 nPos) const
    {
        if (nPos < GetObjectCount())
            return maGalleryObjectCollection.getForPosition( nPos )->eObjKind;
        return SgaObjKind::NONE;
    }

    SAL_DLLPRIVATE const INetURLObject& GetObjectURL(sal_uInt32 nPos) const
    {
        DBG_ASSERT(nPos < GetObjectCount(), "Position out of range");
        return maGalleryObjectCollection.getURLForPosition(nPos);
    }

    SAL_DLLPRIVATE bool         GetThumb(sal_uInt32 nPos, BitmapEx& rBmp);

    bool                        GetGraphic(sal_uInt32 nPos, Graphic& rGraphic);
    bool                        InsertGraphic(const Graphic& rGraphic, sal_uInt32 nInsertPos);

    bool                        GetModel(sal_uInt32 nPos, SdrModel& rModel);
    bool                        InsertModel(const FmFormModel& rModel, sal_uInt32 nInsertPos);

    SAL_DLLPRIVATE bool         GetModelStream(sal_uInt32 nPos, tools::SvRef<SotStorageStream> const & rModelStreamRef);
    SAL_DLLPRIVATE bool         InsertModelStream(const tools::SvRef<SotStorageStream>& rModelStream, sal_uInt32 nInsertPos);

    SAL_DLLPRIVATE bool         GetURL(sal_uInt32 nPos, INetURLObject& rURL);
    bool                        InsertURL(const INetURLObject& rURL, sal_uInt32 nInsertPos = SAL_MAX_UINT32);
    SAL_DLLPRIVATE bool         InsertFileOrDirURL(const INetURLObject& rFileOrDirURL, sal_uInt32 nInsertPos);

    SAL_DLLPRIVATE bool         InsertTransferable(const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable, sal_uInt32 nInsertPos);

    SAL_DLLPRIVATE void         CopyToClipboard(sal_uInt32 nPos);

    DateTime getModificationDate() const;

    const INetURLObject& getThemeURL() const { return mpGalleryStorageEngine->getThemeURL(); }

public:

    SAL_DLLPRIVATE SvStream&    ReadData( SvStream& rIn );
    static void                 InsertAllThemes(weld::ComboBox& rListBox);

    // for buffering PreviewBitmaps and strings for object and path
    SAL_DLLPRIVATE void GetPreviewBitmapExAndStrings(sal_uInt32 nPos, BitmapEx& rBitmapEx, Size& rSize, OUString& rTitle, OUString& rPath);
    SAL_DLLPRIVATE void SetPreviewBitmapExAndStrings(sal_uInt32 nPos, const BitmapEx& rBitmapEx, const Size& rSize, const OUString& rTitle, const OUString& rPath);
};

SvStream& ReadGalleryTheme( SvStream& rIn, GalleryTheme& rTheme );

#endif // INCLUDED_SVX_GALTHEME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

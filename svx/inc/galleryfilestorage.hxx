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

#pragma once

#include <svx/galmisc.hxx>
#include <svx/svxdllapi.h>
#include <svx/fmmodel.hxx>
#include "gallerystoragelocations.hxx"
#include <tools/urlobj.hxx>
#include <sot/storage.hxx>
#include <vcl/salctype.hxx>

#include <tools/datetime.hxx>

#include <memory>

class GalleryObjectCollection;
class SgaObjectSvDraw;
class SgaObjectBmp;
class SgaObject;
class SotStorage;
struct GalleryObject;
class FmFormModel;
class GalleryTheme;
class GalleryThemeEntry;

class GalleryFileStorage final
{
private:
    rtl::Reference<SotStorage> m_aSvDrawStorageRef;
    const GalleryStorageLocations& maGalleryStorageLocations;
    GalleryObjectCollection& mrGalleryObjectCollection;
    bool mbReadOnly;
    OUString m_aDestDir;
    bool m_bDestDirRelative;

    const INetURLObject& GetSdgURL() const { return maGalleryStorageLocations.GetSdgURL(); }
    const INetURLObject& GetSdvURL() const { return maGalleryStorageLocations.GetSdvURL(); }
    const INetURLObject& GetThmURL() const { return maGalleryStorageLocations.GetThmURL(); }

public:
    GalleryFileStorage(const GalleryStorageLocations& rGalleryStorageLocations,
                       GalleryObjectCollection& rGalleryObjectCollection, bool bReadOnly);
    ~GalleryFileStorage();

    void clearSotStorage();

    void setDestDir(const OUString& rDestDir, bool bRelative);

    void ImplCreateSvDrawStorage();
    const rtl::Reference<SotStorage>& GetSvDrawStorage() const;

    const INetURLObject& getThemeURL() const { return maGalleryStorageLocations.getThemeURL(); }

    bool implWrite(const GalleryTheme& rTheme, const GalleryThemeEntry* pThm);

    void insertObject(const SgaObject& rObj, GalleryObject* pFoundEntry, sal_uInt32 nInsertPos);
    void removeObject(const std::unique_ptr<GalleryObject>& pEntry);

    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC)
    std::unique_ptr<SgaObject> implReadSgaObject(GalleryObject const* pEntry);
    bool implWriteSgaObject(const SgaObject& rObj, sal_uInt32 nPos, GalleryObject* pExistentEntry);

    bool readModel(const GalleryObject* pObject, SdrModel& rModel);
    SgaObjectSvDraw insertModel(const FmFormModel& rModel, const INetURLObject& rUserURL);

    bool readModelStream(const GalleryObject* pObject, SvStream& rModelStream);
    SgaObjectSvDraw insertModelStream(SvStream& rModelStream, const INetURLObject& rUserURL);

    INetURLObject implCreateUniqueURL(SgaObjKind eObjKind, const INetURLObject& rUserURL,
                                      ConvertDataFormat nFormat = ConvertDataFormat::Unknown);

    SgaObjectBmp insertGraphic(const Graphic& rGraphic, const GfxLink& aGfxLink,
                               const ConvertDataFormat& nExportFormat,
                               const INetURLObject& rUserURL);

    SgaObjectSvDraw updateSvDrawObject(const GalleryObject* pEntry);

    void updateTheme();
    static void insertFileOrDirURL(const INetURLObject& rFileOrDirURL,
                                   std::vector<INetURLObject>& rURLVector);

    SvStream& writeGalleryTheme(SvStream& rOStm, const GalleryTheme& rTheme,
                                const GalleryThemeEntry* pThm);

    DateTime getModificationDate() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

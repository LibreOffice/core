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

#include <tools/urlobj.hxx>
#include <svx/svxdllapi.h>
#include <svx/galtheme.hxx>
#include <sot/storage.hxx>

class SgaObjectSvDraw;
class SotStorage;
struct GalleryObject;

class SVXCORE_DLLPUBLIC GalleryBinaryEngine
{
private:
    INetURLObject aThmURL;
    INetURLObject aSdgURL;
    INetURLObject aSdvURL;
    INetURLObject aStrURL;
    tools::SvRef<SotStorage> aSvDrawStorageRef;

    static INetURLObject ImplGetURLIgnoreCase(const INetURLObject& rURL);

public:
    void galleryThemeInit(bool bReadOnly);
    void galleryThemeDestroy();

    static void CreateUniqueURL(const INetURLObject& rBaseURL, INetURLObject& aURL);

    OUString ReadStrFromIni(const OUString& aKeyName);

    void SetThmExtension(INetURLObject aURL);
    void SetSdgExtension(INetURLObject aURL);
    void SetSdvExtension(INetURLObject aURL);
    void SetStrExtension(INetURLObject aURL);

    const INetURLObject& GetThmURL() const { return aThmURL; }
    const INetURLObject& GetSdgURL() const { return aSdgURL; }
    const INetURLObject& GetSdvURL() const { return aSdvURL; }
    const INetURLObject& GetStrURL() const { return aStrURL; }

    SAL_DLLPRIVATE void ImplCreateSvDrawStorage(bool bReadOnly);
    SAL_DLLPRIVATE const tools::SvRef<SotStorage>& GetSvDrawStorage() const;

    std::unique_ptr<SgaObject> implReadSgaObject(GalleryObject const* pEntry);
    bool implWriteSgaObject(const SgaObject& rObj, sal_uInt32 nPos, GalleryObject* pExistentEntry,
                            OUString& aDestDir,
                            ::std::vector<std::unique_ptr<GalleryObject>>& aObjectList);
    SAL_DLLPRIVATE bool implWrite(const GalleryTheme& rTheme);

    bool readModel(const GalleryObject* pObject, SdrModel& rModel);
    bool insertModel(const FmFormModel& rModel, INetURLObject& rURL);

    bool readModelStream(const GalleryObject* pObject,
                         tools::SvRef<SotStorageStream> const& rxModelStream);
    SgaObjectSvDraw insertModelStream(const tools::SvRef<SotStorageStream>& rxModelStream,
                                      INetURLObject& rURL);

    void insertObject(const SgaObject& rObj, GalleryObject* pFoundEntry, OUString& rDestDir,
                      ::std::vector<std::unique_ptr<GalleryObject>>& rObjectList,
                      sal_uInt32& rInsertPos);

    SAL_DLLPRIVATE static GalleryThemeEntry* CreateThemeEntry(const INetURLObject& rURL,
                                                              bool bReadOnly);
};

SvStream& WriteGalleryTheme(SvStream& rOut, const GalleryTheme& rTheme);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

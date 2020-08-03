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

#include <memory>
#include <vector>
#include <tools/urlobj.hxx>
#include <sot/storage.hxx>
#include <vcl/salctype.hxx>
#include <svx/galmisc.hxx>

class SgaObjectSvDraw;
class SgaObjectBmp;
class SgaObject;
class SotStorage;
struct GalleryObject;
class FmFormModel;
class SdrModel;
class GalleryTheme;
class GalleryThemeEntry;
class Graphic;
class GfxLink;

class GalleryFileStorage
{
public:
    virtual ~GalleryFileStorage() = 0;

    virtual bool implWrite(const GalleryTheme& rTheme, const GalleryThemeEntry* pThm) = 0;

    virtual void insertObject(const SgaObject& rObj, GalleryObject* pFoundEntry,
                              sal_uInt32& rInsertPos)
        = 0;
    virtual void removeObject(std::unique_ptr<GalleryObject>& pEntry) = 0;

    virtual std::unique_ptr<SgaObject> implReadSgaObject(GalleryObject const* pEntry) = 0;
    virtual bool implWriteSgaObject(const SgaObject& rObj, sal_uInt32 nPos,
                                    GalleryObject* pExistentEntry)
        = 0;

    virtual bool readModel(const GalleryObject* pObject, SdrModel& rModel) = 0;
    virtual SgaObjectSvDraw insertModel(const FmFormModel& rModel, const INetURLObject& rUserURL)
        = 0;

    virtual bool readModelStream(const GalleryObject* pObject,
                                 tools::SvRef<SotStorageStream> const& rxModelStream)
        = 0;
    virtual SgaObjectSvDraw insertModelStream(const tools::SvRef<SotStorageStream>& rxModelStream,
                                              const INetURLObject& rUserURL)
        = 0;

    virtual INetURLObject implCreateUniqueURL(SgaObjKind eObjKind, const INetURLObject& rUserURL,
                                              ConvertDataFormat nFormat
                                              = ConvertDataFormat::Unknown)
        = 0;

    virtual SgaObjectBmp insertGraphic(const Graphic& rGraphic, const GfxLink& aGfxLink,
                                       ConvertDataFormat& nExportFormat,
                                       const INetURLObject& rUserURL)
        = 0;

    virtual SgaObjectSvDraw updateSvDrawObject(GalleryObject* pEntry) = 0;

    virtual void updateTheme() = 0;

    virtual SvStream& writeGalleryTheme(SvStream& rOStm, const GalleryTheme& rTheme,
                                        const GalleryThemeEntry* pThm)
        = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

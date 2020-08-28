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
#include <svx/gallerybinaryengine.hxx>
#include <svx/gallerystoragelocations.hxx>
#include <svx/galleryfilestorageentry.hxx>

class GalleryStorageLocations;

class GalleryBinaryEngineEntry : public GalleryFileStorageEntry
{
private:
    GalleryStorageLocations& m_rGalleryStorageLocations;

public:
    GalleryBinaryEngineEntry(GalleryStorageLocations& rGalleryStorageLocations);
    static void CreateUniqueURL(const INetURLObject& rBaseURL, INetURLObject& aURL);

    OUString ReadStrFromIni(const OUString& aKeyName);

    const INetURLObject& GetThmURL() const { return m_rGalleryStorageLocations.GetThmURL(); }
    const INetURLObject& GetSdgURL() const { return m_rGalleryStorageLocations.GetSdgURL(); }
    const INetURLObject& GetSdvURL() const { return m_rGalleryStorageLocations.GetSdvURL(); }
    const INetURLObject& GetStrURL() const { return m_rGalleryStorageLocations.GetStrURL(); }

    static GalleryThemeEntry* CreateThemeEntry(const INetURLObject& rURL, bool bReadOnly);

    void removeTheme();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

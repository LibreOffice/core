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

#include <svx/svxdllapi.h>
#include <svx/galmisc.hxx>
#include <svx/galleryobjectxmlstorage.hxx>
#include <svx/galleryobjectbinarystorage.hxx>

#include <vcl/bitmapex.hxx>
#include <tools/urlobj.hxx>
#include <memory>
#include <vector>

struct GalleryObject
{
    std::unique_ptr<GalleryObjectStorage> m_pGalleryObjectStorage;
    sal_uInt32 nOffset;
    SgaObjKind eObjKind;
    bool mbDelete;

    //UI visualization buffering
    BitmapEx maPreviewBitmapEx;
    Size maPreparedSize;
    OUString maTitle;
    OUString maPath;

    const INetURLObject& getURL() const { return m_pGalleryObjectStorage->getURL(); }
};

class SVXCORE_DLLPUBLIC GalleryObjectCollection
{
private:
    std::vector<std::unique_ptr<GalleryObject>> m_aObjectList;

public:
    GalleryObjectCollection();
    std::vector<std::unique_ptr<GalleryObject>>& getObjectList() { return m_aObjectList; }
    std::unique_ptr<GalleryObject>& get(sal_uInt32 nPos) { return m_aObjectList[nPos]; }
    const std::unique_ptr<GalleryObject>& get(sal_uInt32 nPos) const { return m_aObjectList[nPos]; }
    sal_uInt32 size() const { return m_aObjectList.size(); }

    const GalleryObject* searchObjectWithURL(const INetURLObject& rURL);
    const GalleryObject* getForPosition(sal_uInt32 nPos) const;
    sal_uInt32 searchPosWithObject(const GalleryObject* pObj);
    const INetURLObject& getURLForPosition(sal_uInt32 nPos) const;

    void clear();

    GalleryObjectCollection(GalleryObjectCollection const&) = delete;
    void operator=(GalleryObjectCollection const&) = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

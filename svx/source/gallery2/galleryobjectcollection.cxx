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

#include <galobj.hxx>
#include <svx/galleryobjectcollection.hxx>

GalleryObjectCollection::GalleryObjectCollection() {}

void GalleryObjectCollection::clear() { m_aObjectList.clear(); }

const GalleryObject* GalleryObjectCollection::searchObjectWithURL(const INetURLObject& rURL)
{
    for (auto const& pObject : m_aObjectList)
    {
        if (pObject->getURL() == rURL)
            return pObject.get();
    }
    return nullptr;
}

sal_uInt32 GalleryObjectCollection::searchPosWithObject(const GalleryObject* pObj)
{
    for (sal_uInt32 i = 0, n = size(); i < n; ++i)
        if (pObj == get(i).get())
            return i;
    return SAL_MAX_UINT32;
}

const GalleryObject* GalleryObjectCollection::getForPosition(sal_uInt32 nPos) const
{
    if (nPos < size())
        return get(nPos).get();
    return nullptr;
}

const INetURLObject& GalleryObjectCollection::getURLForPosition(sal_uInt32 nPos) const
{
    if (nPos < size())
        return get(nPos)->getURL();
    INetURLObject* aInvalidURL = new INetURLObject();
    return *aInvalidURL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

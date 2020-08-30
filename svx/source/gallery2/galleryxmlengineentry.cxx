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

#include <svx/galleryxmlengineentry.hxx>
#include <svx/galleryxmlengine.hxx>
#include <svx/galleryobjectcollection.hxx>
#include <svx/galleryxmlstoragelocations.hxx>

#include <memory>

GalleryXMLEngineEntry::GalleryXMLEngineEntry() {}

std::unique_ptr<GalleryXMLEngine> GalleryXMLEngineEntry::createGalleryStorageEngine(
    GalleryObjectCollection& mrGalleryObjectCollection, bool& bReadOnly)
{
    return std::make_unique<GalleryXMLEngine>(*mpGalleryStorageLocations, mrGalleryObjectCollection,
                                              bReadOnly);
}

void GalleryXMLEngineEntry::setStorageLocations(INetURLObject& rURL)
{
    mpGalleryStorageLocations->SetStorageLocations(rURL);
}

void GalleryXMLEngineEntry::removeTheme()
{
    INetURLObject aXMLURL(mpGalleryStorageLocations->getXMLURL());
    INetURLObject aMimeTypeURL(mpGalleryStorageLocations->getMimeTypeURL());

    KillFile(aXMLURL);
    KillFile(aMimeTypeURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

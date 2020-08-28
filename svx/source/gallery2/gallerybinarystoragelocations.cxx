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

#include <svx/gallerybinarystoragelocations.hxx>
#include <svx/galmisc.hxx>

INetURLObject GalleryBinaryStorageLocations::ImplGetURLIgnoreCase(const INetURLObject& rURL)
{
    INetURLObject aURL(rURL);

    // check original file name
    if (!FileExists(aURL))
    {
        // check upper case file name
        aURL.setName(aURL.getName().toAsciiUpperCase());

        if (!FileExists(aURL))
        {
            // check lower case file name
            aURL.setName(aURL.getName().toAsciiLowerCase());
        }
    }

    return aURL;
}

void GalleryBinaryStorageLocations::SetThmExtension(INetURLObject& aURL)
{
    aURL.setExtension("thm");
    maThmURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryStorageLocations::SetSdgExtension(INetURLObject& aURL)
{
    aURL.setExtension("sdg");
    maSdgURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryStorageLocations::SetSdvExtension(INetURLObject& aURL)
{
    aURL.setExtension("sdv");
    maSdvURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryStorageLocations::SetStrExtension(INetURLObject& aURL)
{
    aURL.setExtension("str");
    maStrURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryStorageLocations::SetStorageLocations(INetURLObject& rURL)
{
    SetThmExtension(rURL);
    SetSdgExtension(rURL);
    SetSdvExtension(rURL);
    SetStrExtension(rURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

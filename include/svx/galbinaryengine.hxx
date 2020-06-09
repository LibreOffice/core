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

#ifndef INCLUDED_SVX_GALBINARYENGINE_HXX
#define INCLUDED_SVX_GALBINARYENGINE_HXX

#include <rtl/ustring.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svx/svxdllapi.h>
#include <tools/urlobj.hxx>

#include <cstdio>
#include <memory>
#include <vector>

class Gallery;
class GalleryTheme;

class SVXCORE_DLLPUBLIC GalleryBinaryEngine
{
    friend class Gallery;
    friend class GalleryTheme;

private:
    INetURLObject aThmURL;
    INetURLObject aSdgURL;
    INetURLObject aSdvURL;
    INetURLObject aStrURL;
    static INetURLObject ImplGetURLIgnoreCase(const INetURLObject& rURL);

public:
    SAL_DLLPRIVATE GalleryBinaryEngine(bool bCreateUniqueURL, const INetURLObject& rBaseURL);

    static OUString ReadStrFromIni(const OUString& aKeyName);

    const INetURLObject& GetThmURL() const { return aThmURL; }
    const INetURLObject& GetSdgURL() const { return aSdgURL; }
    const INetURLObject& GetSdvURL() const { return aSdvURL; }
    const INetURLObject& GetStrURL() const { return aStrURL; }
};

#endif // INCLUDED_SVX_GALBINARYENGINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

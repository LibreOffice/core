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
#ifndef INCLUDED_VCL_GDIMETAFILETOOLS_HXX
#define INCLUDED_VCL_GDIMETAFILETOOLS_HXX

#include <config_options.h>
#include <vcl/dllapi.h>

class GDIMetaFile;

// #i121267# Added tooling to be able to support old exporters which are based on
// metafiles as graphic content, but do not implement using the contained clip
// regions.
// The given metafile will internally clip it's graphic content against the
// included clip regions so that it is safe to ignore clip actions there. This
// is not done completely, but implemented and extended as needed (on demand)
// since all this is a workaround; the better and long term solution will be to
// reimplement these im/exports to use primitives and not metafiles as base
// information.

VCL_DLLPUBLIC void clipMetafileContentAgainstOwnRegions(GDIMetaFile& rSource);

// Allow to check if a Metafile contains clipping or not

VCL_DLLPUBLIC bool usesClipActions(const GDIMetaFile& rSource);

// hook to access metafile members in classes of modules above vcl. Currently
// used in MetafilePrimitive2D to be able to access the local Metafile member
// e.g. from vcl module
class UNLESS_MERGELIBS(VCL_DLLPUBLIC) SAL_LOPLUGIN_ANNOTATE("crosscast") MetafileAccessor
{
public:
    virtual void accessMetafile(GDIMetaFile& rTargetMetafile) const = 0;
    virtual ~MetafileAccessor();
};

#endif // INCLUDED_VCL_GDIMETAFILETOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

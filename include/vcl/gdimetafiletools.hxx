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
#ifndef _SV_GDIMETAFILETOOLS_HXX
#define _SV_GDIMETAFILETOOLS_HXX

#include <vcl/gdimtf.hxx>

//////////////////////////////////////////////////////////////////////////////
// #i121267# Added tooling to be able to support old exporters which are based on
// metafiles as graphic content, but do not implement using the contained clip
// regions.
// The given metafile will internall yclip it's graphic content against the
// included clip regions so that it is safe to ignore clip actions there. This
// is not done completely, but implemented and extended as needed (on demand)
// since all this is a workarund; the better and long term solution will be to
// reimplement these im/exports to use primitives and not metafiles as bese
// information.

void VCL_DLLPUBLIC clipMetafileContentAgainstOwnRegions(GDIMetaFile& rSource);

//////////////////////////////////////////////////////////////////////////////
// Allow to check if a Metafile contains clipping or not

bool VCL_DLLPUBLIC usesClipActions(const GDIMetaFile& rSource);

//////////////////////////////////////////////////////////////////////////////

#endif // _SV_GDIMETAFILETOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

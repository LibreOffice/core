// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config_features.h>

#include <avmedia/mediaitem.hxx>

#if !HAVE_FEATURE_AVMEDIA
SAL_DLLPUBLIC_EXPORT TypeId avmedia::MediaItem::StaticType() { return 0; }
#endif

// vim:set shiftwidth=4 softtabstop=4 expandtab:

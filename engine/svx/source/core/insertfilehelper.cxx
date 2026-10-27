/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <svx/insertfilehelper.hxx>

#include <editeng/sizeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svx/svxids.hrc>
#if HAVE_FEATURE_AVMEDIA
#include <avmedia/mediawindow.hxx>
#endif

namespace svx
{
void DispatchInsertFile(SfxDispatcher& rDispatcher, const vcl::InsertFile& rFile)
{
    if (rFile.eKind == vcl::InsertFileKind::Graphic)
    {
        SfxStringItem aFileName(SID_INSERT_GRAPHIC, rFile.aUrl);
        rDispatcher.ExecuteList(SID_INSERT_GRAPHIC, SfxCallMode::SYNCHRON, { &aFileName });
        return;
    }

#if HAVE_FEATURE_AVMEDIA
    SfxStringItem aUrl(SID_INSERT_AVMEDIA, rFile.aUrl);
    SvxSizeItem aSize(FN_PARAM_1, avmedia::MediaWindow::readVideoSize(rFile.aUrl));
    SfxBoolItem aLink(FN_PARAM_2, false);
    rDispatcher.ExecuteList(SID_INSERT_AVMEDIA, SfxCallMode::SYNCHRON, { &aUrl, &aSize, &aLink });
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

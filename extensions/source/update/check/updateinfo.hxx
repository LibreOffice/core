/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UPDATE_INFO_INCLUDED_
#define _UPDATE_INFO_INCLUDED_

#include <rtl/ustring.hxx>
#include <vector>

struct DownloadSource
{
    bool IsDirect;
    rtl::OUString URL;

    DownloadSource(bool bIsDirect, const rtl::OUString& aURL) : IsDirect(bIsDirect), URL(aURL) {};
    DownloadSource(const DownloadSource& ds) : IsDirect(ds.IsDirect), URL(ds.URL) {};

    DownloadSource & operator=( const DownloadSource & ds ) { IsDirect = ds.IsDirect; URL = ds.URL; return *this; };
};

struct ReleaseNote
{
    sal_uInt8 Pos;
    rtl::OUString URL;
    sal_uInt8 Pos2;
    rtl::OUString URL2;

    ReleaseNote(sal_uInt8 pos, const rtl::OUString aURL) : Pos(pos), URL(aURL), Pos2(0), URL2() {};
    ReleaseNote(sal_uInt8 pos, const rtl::OUString aURL, sal_uInt8 pos2, const rtl::OUString aURL2) : Pos(pos), URL(aURL), Pos2(pos2), URL2(aURL2) {};

    ReleaseNote(const ReleaseNote& rn) :Pos(rn.Pos), URL(rn.URL), Pos2(rn.Pos2), URL2(rn.URL2) {};
    ReleaseNote & operator=( const ReleaseNote& rn) { Pos=rn.Pos; URL=rn.URL; Pos2=rn.Pos2; URL2=rn.URL2; return *this; };
};

struct UpdateInfo
{
    rtl::OUString BuildId;
    rtl::OUString Version;
    rtl::OUString Description;
    std::vector< DownloadSource > Sources;
    std::vector< ReleaseNote > ReleaseNotes;

    UpdateInfo() : BuildId(), Version(), Description(), Sources(), ReleaseNotes() {};
    UpdateInfo(const UpdateInfo& ui) : BuildId(ui.BuildId), Version(ui.Version), Description(ui.Description), Sources(ui.Sources), ReleaseNotes(ui.ReleaseNotes) {};
    inline UpdateInfo & operator=( const UpdateInfo& ui );
};

UpdateInfo & UpdateInfo::operator=( const UpdateInfo& ui )
{
    BuildId = ui.BuildId;
    Version = ui.Version;
    Description = ui.Description;
    Sources = ui.Sources;
    ReleaseNotes = ui.ReleaseNotes;
    return *this;
}


// Returns the URL of the release note for the given position
rtl::OUString getReleaseNote(const UpdateInfo& rInfo, sal_uInt8 pos, bool autoDownloadEnabled=false);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

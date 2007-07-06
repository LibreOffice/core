/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updateinfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 14:38:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

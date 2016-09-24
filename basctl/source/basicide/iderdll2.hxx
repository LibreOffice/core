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

#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_IDERDLL2_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_IDERDLL2_HXX

class StarBASIC;
class SvxSearchItem;


#include <bastypes.hxx>
#include <bastype2.hxx>

#include <memory>

namespace basctl
{

class ExtraData
{
private:
    std::unique_ptr<SvxSearchItem> pSearchItem;

    LibInfo        aLibInfo;

    EntryDescriptor m_aLastEntryDesc;

    OUString        aAddLibPath;
    OUString        aAddLibFilter;

    bool            bChoosingMacro;
    bool            bShellInCriticalSection;

protected:
    DECL_STATIC_LINK_TYPED( ExtraData, GlobalBasicBreakHdl, StarBASIC *, sal_uInt16 );

public:
    ExtraData();
    ~ExtraData();

    LibInfo&         GetLibInfo () { return aLibInfo; }

    EntryDescriptor&  GetLastEntryDescriptor () { return m_aLastEntryDesc; }
    void              SetLastEntryDescriptor (EntryDescriptor& rDesc) { m_aLastEntryDesc = rDesc; }

    bool&             ChoosingMacro()          { return bChoosingMacro; }
    bool&             ShellInCriticalSection() { return bShellInCriticalSection; }

    SvxSearchItem&    GetSearchItem() const { return *pSearchItem; }
    void              SetSearchItem( const SvxSearchItem& rItem );

    const OUString&   GetAddLibPath() const   { return aAddLibPath; }
    void              SetAddLibPath( const OUString& rPath ) { aAddLibPath = rPath; }

    const OUString&   GetAddLibFilter() const { return aAddLibFilter; }
    void              SetAddLibFilter( const OUString& rFilter )  { aAddLibFilter = rFilter; }
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_IDERDLL2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

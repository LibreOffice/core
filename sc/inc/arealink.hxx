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

#ifndef SC_AREALINK_HXX
#define SC_AREALINK_HXX

#include "global.hxx"
#include "refreshtimer.hxx"
#include "address.hxx"
#include <sfx2/lnkbase.hxx>
#include "scdllapi.h"

class SfxObjectShell;
struct AreaLink_Impl;

class SC_DLLPUBLIC ScAreaLink : public ::sfx2::SvBaseLink, public ScRefreshTimer
{
private:
    AreaLink_Impl*  pImpl;
    OUString        aFileName;
    OUString        aFilterName;
    OUString        aOptions;
    OUString        aSourceArea;
    ScRange         aDestArea;
    bool            bAddUndo;
    bool            bInCreate;
    bool            bDoInsert;      // is set to FALSE for first update
    bool FindExtRange( ScRange& rRange, ScDocument* pSrcDoc, const OUString& rAreaName );

public:
    TYPEINFO();
    ScAreaLink( SfxObjectShell* pShell, const OUString& rFile,
                    const OUString& rFilter, const OUString& rOpt,
                    const OUString& rArea, const ScRange& rDest, sal_uLong nRefresh );
    virtual ~ScAreaLink();

    virtual void Closed();
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue );

    virtual void    Edit( Window*, const Link& rEndEditHdl );

    sal_Bool    Refresh( const OUString& rNewFile, const OUString& rNewFilter,
                    const OUString& rNewArea, sal_uLong nNewRefresh );

    void    SetInCreate(bool bSet)                  { bInCreate = bSet; }
    void    SetDoInsert(bool bSet)                  { bDoInsert = bSet; }
    void    SetDestArea(const ScRange& rNew);
    void    SetSource(const OUString& rDoc, const OUString& rFlt, const OUString& rOpt,
                        const OUString& rArea);

    bool IsEqual( const OUString& rFile, const OUString& rFilter, const OUString& rOpt,
                  const OUString& rSource, const ScRange& rDest ) const;

    const OUString& GetFile() const         { return aFileName;     }
    const OUString& GetFilter() const       { return aFilterName;   }
    const OUString& GetOptions() const      { return aOptions;      }
    const OUString& GetSource() const       { return aSourceArea;   }
    const ScRange&  GetDestArea() const     { return aDestArea;     }

    DECL_LINK( RefreshHdl, void* );
    DECL_LINK( AreaEndEditHdl, void* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

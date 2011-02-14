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

#ifndef SC_AREALINK_HXX
#define SC_AREALINK_HXX

#include "global.hxx"
#include "refreshtimer.hxx"
#include "address.hxx"
#include <sfx2/lnkbase.hxx>
#include "scdllapi.h"

class ScDocShell;
class SfxObjectShell;
class AbstractScLinkedAreaDlg;
struct AreaLink_Impl;

class SC_DLLPUBLIC ScAreaLink : public ::sfx2::SvBaseLink, public ScRefreshTimer
{
private:
    AreaLink_Impl*  pImpl;
    String          aFileName;
    String          aFilterName;
    String          aOptions;
    String          aSourceArea;
    ScRange         aDestArea;
    sal_Bool            bAddUndo;
    sal_Bool            bInCreate;
    sal_Bool            bDoInsert;      // wird fuer das erste Update auf sal_False gesetzt

    sal_Bool        FindExtRange( ScRange& rRange, ScDocument* pSrcDoc, const String& rAreaName );

public:
    TYPEINFO();
    ScAreaLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt,
                    const String& rArea, const ScRange& rDest, sal_uLong nRefresh );
    virtual ~ScAreaLink();

    virtual void Closed();
    virtual void DataChanged( const String& rMimeType,
                              const ::com::sun::star::uno::Any & rValue );

    virtual void    Edit( Window*, const Link& rEndEditHdl );

    sal_Bool    Refresh( const String& rNewFile, const String& rNewFilter,
                    const String& rNewArea, sal_uLong nNewRefresh );

    void    SetInCreate(sal_Bool bSet)                  { bInCreate = bSet; }
    void    SetDoInsert(sal_Bool bSet)                  { bDoInsert = bSet; }
    void    SetDestArea(const ScRange& rNew);
    void    SetSource(const String& rDoc, const String& rFlt, const String& rOpt,
                        const String& rArea);

    sal_Bool    IsEqual( const String& rFile, const String& rFilter, const String& rOpt,
                        const String& rSource, const ScRange& rDest ) const;

    const String&   GetFile() const         { return aFileName;     }
    const String&   GetFilter() const       { return aFilterName;   }
    const String&   GetOptions() const      { return aOptions;      }
    const String&   GetSource() const       { return aSourceArea;   }
    const ScRange&  GetDestArea() const     { return aDestArea;     }

    DECL_LINK( RefreshHdl, ScAreaLink* );
    DECL_LINK( AreaEndEditHdl, void* );
};

#endif


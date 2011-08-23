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

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_REFRESHTIMER_HXX
#include "refreshtimer.hxx"
#endif

#ifndef _LNKBASE_HXX //autogen
#include <bf_so3/lnkbase.hxx>
#endif
namespace binfilter {

class ScDocShell;
class SfxObjectShell;

class ScAreaLink : public ::binfilter::SvBaseLink, public ScRefreshTimer
{
private:
    ScDocShell*	pDocShell;		// Container
    String		aFileName;
    String		aFilterName;
    String		aOptions;
    String		aSourceArea;
    ScRange		aDestArea;
    BOOL		bAddUndo;
    BOOL		bInCreate;
    BOOL		bDoInsert;		// wird fuer das erste Update auf FALSE gesetzt

    BOOL		FindExtRange( ScRange& rRange, ScDocument* pSrcDoc, const String& rAreaName );

public:
    TYPEINFO();
    ScAreaLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt,
                    const String& rArea, const ScRange& rDest, ULONG nRefresh );
    virtual ~ScAreaLink();

    virtual void DataChanged( const String& rMimeType,
                              const ::com::sun::star::uno::Any & rValue );


    BOOL	Refresh( const String& rNewFile, const String& rNewFilter,
                    const String& rNewArea, ULONG nNewRefresh );

    void	SetInCreate(BOOL bSet)					{ bInCreate = bSet; }
    void	SetDoInsert(BOOL bSet)					{ bDoInsert = bSet; }
    void	SetDestArea(const ScRange& rNew);


    const String&	GetFile() const			{ return aFileName;		}
    const String&	GetFilter() const		{ return aFilterName;	}
    const String&	GetOptions() const		{ return aOptions;		}
    const String&	GetSource() const		{ return aSourceArea;	}
    const ScRange&	GetDestArea() const		{ return aDestArea;		}

    DECL_LINK( RefreshHdl, ScAreaLink* );

};


} //namespace binfilter
#endif

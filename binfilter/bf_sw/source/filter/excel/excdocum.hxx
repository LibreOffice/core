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
#ifdef USED

#ifndef __EXCDOCUM_HXX__
#define __EXCDOCUM_HXX__

// auto strip #include <tools/string.hxx>
namespace binfilter {


// Q&D defines statt consts
#define MAX_TABS_EXCDOK 1024

class ExcDokument

// -----------------------------------------------------------------------
// beschreibt eine Excel-Tabelle
class ExcTabelle
    {
    private:
        friend ExcDokument;
        String				aName;
        USHORT				nCS;
        USHORT				nCE;
        USHORT				nRS;
        USHORT				nRE;
        ULONG				nStrPos;

        ExcTabelle( ULONG );							// mit Stream-Pos
        ExcTabelle( const String & );					// mit Name
        ExcTabelle( USHORT, USHORT, USHORT, USHORT );	// mit Range
    public:
        ExcTabelle();
        ~ExcTabelle();

        void GetRange( USHORT& rCS, USHORT& rCE, USHORT& rRS, USHORT& rRE ) const
            {
            rCS = nCS;
            rCE = nCE;
            rRS = nRS;
            rRE = nRE;
            }

        ULONG GetStrPos( void ) const
            {
            return nStrPos;
            }

        const String &GetName( void ) const
            {
            return aName;
            }
    };

// -----------------------------------------------------------------------
// beschreibt ein Excel-Dokument
class ExcDokument
    {
    private:
        ExcTabelle			**ppTabellen;
        INT32				nAktGetNext;		// aktueller Index fuer GetNext
        USHORT				nAktTabIndex;		// aktuelle Tabelle in ppTabellen
    public:
        ExcDokument();
        ~ExcDokument();

#ifdef USED
        USHORT				GetAnzTabs( void ) const;
        ExcTabelle			*operator[]( USHORT );
        void				NewTab( const String &rName,
                                ULONG nStrPos = 0xFFFFFFFF );
        void				Set( USHORT nNewCS, USHORT nNewCE,
                                USHORT nNewRS, USHORT nNewRE,
                                USHORT nIndex = 0xFFFF );
        ExcTabelle			*GetNextTab( void );
        ExcTabelle			*GetFirstTab( void );
        void				Set( ULONG nStrPos, USHORT nIndex = 0xFFFF );
        void				Set( const String &rName,
                                USHORT nIndex = 0xFFFF );
#endif
    };

#endif

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

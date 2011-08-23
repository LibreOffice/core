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

#ifndef _SC_BCASLOT_HXX
#define _SC_BCASLOT_HXX

#include <bf_svtools/brdcst.hxx>
#include <bf_svtools/svarray.hxx>

#include "global.hxx"
#include "brdcst.hxx"
namespace binfilter {

class ScBroadcastArea;
class ScBroadcastAreaList;

#define BCA_INITGROWSIZE 16
typedef ScBroadcastArea* ScBroadcastAreaPtr;
SV_DECL_PTRARR_SORT( ScBroadcastAreas, ScBroadcastAreaPtr,
    BCA_INITGROWSIZE, BCA_INITGROWSIZE )//STRIP008 ;

// wird in einem nach Objekten sortierten SV_PTRARR_SORT benutzt
class ScBroadcastArea : public ScRange, public SfxBroadcaster
{
private:
    ScBroadcastArea*	pUpdateChainNext;
    USHORT				nRefCount;
    BOOL				bInUpdateChain;

public:
            ScBroadcastArea( const ScRange& rRange )
                : ScRange( rRange ), SfxBroadcaster(), pUpdateChainNext( NULL ),
                nRefCount( 0 ), bInUpdateChain( FALSE ) {}
    inline void			UpdateRange( const ScRange& rNewRange )
            { aStart = rNewRange.aStart; aEnd = rNewRange.aEnd; }
    inline ScAddress	GetStart() const { return aStart; }
    inline ScAddress	GetEnd() const { return aEnd; }
    inline void			IncRef() { ++nRefCount; }
    inline USHORT		DecRef() { return --nRefCount; }
    inline ScBroadcastArea* GetUpdateChainNext() const { return pUpdateChainNext; }
    inline void			SetUpdateChainNext( ScBroadcastArea* p ) { pUpdateChainNext = p; }
    inline BOOL			IsInUpdateChain() const { return bInUpdateChain; }
    inline void			SetInUpdateChain( BOOL b ) { bInUpdateChain = b; }

            // zur Sortierung wird die linke obere Ecke herangezogen,
            // ist diese gleich, zaehlt auch die rechte untere Ecke
    BOOL	operator < ( const ScBroadcastArea& rArea ) const
                { return aStart < rArea.aStart ||
                    (aStart == rArea.aStart && aEnd < rArea.aEnd) ; }
    BOOL	operator ==( const ScBroadcastArea& rArea ) const
                { return aStart == rArea.aStart && aEnd == rArea.aEnd; }
};
class ScBroadcastAreaSlotMachine;

// Sammlung von BroadcastAreas
class ScBroadcastAreaSlot
{
private:
    ScBroadcastAreas*	pBroadcastAreaTbl;
    ScBroadcastArea*	pTmpSeekBroadcastArea;			// fuer Seek_Entry
    ScDocument*			pDoc;
    ScBroadcastAreaSlotMachine* pBASM;

    USHORT				FindBroadcastArea( const ScRange& rRange ) const;
    ScBroadcastArea*	GetBroadcastArea( const ScRange& rRange ) const;

public:
                        ScBroadcastAreaSlot( ScDocument* pDoc,
                                        ScBroadcastAreaSlotMachine* pBASM );
                        ~ScBroadcastAreaSlot();
    const ScBroadcastAreas&	GetBroadcastAreas() const
                                            { return *pBroadcastAreaTbl; }
    void				StartListeningArea( const ScRange& rRange,
                                            SfxListener* pListener,
                                            ScBroadcastArea*& rpArea );
    void				EndListeningArea( const ScRange& rRange,
                                            SfxListener* pListener,
                                            ScBroadcastArea*& rpArea );
    BOOL				AreaBroadcast( const ScHint& rHint ) const;
        // return: mindestens ein Broadcast gewesen
    BOOL				AreaBroadcastInRange( const ScRange& rRange,
                                              const ScHint& rHint ) const;
/*N*/ 	void				DelBroadcastAreasInRange( const ScRange& rRange );
};


/*
    BroadcastAreaSlots und deren Verwaltung, einmal je Dokument

    +---+---+
    | 0 | 2 |	Anordnung Cols/Rows
    +---+---+
    | 1 | 3 |
    +---+---+
 */

class  ScBroadcastAreaSlotMachine
{
private:
    ScBroadcastAreaSlot**	ppSlots;
    ScDocument*			pDoc;
    ScBroadcastAreaList*	pBCAlwaysList;	// fuer den RC_ALWAYS Spezialbereich
    ScBroadcastArea*	pUpdateChain;
    ScBroadcastArea*	pEOUpdateChain;

    inline USHORT		ComputeSlotOffset( const ScAddress& rAddress ) const;
    void				ComputeAreaPoints( const ScRange& rRange,
                                            USHORT& nStart, USHORT& nEnd,
                                            USHORT& nRowBreak ) const;

public:
                        ScBroadcastAreaSlotMachine( ScDocument* pDoc );
                        ~ScBroadcastAreaSlotMachine();
    void				StartListeningArea( const ScRange& rRange,
                                            SfxListener* pListener );
    void				EndListeningArea( const ScRange& rRange,
                                            SfxListener* pListener );
    BOOL				AreaBroadcast( const ScHint& rHint ) const;
        // return: mindestens ein Broadcast gewesen
    BOOL                AreaBroadcastInRange( const ScRange& rRange, const ScHint& rHint ) const;
/*N*/ 	void				DelBroadcastAreasInRange( const ScRange& rRange );
    inline ScBroadcastArea* GetUpdateChain() const { return pUpdateChain; }
    inline void SetUpdateChain( ScBroadcastArea* p ) { pUpdateChain = p; }
    inline ScBroadcastArea* GetEOUpdateChain() const { return pEOUpdateChain; }
    inline void SetEOUpdateChain( ScBroadcastArea* p ) { pEOUpdateChain = p; }
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

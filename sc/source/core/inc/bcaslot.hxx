/*************************************************************************
 *
 *  $RCSfile: bcaslot.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SC_BCASLOT_HXX
#define _SC_BCASLOT_HXX

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#include "global.hxx"
#include "brdcst.hxx"

class ScBroadcastArea;
class ScBroadcastAreaList;

#define BCA_INITGROWSIZE 16
typedef ScBroadcastArea* ScBroadcastAreaPtr;
SV_DECL_PTRARR_SORT( ScBroadcastAreas, ScBroadcastAreaPtr,
    BCA_INITGROWSIZE, BCA_INITGROWSIZE );

// wird in einem nach Objekten sortierten SV_PTRARR_SORT benutzt
class ScBroadcastArea : public ScRange, public SfxBroadcaster
{
private:
    ScBroadcastArea*    pUpdateChainNext;
    USHORT              nRefCount;
    BOOL                bInUpdateChain;

public:
            ScBroadcastArea( const ScRange& rRange )
                : ScRange( rRange ), SfxBroadcaster(), pUpdateChainNext( NULL ),
                nRefCount( 0 ), bInUpdateChain( FALSE ) {}
    inline void         UpdateRange( const ScRange& rNewRange )
            { aStart = rNewRange.aStart; aEnd = rNewRange.aEnd; }
    inline ScAddress    GetStart() const { return aStart; }
    inline ScAddress    GetEnd() const { return aEnd; }
    inline void         IncRef() { ++nRefCount; }
    inline USHORT       DecRef() { return --nRefCount; }
    inline ScBroadcastArea* GetUpdateChainNext() const { return pUpdateChainNext; }
    inline void         SetUpdateChainNext( ScBroadcastArea* p ) { pUpdateChainNext = p; }
    inline BOOL         IsInUpdateChain() const { return bInUpdateChain; }
    inline void         SetInUpdateChain( BOOL b ) { bInUpdateChain = b; }

            // zur Sortierung wird die linke obere Ecke herangezogen,
            // ist diese gleich, zaehlt auch die rechte untere Ecke
    BOOL    operator < ( const ScBroadcastArea& rArea ) const
                { return aStart < rArea.aStart ||
                    (aStart == rArea.aStart && aEnd < rArea.aEnd) ; }
    BOOL    operator ==( const ScBroadcastArea& rArea ) const
                { return aStart == rArea.aStart && aEnd == rArea.aEnd; }
};
class ScBroadcastAreaSlotMachine;

// Sammlung von BroadcastAreas
class ScBroadcastAreaSlot
{
private:
    ScBroadcastAreas*   pBroadcastAreaTbl;
    ScBroadcastArea*    pTmpSeekBroadcastArea;          // fuer Seek_Entry
    ScDocument*         pDoc;
    ScBroadcastAreaSlotMachine* pBASM;

    USHORT              FindBroadcastArea( const ScRange& rRange ) const;
    ScBroadcastArea*    GetBroadcastArea( const ScRange& rRange ) const;

public:
                        ScBroadcastAreaSlot( ScDocument* pDoc,
                                        ScBroadcastAreaSlotMachine* pBASM );
                        ~ScBroadcastAreaSlot();
    const ScBroadcastAreas& GetBroadcastAreas() const
                                            { return *pBroadcastAreaTbl; }
    void                StartListeningArea( const ScRange& rRange,
                                            SfxListener* pListener,
                                            ScBroadcastArea*& rpArea );
    void                EndListeningArea( const ScRange& rRange,
                                            SfxListener* pListener,
                                            ScBroadcastArea*& rpArea );
    BOOL                AreaBroadcast( const ScAddress& rAddress,
                                        const ScHint& rHint ) const;
        // return: mindestens ein Broadcast gewesen
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateRemove( UpdateRefMode eUpdateRefMode,
                                        const ScRange& rRange,
                                        short nDx, short nDy, short nDz );
    void                UpdateInsert( ScBroadcastArea* pArea );
};


/*
    BroadcastAreaSlots und deren Verwaltung, einmal je Dokument

    +---+---+
    | 0 | 2 |   Anordnung Cols/Rows
    +---+---+
    | 1 | 3 |
    +---+---+
 */

class  ScBroadcastAreaSlotMachine
{
private:
    ScBroadcastAreaSlot**   ppSlots;
    ScDocument*         pDoc;
    ScBroadcastAreaList*    pBCAlwaysList;  // fuer den RC_ALWAYS Spezialbereich
    ScBroadcastArea*    pUpdateChain;
    ScBroadcastArea*    pEOUpdateChain;

    inline USHORT       ComputeSlotOffset( const ScAddress& rAddress ) const;
    void                ComputeAreaPoints( const ScRange& rRange,
                                            USHORT& nStart, USHORT& nEnd,
                                            USHORT& nRowBreak ) const;

public:
                        ScBroadcastAreaSlotMachine( ScDocument* pDoc );
                        ~ScBroadcastAreaSlotMachine();
    void                StartListeningArea( const ScRange& rRange,
                                            SfxListener* pListener );
    void                EndListeningArea( const ScRange& rRange,
                                            SfxListener* pListener );
    BOOL                AreaBroadcast( const ScAddress& rAddress,
                                        const ScHint& rHint ) const;
        // return: mindestens ein Broadcast gewesen
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
                                            const ScRange& rRange,
                                            short nDx, short nDy, short nDz );
    inline ScBroadcastArea* GetUpdateChain() const { return pUpdateChain; }
    inline void SetUpdateChain( ScBroadcastArea* p ) { pUpdateChain = p; }
    inline ScBroadcastArea* GetEOUpdateChain() const { return pEOUpdateChain; }
    inline void SetEOUpdateChain( ScBroadcastArea* p ) { pEOUpdateChain = p; }
};



#endif

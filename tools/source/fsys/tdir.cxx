/*************************************************************************
 *
 *  $RCSfile: tdir.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-06 18:28:29 $
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

#define _DIR_CXX

#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef _LIST_HXX
#include <list.hxx>
#endif

#ifndef _COMPED_HXX
#include "comdep.hxx"
#endif
#ifndef _FSYS_HXX
#include <fsys.hxx>
#endif


DBG_NAME( Dir );

DECLARE_LIST( DirEntryList, DirEntry* );
DECLARE_LIST( FSysSortList, FSysSort* );
DECLARE_LIST( FileStatList, FileStat* );

#define APPEND (USHORT) 65535

/*************************************************************************
|*
|*    Dir::InsertPointReached()
|*
|*    Beschreibung      stellt fest, ob eingefuegt werden musz
|*    Ersterstellung    MA 05.11.91
|*    Letzte Aenderung  MI 05.02.92
|*
*************************************************************************/

BOOL Dir::ImpInsertPointReached( const DirEntry& rNewEntry,
                                 const FileStat& rNewStat,
                                 ULONG nCurPos, ULONG nSortIndex ) const
{
#define VALUE( nKindFlags ) \
    ( ( FSYS_KIND_FILE | FSYS_KIND_DIR | FSYS_KIND_DEV | \
        FSYS_KIND_CHAR | FSYS_KIND_BLOCK ) & nKindFlags )

    // einfache Dinge erfordern einfache Loesungen
    if ( !pLst->Count() )
        return TRUE;

    FSysSort  nSort      = *( pSortLst->GetObject( nSortIndex ) );
    FileStat *pOldStat   = NULL;
    DirEntry *pCurLstObj = pLst->GetObject( nCurPos );
    if ( pStatLst )
        pOldStat = pStatLst->GetObject( nCurPos );

    switch( nSort )
    {
        case  FSYS_SORT_NAME:
        case (FSYS_SORT_NAME | FSYS_SORT_ASCENDING):
            if ( pCurLstObj->aName > rNewEntry.aName )
                return TRUE;
            if ( !(pCurLstObj->aName == rNewEntry.aName) )
                return FALSE;
            break;
        case (FSYS_SORT_NAME | FSYS_SORT_DESCENDING):
            if ( pCurLstObj->aName < rNewEntry.aName )
                return TRUE;
            if ( !(pCurLstObj->aName == rNewEntry.aName) )
                return FALSE;
            break;

        case  FSYS_SORT_EXT:
        case (FSYS_SORT_EXT | FSYS_SORT_ASCENDING):
        {
            if ( pCurLstObj->GetExtension() > rNewEntry.GetExtension() )
                return TRUE;
            if ( !(pCurLstObj->GetExtension() == rNewEntry.GetExtension()) )
                return FALSE;
            break;
        }
        case (FSYS_SORT_EXT | FSYS_SORT_DESCENDING):
        {
            if ( pCurLstObj->GetExtension() < rNewEntry.GetExtension() )
                return TRUE;
            if ( !(pCurLstObj->GetExtension() == rNewEntry.GetExtension()) )
                return FALSE;
            break;
        }

        case  FSYS_SORT_KIND:
        case (FSYS_SORT_KIND | FSYS_SORT_ASCENDING ):
            if ( VALUE(pOldStat->nKindFlags) > VALUE(rNewStat.nKindFlags) )
                return TRUE;
            if ( !(VALUE(pOldStat->nKindFlags) == VALUE(rNewStat.nKindFlags)) )
                return FALSE;
            break;
        case (FSYS_SORT_KIND | FSYS_SORT_DESCENDING):
            if ( VALUE(pOldStat->nKindFlags) < VALUE(rNewStat.nKindFlags) )
                return TRUE;
            if ( !(VALUE(pOldStat->nKindFlags) == VALUE(rNewStat.nKindFlags)) )
                return FALSE;
            break;

        case  FSYS_SORT_SIZE:
        case (FSYS_SORT_SIZE | FSYS_SORT_ASCENDING):
            if ( pOldStat->nSize > rNewStat.nSize )
                return TRUE;
            if ( !(pOldStat->nSize == rNewStat.nSize) )
                return FALSE;
            break;
        case (FSYS_SORT_SIZE | FSYS_SORT_DESCENDING):
            if ( pOldStat->nSize < rNewStat.nSize )
                return TRUE;
            if ( !(pOldStat->nSize == rNewStat.nSize) )
                return FALSE;
            break;

        case  FSYS_SORT_MODIFYED:
        case (FSYS_SORT_MODIFYED | FSYS_SORT_ASCENDING):
            if ( (pOldStat->aDateModified >= rNewStat.aDateModified) &&
                 (pOldStat->aTimeModified >  rNewStat.aTimeModified) )
                 return TRUE;
            if ( !((pOldStat->aDateModified == rNewStat.aDateModified) &&
                   (pOldStat->aTimeModified == rNewStat.aTimeModified)) )
                return FALSE;
            break;
        case (FSYS_SORT_MODIFYED | FSYS_SORT_DESCENDING):
            if ( (pOldStat->aDateModified <= rNewStat.aDateModified) &&
                 (pOldStat->aTimeModified <  rNewStat.aTimeModified) )
                 return TRUE;
            if ( !((pOldStat->aDateModified == rNewStat.aDateModified) &&
                   (pOldStat->aTimeModified == rNewStat.aTimeModified)) )
                return FALSE;
            break;

        case  FSYS_SORT_CREATED:
        case (FSYS_SORT_CREATED | FSYS_SORT_ASCENDING):
            if ( (pOldStat->aDateCreated >= rNewStat.aDateCreated) &&
                 (pOldStat->aTimeCreated >  rNewStat.aTimeCreated) )
                 return TRUE;
            if ( !((pOldStat->aDateCreated == rNewStat.aDateCreated) &&
                   (pOldStat->aTimeCreated == rNewStat.aTimeCreated)) )
                return FALSE;
            break;
        case (FSYS_SORT_CREATED | FSYS_SORT_DESCENDING):
            if ( (pOldStat->aDateCreated <= rNewStat.aDateCreated) &&
                 (pOldStat->aTimeCreated <  rNewStat.aTimeCreated) )
                 return TRUE;
            if ( !((pOldStat->aDateCreated == rNewStat.aDateCreated) &&
                   (pOldStat->aTimeCreated == rNewStat.aTimeCreated)) )
                return FALSE;
            break;

        case  FSYS_SORT_ACCESSED:
        case (FSYS_SORT_ACCESSED | FSYS_SORT_ASCENDING):
            if ( (pOldStat->aDateAccessed >= rNewStat.aDateAccessed) &&
                 (pOldStat->aTimeAccessed >  rNewStat.aTimeAccessed) )
                 return TRUE;
            if ( !((pOldStat->aDateAccessed == rNewStat.aDateAccessed) &&
                   (pOldStat->aTimeAccessed == rNewStat.aTimeAccessed)) )
                return FALSE;
            break;
        case (FSYS_SORT_ACCESSED | FSYS_SORT_DESCENDING):
            if ( (pOldStat->aDateAccessed <= rNewStat.aDateAccessed) &&
                 (pOldStat->aTimeAccessed <  rNewStat.aTimeAccessed) )
                 return TRUE;
            if ( !((pOldStat->aDateAccessed == rNewStat.aDateAccessed) &&
                   (pOldStat->aTimeAccessed == rNewStat.aTimeAccessed)) )
                return FALSE;
            break;
        default: /* Kann nicht sein */;
    }

    if ( nSortIndex == ( pSortLst->Count() - 1 ) )
        return TRUE;
    else
        //Rekursion
        return ImpInsertPointReached( rNewEntry, rNewStat,
                                      nCurPos, nSortIndex + 1 );
#undef VALUE
}

/*************************************************************************
|*
|*    Dir::ImpSortedInsert()
|*
|*    Beschreibung      fuegt sortiert ein
|*    Ersterstellung    MA 05.11.91
|*    Letzte Aenderung  MA 03.12.91
|*
*************************************************************************/

void Dir::ImpSortedInsert( const DirEntry *pNewEntry, const FileStat *pNewStat )
{
    //Sonderfall, keine Sortierung gewuenscht.
    if ( !pSortLst ) {
        pLst->Insert( (DirEntry*)pNewEntry, APPEND );
        return;
    }

    pLst->First();
    do {
        if ( ImpInsertPointReached( *pNewEntry, *pNewStat, pLst->GetCurPos(),
                                    (ULONG)0  ) )
        {
            if ( pStatLst )
                pStatLst->Insert( (FileStat*)pNewStat, pLst->GetCurPos() );
            pLst->Insert( (DirEntry*)pNewEntry );
            return;
        }
    } while( pLst->Next() );

    if ( pStatLst )
        pStatLst->Insert( (FileStat*)pNewStat, APPEND );
    pLst->Insert( (DirEntry*)pNewEntry, APPEND );
}

/*************************************************************************
|*
|*    Dir::Construct()
|*
|*    Beschreibung      gemeinsame Implementation der Ctoren
|*    Ersterstellung    MI 02.06.93
|*    Letzte Aenderung  MI 02.06.93
|*
*************************************************************************/

void Dir::Construct( DirEntryKind nKindFlags )
{
    pLst     = NULL;
    pSortLst = NULL;
    pStatLst = NULL;
    eAttrMask = nKindFlags;
    ByteString aTempName( GetName(), osl_getThreadTextEncoding() );
    if ( aTempName.Search( "*" ) != STRING_NOTFOUND ||
         aTempName.Search( "?" ) != STRING_NOTFOUND )
#if defined( WNT ) && !defined( WTC )
    {
        ByteString aTStr(CutName(), osl_getThreadTextEncoding());
        char* pBuffer = new char[aTStr.Len()+1];
        strcpy( pBuffer, aTStr.GetBuffer() );
        CharLowerBuff( pBuffer, aTStr.Len() );
        aNameMask = WildCard( String(pBuffer, osl_getThreadTextEncoding()), ';' );
        delete pBuffer;
    }
#else
    aNameMask = WildCard( CutName(), ';' );
#endif
    else
        aNameMask = String("*", osl_getThreadTextEncoding());
}

/*************************************************************************
|*
|*    Dir::Update()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 16.05.91
|*    Letzte Aenderung  MI 19.09.96
|*
*************************************************************************/

BOOL Dir::Update()
{
    Reset();
    return Scan( USHRT_MAX ) > 0;
}

/*************************************************************************
|*
|*    Dir::Reset()
|*
|*    Beschreibung
|*    Ersterstellung    MI 22.10.96
|*    Letzte Aenderung  MI 22.10.96
|*
*************************************************************************/

void Dir::Reset()
{
    // ggf. alten Reader l"oschen
    if ( pReader && pReader->bInUse )
        DELETEZ(pReader);

    // alle DirEntries aus der Liste entfernen und deren Speicher freigeben
    if ( pLst )
    {
        DirEntry* pEntry = pLst->First();
        while (pEntry)
        {
            DirEntry* pNext = pLst->Next();
            delete pEntry;
            pEntry = pNext;
        }
        pLst->Clear();
    }
    else
        pLst = new DirEntryList();

    //  Alte File-Stat's Loeschen
    if ( pStatLst )
    {
        //Erstmal die alten Loeschen
        FileStat* pEntry = pStatLst->First();
        while (pEntry)
        {
            FileStat*  pNext = pStatLst->Next();
            delete pEntry;
            pEntry = pNext;
        }
        pStatLst->Clear();
        delete pStatLst;
    }

    // Verlangen die Sortierkriterien FileStat's?
    if ( pSortLst )
    {
        pSortLst->First();
        do
        {
            if ( *( pSortLst->GetCurObject() ) &
                    ( FSYS_SORT_KIND | FSYS_SORT_SIZE |
                    FSYS_SORT_CREATED | FSYS_SORT_MODIFYED | FSYS_SORT_ACCESSED ) )
                pStatLst = new FileStatList();
        } while ( !pStatLst && pSortLst->Next() );
    }

#ifndef BOOTSTRAP
    // ggf. einen neuen Reader aufsetzen
    if ( !pReader )
        pReader = new DirReader_Impl( *this );
#endif

    // gibt es das zu oeffnende Verzeichnis ueberhaupt?
#ifndef UNX //explanation: see DirReader_Impl::Read() in unx.cxx
    if( !pReader->pDosDir )
    {
        nError = FSYS_ERR_NOTADIRECTORY;
        DELETEZ( pReader );
        return;
    }
#endif
}

/*************************************************************************
|*
|*    Dir::Scan()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 18.09.96
|*    Letzte Aenderung  MI 19.09.96
|*
*************************************************************************/

USHORT Dir::Scan( USHORT nCount )
{

    USHORT nRead = 0; // Anzahl in dieser Runde gelesener Eintr"age
    FSysFailOnErrorImpl();

    // noch nicht fertig gewesen
    if ( pReader )
    {
        // frischer Reader?
        if ( !pLst->Count() )
        {
            // dann ggf. Laufwerke scannen
            pReader->bInUse = TRUE;
            nRead = pReader->Init();
        }

        // weiterlesen...
        while ( nRead <= nCount && !pReader->bReady )
            nRead += pReader->Read();

        // fertig?
        if ( pReader && pReader->bReady )
            DELETEZ( pReader );
    }

    // Anzahl der gelesenen zur"uckgeben
    return nRead;
}

/*************************************************************************
|*
|*    Dir::Dir()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 16.05.91
|*    Letzte Aenderung  MI 04.03.92
|*
*************************************************************************/

Dir::Dir( const DirEntry& rDirEntry, DirEntryKind nKindFlags, FSysSort nSort, ... ):
    DirEntry( rDirEntry ),
    pReader( 0 )
{
    DBG_CTOR( Dir, NULL );

    Construct( nKindFlags );

    va_list pArgs;
    va_start( pArgs, nSort );
    ImpSetSort( pArgs, nSort );

    Reset();
}

/*************************************************************************
|*
|*    Dir::Dir()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 02.06.93
|*    Letzte Aenderung  MI 02.06.93
|*
*************************************************************************/

Dir::Dir( const DirEntry& rDirEntry, DirEntryKind nKindFlags ):
    DirEntry( rDirEntry ),
    pReader( 0 )
{
    DBG_CTOR( Dir, NULL );

    Construct( nKindFlags );
    Reset();
}

/*************************************************************************
|*
|*    Dir::Dir()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 16.05.91
|*    Letzte Aenderung  MA 04.11.91
|*
*************************************************************************/

Dir::Dir():
    pReader( 0 )
{
    DBG_CTOR( Dir, NULL );

    pLst     = NULL;
    pSortLst = NULL;
    pStatLst = NULL;
    eAttrMask = FSYS_KIND_ALL;
    aNameMask = String("*", osl_getThreadTextEncoding());
}

/*************************************************************************
|*
|*    Dir::~Dir()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 16.05.91
|*    Letzte Aenderung  MA 04.11.91
|*
*************************************************************************/

Dir::~Dir()
{
    DBG_DTOR( Dir, NULL );

    // alle DirEntries aus der Liste entfernen und deren Speicher freigeben
    if ( pLst )
    {
        DirEntry* pEntry = pLst->First();
        while (pEntry)
        {
            DirEntry* pNext = pLst->Next();
            delete pEntry;
            pEntry = pNext;
        }
        pLst->Clear();

        delete pLst;
    }

    // alle Sorts aus der Liste entfernen und deren Speicher freigeben
    if ( pSortLst )
    {
        FSysSort* pEntry = pSortLst->First();
        while (pEntry)
        {
            FSysSort*  pNext = pSortLst->Next();
            delete pEntry;
            pEntry = pNext;
        }
        pSortLst->Clear();

        delete pSortLst;
    }

    // alle FileStat's aus der Liste entfernen und deren Speicher freigeben
    if ( pStatLst )
    {
        FileStat* pEntry = pStatLst->First();
        while (pEntry)
        {
            FileStat*  pNext = pStatLst->Next();
            delete pEntry;
            pEntry = pNext;
        }
        pStatLst->Clear();
        delete pStatLst;
    }

    // ggf. laufenden Reader freigeben
    delete pReader;
}

/*************************************************************************
|*
|*    Dir::ImpSetSort()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MA 04.11.91
|*    Letzte Aenderung  MI 05.02.92
|*
*************************************************************************/

FSysError Dir::ImpSetSort( va_list pArgs, int nFirstSort )
{
    BOOL             bLast;
    FSysSort        *pSort;
    FSysSortList    *pNewSortLst = new FSysSortList;

    *( pSort = new FSysSort ) = nFirstSort;
    do
    {
        // letztes Kriterium?
        bLast = FSYS_SORT_END == (*pSort & FSYS_SORT_END);
        *pSort &= ~FSYS_SORT_END;

        // Ascending oder Descending?
        BOOL bAscending = FSYS_SORT_ASCENDING ==
                    ( *pSort & ( FSYS_SORT_ASCENDING | FSYS_SORT_DESCENDING ) );
        USHORT nSort = *pSort & ~(USHORT)FSYS_SORT_ASCENDING
                              &  ~(USHORT)FSYS_SORT_DESCENDING;

        // g"utliges Sortierkriterium?
        if ( ( nSort ==  FSYS_SORT_NAME ) ||
             ( nSort ==  FSYS_SORT_SIZE ) ||
             ( nSort ==  FSYS_SORT_EXT )  ||
             ( nSort ==  FSYS_SORT_CREATED ) ||
             ( nSort ==  FSYS_SORT_MODIFYED ) ||
             ( nSort ==  FSYS_SORT_ACCESSED ) ||
             ( nSort ==  FSYS_SORT_KIND ) )
        {
            pNewSortLst->Insert( pSort, APPEND );
            *(pSort = new FSysSort) = va_arg( pArgs, FSysSort );
        }
        else
        {   // ungueltiger Sort oder FSYS_SORT_NONE
            FSysSort* pEntry = pNewSortLst->First();
            while (pEntry)
            {
                FSysSort* pNext = pNewSortLst->Next();
                delete pEntry;
                pEntry = pNext;
            }
            pNewSortLst->Clear();
            delete pNewSortLst;
            if ( *pSort ==  FSYS_SORT_NONE )
            {
                delete pSort;
                if ( pSortLst )
                    delete pSortLst;
                return FSYS_ERR_OK;
            }
            else
            {
                delete pSort;
                return FSYS_ERR_NOTSUPPORTED;
            }
        }
    } while ( !bLast );

    va_end( pArgs );
    delete pSort;           // JP:6.3.00 - delete the initial pointer

    //Enfernen der alten Sort-Elemente
    if ( pSortLst )
    {
        FSysSort* pEntry = pSortLst->First();
        while (pEntry)
        {
            FSysSort* pNext = pSortLst->Next();
            delete pEntry;
            pEntry = pNext;
        }
        pSortLst->Clear();
        delete pSortLst;
    }
    pSortLst = pNewSortLst;

    //Jetzt noch neu Sortieren...

    //Wenn keine FileStats da sind, aber nun welche gebraucht werden,
    //ist der Aufruf von Update() die einfachste Moeglichkeit
    if ( !pStatLst && pSortLst )
    {
        pSortLst->First();
        do
        {
            if ( *(pSortLst->GetCurObject()) &
                  ( FSYS_SORT_CREATED | FSYS_SORT_MODIFYED | FSYS_SORT_SIZE |
                    FSYS_SORT_ACCESSED | FSYS_SORT_KIND ) )
            {
                Update();
                return FSYS_ERR_OK;
            }
        } while ( !pStatLst && pSortLst->Next() );
    }

    if ( pLst ) { //Keine DirEntry's, kein Sort.
        DirEntryList    *pOldLst = pLst; //alte Liste merken
        pLst = new DirEntryList();       //neue Liste (zu Sortieren)

        FileStatList *pOldStatLst = NULL; //alte StatListe merken
        if ( pStatLst ) {
            pOldStatLst = pStatLst;
            pStatLst = new FileStatList(); //neue StatListe (zu Sortieren)
        }
        pOldLst->First();
        do
        {
            //Sortiertes Einfuegen der Elemente aus den gemerkten Listen
            //in die 'richtigen' Listen
            if ( pOldStatLst )
                ImpSortedInsert( pOldLst->GetCurObject(),
                                 pOldStatLst->GetObject( pOldLst->GetCurPos() ) );
            else
                ImpSortedInsert( pOldLst->GetCurObject(), NULL );
        } while( pOldLst->Next() );

        delete pOldLst;
        if ( pOldStatLst )
            delete pOldStatLst;
    }
    return FSYS_ERR_OK;
}

/*************************************************************************
|*
|*    Dir::SetSort()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MA 04.11.91
|*    Letzte Aenderung  MI 05.02.92
|*
*************************************************************************/

FSysError Dir::SetSort( FSysSort nSort, ... )
{
    va_list pArgs;
    va_start( pArgs, nSort );
    return ImpSetSort( pArgs, nSort );
}

/*************************************************************************
|*
|*    Dir::operator[]()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 16.05.91
|*    Letzte Aenderung  MI 16.05.91
|*
*************************************************************************/

DirEntry& Dir::operator[] ( USHORT nIndex ) const
{
    DBG_ASSERT( nIndex < Count(), "Dir::operator[] : nIndex > Count()" );

    DirEntry *pEntry = pLst->GetObject( nIndex );
    return *pEntry;
}

/*************************************************************************
|*
|*    Dir::operator+= ()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 16.05.91
|*    Letzte Aenderung  MI 16.05.91
|*
*************************************************************************/

Dir& Dir::operator+=( const Dir& rDir )
{
    // ggf. erst den Rest lesen
    if ( pReader )
        Scan( USHRT_MAX );
    DBG_ASSERT( !rDir.pReader, "Dir::+= with incomplete Dir" );

    // ggf. initiale Liste erzeugen
    if ( !pLst )
        pLst = new DirEntryList();

    //Verlangen die Sortierkriterien FileStat's?
    BOOL bStat = FALSE;
    if ( pSortLst ) {
        pSortLst->First();
        do {
            if ( *(pSortLst->GetCurObject()) &
                  ( FSYS_SORT_CREATED | FSYS_SORT_MODIFYED | FSYS_SORT_SIZE |
                    FSYS_SORT_ACCESSED | FSYS_SORT_KIND ) )
                bStat = TRUE;
        } while ( !bStat && pSortLst->Next() );
    }
    FileStat *pStat = NULL;
    for ( USHORT nNr = 0; nNr < rDir.Count(); nNr++ ) {
        if ( bStat )
            if ( rDir.pStatLst )
            {
                pStat = new FileStat( *rDir.pStatLst->GetObject(nNr) );
            }
            else
                pStat = new FileStat( rDir[nNr] );
        ImpSortedInsert( new DirEntry( rDir[nNr] ), pStat );
    }
    return *this;
}

/*************************************************************************
|*
|*    Dir::Count()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 16.05.91
|*    Letzte Aenderung  MI 18.09.96
|*
*************************************************************************/


USHORT Dir::Count( BOOL bUpdated ) const
{
    // ggf. erst den Rest lesen
    if ( bUpdated && pReader )
        ((Dir*)this)->Scan( USHRT_MAX );

    return pLst == NULL ? 0 : (USHORT) pLst->Count();
}

/*************************************************************************
 *
 *  $RCSfile: unqidx.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:08 $
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

#define protected public

#ifndef _IMPCONT_HXX
#include <impcont.hxx>
#endif

#ifndef _UNQIDX_HXX
#include <unqidx.hxx>
#endif

#ifndef _UNQID_HXX
#include <unqid.hxx>
#endif

/*************************************************************************
|*
|*    UniqueIndex::UniqueIndex()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

UniqueIndex::UniqueIndex( ULONG _nStartIndex,
                          ULONG _nInitSize, ULONG _nReSize ) :
                 Container( _nInitSize )
{
    nReSize         = _nReSize;
    nStartIndex     = _nStartIndex;
    nUniqIndex      = 0;
    nCount          = 0;
}

/*************************************************************************
|*
|*    UniqueIndex::UniqueIndex()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

UniqueIndex::UniqueIndex( const UniqueIndex& rIdx ) :
                 Container( rIdx )
{
    nReSize     = rIdx.nReSize;
    nStartIndex = rIdx.nStartIndex;
    nUniqIndex  = rIdx.nUniqIndex;
    nCount      = rIdx.nCount;
}

/*************************************************************************
|*
|*    UniqueIndex::Insert()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

ULONG UniqueIndex::Insert( void* p )
{
    // NULL-Pointer ist nicht erlaubt
    if ( !p )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    // Ist Array voll, dann expandieren
    if ( nCount == Container::GetSize() )
        SetSize( nCount + nReSize );

    // Damit UniqIndex nicht ueberlaeuft, wenn Items geloescht wurden
    nUniqIndex = nUniqIndex % Container::GetSize();

    // Leeren Eintrag suchen
    while ( Container::ImpGetObject( nUniqIndex ) != NULL )
        nUniqIndex = (nUniqIndex+1) % Container::GetSize();

    // Object im Array speichern
    Container::Replace( p, nUniqIndex );

    // Anzahl der Eintraege erhoehen und Index zurueckgeben
    nCount++;
    nUniqIndex++;
    return ( nUniqIndex + nStartIndex - 1 );
}

/*************************************************************************
|*
|*    UniqueIndex::Insert()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    MM 21.04.96
|*    Letzte Aenderung  MM 21.04.96
|*
*************************************************************************/

ULONG UniqueIndex::Insert( ULONG nIndex, void* p )
{
    // NULL-Pointer ist nicht erlaubt
    if ( !p )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    ULONG nContIndex = nIndex - nStartIndex;
    // Ist Array voll, dann expandieren
    if ( nContIndex >= Container::GetSize() )
        SetSize( nContIndex + nReSize );

    // Object im Array speichern
    Container::Replace( p, nContIndex );

    // Anzahl der Eintraege erhoehen und Index zurueckgeben
    nCount++;
    return nIndex;
}

/*************************************************************************
|*
|*    UniqueIndex::Remove()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::Remove( ULONG nIndex )
{
    // Ist Index zulaessig
    if ( (nIndex >= nStartIndex) &&
         (nIndex < (Container::GetSize()+nStartIndex)) )
    {
        // Index-Eintrag als leeren Eintrag setzen und Anzahl der
        // gespeicherten Indexe erniedriegen, wenn Eintrag belegt war
        void* p = Container::Replace( NULL, nIndex-nStartIndex );
        if ( p )
            nCount--;
        return p;
    }
    else
        return NULL;
}

/*************************************************************************
|*
|*    UniqueIndex::Replace()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::Replace( ULONG nIndex, void* p )
{
    // NULL-Pointer ist nicht erlaubt
    if ( !p )
        return NULL;

    // Ist Index zulaessig
    if ( IsIndexValid( nIndex ) )
    {
        // Index-Eintrag ersetzen und alten zurueckgeben
        return Container::Replace( p, nIndex-nStartIndex );
    }
    else
        return NULL;
}

/*************************************************************************
|*
|*    UniqueIndex::Get()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::Get( ULONG nIndex ) const
{
    // Ist Index zulaessig
    if ( (nIndex >= nStartIndex) &&
         (nIndex < (Container::GetSize()+nStartIndex)) )
        return Container::ImpGetObject( nIndex-nStartIndex );
    else
        return NULL;
}

/*************************************************************************
|*
|*    UniqueIndex::GetCurIndex()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

ULONG UniqueIndex::GetCurIndex() const
{
    ULONG nPos = Container::GetCurPos();

    // Ist der Current-Index nicht belegt, dann gibt es keinen Current-Index
    if ( !Container::ImpGetObject( nPos ) )
        return UNIQUEINDEX_ENTRY_NOTFOUND;
    else
        return nPos+nStartIndex;
}

/*************************************************************************
|*
|*    UniqueIndex::GetIndex()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

ULONG UniqueIndex::GetIndex( const void* p ) const
{
    // Wird ein NULL-Pointer uebergeben, dann wurde Pointer nicht gefunden
    if ( !p )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    ULONG nIndex = Container::GetPos( p );

    if ( nIndex != CONTAINER_ENTRY_NOTFOUND )
        return nIndex+nStartIndex;
    else
        return UNIQUEINDEX_ENTRY_NOTFOUND;
}

/*************************************************************************
|*
|*    UniqueIndex::IsIndexValid()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

BOOL UniqueIndex::IsIndexValid( ULONG nIndex ) const
{
    // Ist Index zulaessig
    if ( (nIndex >= nStartIndex) &&
         (nIndex < (Container::GetSize()+nStartIndex)) )
    {
        // Index ist nur zulaessig, wenn Eintrag auch belegt ist
        if ( Container::ImpGetObject( nIndex-nStartIndex ) )
            return TRUE;
        else
            return FALSE;
    }
    else
        return FALSE;
}

/*************************************************************************
|*
|*    UniqueIndex::Seek()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::Seek( ULONG nIndex )
{
    // Index-Eintrag als aktuellen setzten, wenn er gueltig ist
    if ( IsIndexValid( nIndex ) )
        return Container::Seek( nIndex-nStartIndex );
    else
        return NULL;
}

/*************************************************************************
|*
|*    UniqueIndex::Seek()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::Seek( void* p )
{
    // Wird ein NULL-Pointer uebergeben, dann wurde Pointer nicht gefunden
    if ( !p )
        return NULL;

    ULONG nIndex = GetIndex( p );

    // Ist Index vorhanden, dann als aktuellen Eintrag setzen
    if ( nIndex != UNIQUEINDEX_ENTRY_NOTFOUND )
        return Container::Seek( nIndex-nStartIndex );
    else
        return NULL;
}

/*************************************************************************
|*
|*    UniqueIndex::First()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::First()
{
    void* p = Container::First();

    while ( !p && (Container::GetCurPos() < (Container::GetSize()-1)) )
        p = Container::Next();

    return p;
}

/*************************************************************************
|*
|*    UniqueIndex::Last()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::Last()
{
    void* p = Container::Last();

    while ( !p && Container::GetCurPos() )
        p = Container::Prev();

    return p;
}

/*************************************************************************
|*
|*    UniqueIndex::Next()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::Next()
{
    void* p = NULL;

    while ( !p && (Container::GetCurPos() < (Container::GetSize()-1)) )
        p = Container::Next();

    return p;
}

/*************************************************************************
|*
|*    UniqueIndex::Prev()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

void* UniqueIndex::Prev()
{
    void* p = NULL;

    while ( !p && Container::GetCurPos() )
        p = Container::Prev();

    return p;
}

/*************************************************************************
|*
|*    UniqueIndex::operator =()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

UniqueIndex& UniqueIndex::operator =( const UniqueIndex& rIdx )
{
    // Neue Werte zuweisen
    Container::operator =( rIdx );
    nReSize     = rIdx.nReSize;
    nStartIndex = rIdx.nStartIndex;
    nUniqIndex  = rIdx.nUniqIndex;
    nCount      = rIdx.nCount;
    return *this;
}

/*************************************************************************
|*
|*    UniqueIndex::operator ==()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

BOOL UniqueIndex::operator ==( const UniqueIndex& rIdx ) const
{
    // Neue Werte zuweisen
    if ( (nStartIndex == rIdx.nStartIndex) &&
         (nCount      == rIdx.nCount)      &&
         (Container::operator ==( rIdx )) )
        return TRUE;
    else
        return FALSE;
}
/*************************************************************************
|*
|*    UniqueIdContainer::UniqueIdContainer ()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    MM 29.04.96
|*    Letzte Aenderung  MM 29.04.96
|*
*************************************************************************/

UniqueIdContainer::UniqueIdContainer( const UniqueIdContainer& rObj )
    : UniqueIndex( rObj )
    , nCollectCount( rObj.nCollectCount )
{
    ULONG nCur = GetCurIndex();

    ImpUniqueId * pEle = (ImpUniqueId *)First();
    while( pEle )
    {
        pEle->nRefCount++;
        pEle = (ImpUniqueId *)Next();
    }
    Seek( nCur );
}

/*************************************************************************
|*
|*    UniqueIdContainer::operator = ()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    MM 01.08.94
|*    Letzte Aenderung  MM 01.08.94
|*
*************************************************************************/

UniqueIdContainer& UniqueIdContainer::operator = ( const UniqueIdContainer & rObj )
{
    UniqueIndex::operator = ( rObj );
    nCollectCount = rObj.nCollectCount;

    ULONG nCur = GetCurIndex();

    ImpUniqueId * pEle = (ImpUniqueId *)First();
    while( pEle )
    {
        pEle->nRefCount++;
        pEle = (ImpUniqueId *)Next();
    }
    Seek( nCur );
    return *this;
}

/*************************************************************************
|*
|*    UniqueIdContainer::Clear()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    MM 01.08.94
|*    Letzte Aenderung  MM 01.08.94
|*
*************************************************************************/

void UniqueIdContainer::Clear( BOOL bAll )
{
    USHORT nFree = bAll ? 0xFFFF : 1;

    ImpUniqueId* pId = (ImpUniqueId*)Last();
    BOOL bLast = TRUE;
    while ( pId )
    {
        if ( pId->nRefCount <= nFree )
        {
            ((ImpUniqueId *)Remove( pId->nId ))->Release();
            if( bLast )
                pId = (ImpUniqueId *)Last();
            else
                pId = (ImpUniqueId *)Prev();
        }
        else
        {
            pId = (ImpUniqueId *)Prev();
            bLast = FALSE;
        }
    }
}

/*************************************************************************
|*
|*    UniqueIdContainer::CreateId()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    MM 01.08.94
|*    Letzte Aenderung  MM 01.08.94
|*
*************************************************************************/

UniqueItemId UniqueIdContainer::CreateId()
{
    if( nCollectCount > 50 )
    { // aufraeumen
        Clear( FALSE );
        nCollectCount = 0;
    }
    nCollectCount++;

    ImpUniqueId * pId = new ImpUniqueId;
    pId->nRefCount = 1;
    pId->nId = Insert( pId );
    return UniqueItemId( pId );
}

/*************************************************************************
|*
|*    UniqueIdContainer::CreateIdProt()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    MM 01.08.94
|*    Letzte Aenderung  MM 01.08.94
|*
*************************************************************************/

UniqueItemId UniqueIdContainer::CreateFreeId( ULONG nId )
{
    // Einfach erzeugen, fuer abgeleitete Klasse
    ImpUniqueId * pId = new ImpUniqueId;
    pId->nRefCount = 0;
    pId->nId = nId;
    return UniqueItemId( pId );
}

/*************************************************************************
|*
|*    UniqueIdContainer::CreateIdProt()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    MM 01.08.94
|*    Letzte Aenderung  MM 01.08.94
|*
*************************************************************************/

UniqueItemId UniqueIdContainer::CreateIdProt( ULONG nId )
{
    if ( IsIndexValid( nId ) )
        return UniqueItemId( (ImpUniqueId *)Get( nId ) );

    ImpUniqueId * pId;
    do
    {
        pId = new ImpUniqueId;
        pId->nRefCount = 1;
        pId->nId = Insert( pId );
    }
    while( pId->nId != nId );
    return UniqueItemId( pId );
}

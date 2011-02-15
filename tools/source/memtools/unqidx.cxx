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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <impcont.hxx>
#include <tools/unqidx.hxx>
#include <tools/unqid.hxx>

/*************************************************************************
|*
|*    UniqueIndex::UniqueIndex()
|*
|*    Beschreibung      UNQIDX.SDW
|*    Ersterstellung    TH 24.09.91
|*    Letzte Aenderung  TH 24.09.91
|*
*************************************************************************/

UniqueIndex::UniqueIndex( sal_uIntPtr _nStartIndex,
                          sal_uIntPtr _nInitSize, sal_uIntPtr _nReSize ) :
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

sal_uIntPtr UniqueIndex::Insert( void* p )
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

sal_uIntPtr UniqueIndex::Insert( sal_uIntPtr nIndex, void* p )
{
    // NULL-Pointer ist nicht erlaubt
    if ( !p )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    sal_uIntPtr nContIndex = nIndex - nStartIndex;
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

void* UniqueIndex::Remove( sal_uIntPtr nIndex )
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

void* UniqueIndex::Replace( sal_uIntPtr nIndex, void* p )
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

void* UniqueIndex::Get( sal_uIntPtr nIndex ) const
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

sal_uIntPtr UniqueIndex::GetCurIndex() const
{
    sal_uIntPtr nPos = Container::GetCurPos();

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

sal_uIntPtr UniqueIndex::GetIndex( const void* p ) const
{
    // Wird ein NULL-Pointer uebergeben, dann wurde Pointer nicht gefunden
    if ( !p )
        return UNIQUEINDEX_ENTRY_NOTFOUND;

    sal_uIntPtr nIndex = Container::GetPos( p );

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

sal_Bool UniqueIndex::IsIndexValid( sal_uIntPtr nIndex ) const
{
    // Ist Index zulaessig
    if ( (nIndex >= nStartIndex) &&
         (nIndex < (Container::GetSize()+nStartIndex)) )
    {
        // Index ist nur zulaessig, wenn Eintrag auch belegt ist
        if ( Container::ImpGetObject( nIndex-nStartIndex ) )
            return sal_True;
        else
            return sal_False;
    }
    else
        return sal_False;
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

void* UniqueIndex::Seek( sal_uIntPtr nIndex )
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

    sal_uIntPtr nIndex = GetIndex( p );

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

sal_Bool UniqueIndex::operator ==( const UniqueIndex& rIdx ) const
{
    // Neue Werte zuweisen
    if ( (nStartIndex == rIdx.nStartIndex) &&
         (nCount      == rIdx.nCount)      &&
         (Container::operator ==( rIdx )) )
        return sal_True;
    else
        return sal_False;
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
    sal_uIntPtr nCur = GetCurIndex();

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

    sal_uIntPtr nCur = GetCurIndex();

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

void UniqueIdContainer::Clear( sal_Bool bAll )
{
    sal_uInt16 nFree = bAll ? 0xFFFF : 1;

    ImpUniqueId* pId = (ImpUniqueId*)Last();
    sal_Bool bLast = sal_True;
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
            bLast = sal_False;
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
        Clear( sal_False );
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

UniqueItemId UniqueIdContainer::CreateFreeId( sal_uIntPtr nId )
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

UniqueItemId UniqueIdContainer::CreateIdProt( sal_uIntPtr nId )
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

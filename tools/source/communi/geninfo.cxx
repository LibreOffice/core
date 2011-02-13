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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include "tools/geninfo.hxx"
#include <stdio.h>

//
// class GenericInformation
//

/*****************************************************************************/
GenericInformation::GenericInformation( const ByteString &rKey,
                        const ByteString &rValue,
                        GenericInformationList *pParentList,
                        GenericInformationList *pSubInfos )
/*****************************************************************************/
                : ByteString( rKey ),
                sValue( rValue ),
                pInfoList( pSubInfos ),
                pParent( pParentList )
{
    // if a ParentList exists, insert this object into it
    if ( pParent )
        pParent->InsertInfo( this );
    // make myself owner of pInfoList
    if ( pInfoList )
        pInfoList->SetOwner( this );
}

/*****************************************************************************/
GenericInformation::GenericInformation( const GenericInformation& rInf,
                                        BOOL bCopySubs)
/*****************************************************************************/
                : ByteString( rInf ),
                sValue( rInf.sValue ),
                pInfoList( 0L ),
                pParent(NULL)
{
    if(bCopySubs && rInf.pInfoList)
        pInfoList = new GenericInformationList(*rInf.pInfoList, this);
}

/*****************************************************************************/
GenericInformation::~GenericInformation()
/*****************************************************************************/
{
    // remove pInfoList and all childs out of memory
    delete pInfoList;
    pInfoList = 0;

    // remove this Info out of ParentList
    if ( pParent )
        pParent->RemoveInfo( this );
}

/*****************************************************************************/
BOOL GenericInformation::InsertSubInfo( GenericInformation *pInfo )
/*****************************************************************************/
{
    return ( pInfoList && pInfoList->InsertInfo( pInfo ));
}

/*****************************************************************************/
BOOL GenericInformation::InsertSubInfo( const ByteString &rPathKey, const ByteString &rValue,
                    BOOL bSearchByPath, BOOL bNewPath )
/*****************************************************************************/
{
  return (pInfoList && pInfoList->InsertInfo( rPathKey, rValue, bSearchByPath, bNewPath ));
}

/*****************************************************************************/
void GenericInformation::RemoveSubInfo( GenericInformation *pInfo,
                            BOOL bDelete )
/*****************************************************************************/
{
    pInfoList->RemoveInfo( pInfo, bDelete );
}

/*****************************************************************************/
GenericInformation *GenericInformation::GetSubInfo( ByteString &rKey,
                        BOOL bSearchByPath,
                            BOOL bCreatePath )
/*****************************************************************************/
{
  if ( !pInfoList && bCreatePath )
    pInfoList = new GenericInformationList( this );
  if ( pInfoList )
    return pInfoList->GetInfo( rKey, bSearchByPath, bCreatePath );
  return NULL;
}


//
// class GenericInformationList
//

/*****************************************************************************/
GenericInformationList::GenericInformationList( GenericInformation *pParent )
/*****************************************************************************/
                : pOwner( pParent )
{
}

/*****************************************************************************/
GenericInformationList::GenericInformationList(
    const GenericInformationList& rList,
    GenericInformation *pParent
)
/*****************************************************************************/
{
    GenericInformation* pTemp,*pWork;

    pOwner = pParent;

    for( size_t i = 0; i < rList.size(); i++ )
    {
        pTemp = rList[ i ];
        pWork = new GenericInformation( *pTemp, TRUE );

        maList.push_back( pWork );
    }
}

/*****************************************************************************/
GenericInformationList::~GenericInformationList()
/*****************************************************************************/
{
    // delete all Informations stored in this List
    for ( size_t i = 0, n = maList.size(); i < n; ++i ) {
        maList[ i ]->ListDeleted();
        delete maList[ i ];
    }
    maList.clear();
}

/*****************************************************************************/
GenericInformation *GenericInformationList::Search(
    size_t &rPos,
    ByteString sKey,
    size_t nStart,
    size_t nEnd
)
/*****************************************************************************/
{
    if ( maList.empty() ) {
        rPos = 0;
        return NULL;
    }

    if ( nStart == nEnd ) {
        rPos = nStart;
        ByteString sCandidate = ByteString( *maList[ nStart ] );
        if ( sCandidate.ToUpperAscii() == sKey.ToUpperAscii()) {
            return maList[ nStart ]; // found !!!
        }
        else {
            // requested key not found
            return NULL;
        }
    }

    // search binary in existing list
    size_t nActPos = nStart + (( nEnd - nStart ) / 2 );
    rPos = nActPos;
    ByteString sCandidate = ByteString( *maList[ nActPos ] );

    if ( sCandidate.ToUpperAscii()  == sKey.ToUpperAscii() )
        return maList[ nActPos ]; // found !!!

    // split the list at ActPos
    if ( sCandidate < sKey )
        return Search( rPos, sKey, nActPos + 1, nEnd );
    else
        return Search( rPos, sKey, nStart, nActPos );
}

/*****************************************************************************/
GenericInformation *GenericInformationList::GetInfo( ByteString &rKey,
                             BOOL bSearchByPath,
                             BOOL bCreatePath )
/*****************************************************************************/
{

    rKey.EraseLeadingChars( '/' );
    rKey.EraseTrailingChars( '/' );

    ByteString sKey;
    if ( bSearchByPath )
        sKey = rKey.GetToken( 0, '/' );
    else
        sKey = rKey;

    size_t nPos = 0;
    GenericInformation *pReturnInfo = Search( nPos, sKey, 0, maList.size() - 1 );
    /* wenn kein Searchpath gesetzt und kein Returninfo vorhanden,
     *   gib NULL zurueck
     * wenn Searchpath gesetzt und returninfo vorhanden,
     *   suche weiter nach unten
     * wenn searchpath gesetzt kein returninfo vorhanden und newpath gesetzt,
     *   mache neues Verzeichniss
     */
    USHORT nTokenCount = rKey.GetTokenCount('/');
    // search for next key of path in next level of tree
    if ( bSearchByPath && (nTokenCount > 1)) {
      ByteString sPath = ByteString(rKey.Copy( sKey.Len() + 1 ));
      if ( !pReturnInfo ) { // wenn kein Return, dann muss man es anlegen
        if ( !bCreatePath ) // wenn aber kein Create, dann nicht anlegen
          return NULL;
        pReturnInfo = new GenericInformation( sKey, "", this, NULL);
        pReturnInfo->SetSubList( new GenericInformationList( pReturnInfo ));
      }
      return pReturnInfo->GetSubInfo( sPath, TRUE, bCreatePath );
    }
    if ( !pReturnInfo && bCreatePath ) {
      pReturnInfo = new GenericInformation ( sKey, "", this, NULL);
    }

    return pReturnInfo; // kann durchaus NULL sein.
}

/*****************************************************************************/
size_t GenericInformationList::InsertSorted(
    GenericInformation *pInfo,
    BOOL bOverwrite,
    size_t nStart,
    size_t nEnd
)
/*****************************************************************************/
{
    if ( maList.empty() ) {
        // empty list, so insert at first pos
        maList.push_back( pInfo );
        return 0;
    }

    ByteString sKey( pInfo->GetBuffer());
    sKey.ToUpperAscii();

    // Check to speed up reading a (partially) sorted list
    if ( nStart == 0 && maList.size()-1 == nEnd )
    {
        ByteString sCandidate( *maList[ nEnd ] );
        if ( sCandidate.ToUpperAscii() < sKey )
        {
            maList.push_back( pInfo );
            return nEnd+1;
        }
    }

    // Only one element, so insert before or after
    if ( maList.size() == 1 ) {
        ByteString sCandidate( *maList[ 0 ] );
        if ( sCandidate.ToUpperAscii() == sKey ) {
            // key allready exists in list
            if ( bOverwrite ) {
                if ( pInfo != maList[ 0 ] )
                    delete maList[ 0 ];
                maList[ 0 ] = pInfo;
            }
            return 0;
        }
        else if ( sCandidate > sKey ) {
            maList.insert( maList.begin(), pInfo );
            return 0;
        }
        else {
            maList.push_back( pInfo );
            return 1;
        }
    }

    size_t nActPos = nStart + (( nEnd - nStart ) / 2 );
    ByteString sCandidate = ByteString( *maList[ nActPos ] );

    if ( sCandidate.ToUpperAscii() == sKey ) {
        // key allready exists in list
        if ( bOverwrite ) {
            if ( pInfo != maList[ nActPos ] )
                delete maList[ nActPos ];
            maList[ nActPos ] = pInfo;
        }
        return nActPos;
    }

    if ( nStart == nEnd ) {
        // now more ways to search for key -> insert here
        GenericInformationList_Impl::iterator it = maList.begin();
        ::std::advance( it, nStart );
        if ( sCandidate > sKey ) {
            maList.insert( it, pInfo );
            return nStart;
        }
        else {
            ++it;
            maList.insert( it, pInfo );
            return ( nStart + 1 );
        }
    }

    if ( nActPos == maList.size() - 1 ) {
        // reached end of list -> insert here
        maList.push_back( pInfo );
        return ( nActPos + 1 );
    }

    ByteString sSecondCand = ByteString( *maList[ nActPos + 1 ] );
    if (( sCandidate < sKey ) && ( sSecondCand.ToUpperAscii() > sKey )) {
        // optimal position to insert object
        GenericInformationList_Impl::iterator it = maList.begin();
        ::std::advance( it, nActPos + 1 );
        maList.insert( it, pInfo );
        return ( nActPos + 1 );
    }

    if ( sCandidate < sKey )
        return InsertSorted( pInfo, bOverwrite, nActPos + 1, nEnd );
    else
        return InsertSorted( pInfo, bOverwrite, nStart, nActPos );
}

/*****************************************************************************/
BOOL GenericInformationList::InsertInfo( GenericInformation *pInfo,
                                BOOL bOverwrite )
/*****************************************************************************/
{
    if ( !pInfo->Len())
        return FALSE;

    InsertSorted( pInfo, bOverwrite, 0, maList.size() - 1 );
    return TRUE;
}


/*****************************************************************************/
BOOL GenericInformationList::InsertInfo( const ByteString &rPathKey, const ByteString &rValue,
                     BOOL bSearchByPath, BOOL bNewPath )
/*****************************************************************************/
{
  GenericInformation *pInfo;
  ByteString sPathKey ( rPathKey );
  sPathKey.EraseLeadingChars( '/' );
  sPathKey.EraseTrailingChars( '/' );

  pInfo = GetInfo( sPathKey, bSearchByPath, bNewPath );

  if ( pInfo ) {
    pInfo->SetValue( rValue );
    return TRUE;
  }
  return FALSE;
}

/*****************************************************************************/
void GenericInformationList::RemoveInfo( GenericInformation *pInfo,
                                BOOL bDelete )
/*****************************************************************************/
{
    for (
        GenericInformationList_Impl::iterator it = maList.begin();
        it < maList.end();
        ++it
    )   {
        if ( *it == pInfo ) {
            maList.erase( it );
        }
    }
    if ( bDelete )
        delete pInfo;
}

GenericInformation* GenericInformationList::SetOwner( GenericInformation *pNewOwner )
{
    GenericInformation *pOldOwner = pOwner;
    if ( pOwner )   // bei parent austragen;
        pOwner->SetSubList( NULL );
    if ( pNewOwner )
        pNewOwner->SetSubList( this );
    pOwner = pNewOwner;
    return pOldOwner;
}

size_t GenericInformationList::size() const {
    return maList.size();
}

GenericInformation* GenericInformationList::operator[]( size_t i ) const {
    return ( i < maList.size() ) ? maList[ i ] : NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

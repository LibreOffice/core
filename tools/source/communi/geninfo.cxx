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
                                        sal_Bool bCopySubs)
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
sal_Bool GenericInformation::InsertSubInfo( GenericInformation *pInfo )
/*****************************************************************************/
{
    return ( pInfoList && pInfoList->InsertInfo( pInfo ));
}

/*****************************************************************************/
sal_Bool GenericInformation::InsertSubInfo( const ByteString &rPathKey, const ByteString &rValue,
                    sal_Bool bSearchByPath, sal_Bool bNewPath )
/*****************************************************************************/
{
  return (pInfoList && pInfoList->InsertInfo( rPathKey, rValue, bSearchByPath, bNewPath ));
}

/*****************************************************************************/
void GenericInformation::RemoveSubInfo( GenericInformation *pInfo,
                            sal_Bool bDelete )
/*****************************************************************************/
{
    pInfoList->RemoveInfo( pInfo, bDelete );
}

/*****************************************************************************/
//void GenericInformation::RemoveSelf( sal_Bool bDelete )
/*****************************************************************************/
/*{
  if ( pParent )
    pParent->RemoveInfo( this, bDelete ); // loescht sich aus der Liste vom Parent und
  // bei Bedarf auch mit obiger Methode alle Sublisten

  // loescht sich bei Bedarf auch selbst
    if ( bDelete )
    delete this;
}
*/

/*****************************************************************************/
GenericInformation *GenericInformation::GetSubInfo( ByteString &rKey,
                        sal_Bool bSearchByPath,
                            sal_Bool bCreatePath )
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
GenericInformationList::GenericInformationList(const GenericInformationList& rList,
                            GenericInformation *pParent)
/*****************************************************************************/
    : GenericInformationList_Impl()
{
    sal_uInt16 i;
    GenericInformation* pTemp,*pWork;

    pOwner = pParent;

    for(i=0;i<rList.Count();i++)
    {
        pTemp = rList.GetObject(i);
        pWork = new GenericInformation(*pTemp,sal_True);

        Insert(pWork,LIST_APPEND);
    }
}

/*****************************************************************************/
GenericInformationList::~GenericInformationList()
/*****************************************************************************/
{
    // delete all Informations stored in this List
    // ### GH: Hier werden dann wohl etwa die H�lfte der Eintr�ge gel�scht
/*  for ( sal_uIntPtr i = 0; i < Count(); i++ ) {
        GetObject( i )->ListDeleted();
        delete GetObject( i );
        Remove( i );*/
    // Neue Variante:
    while ( Count() ) {
        GetObject( 0 )->ListDeleted();
        delete GetObject( 0 );
        Remove( (sal_uIntPtr)0 );
    }
}

/*****************************************************************************/
GenericInformation *GenericInformationList::Search( sal_uIntPtr &rPos, ByteString sKey,
                                                   sal_uIntPtr nStart, sal_uIntPtr nEnd )
/*****************************************************************************/
{
    if ( Count() == 0 ) {
        rPos = 0;
        return NULL;
    }

    if ( nStart == nEnd ) {
        rPos = nStart;
        ByteString sCandidate = ByteString( *GetObject( nStart ));
        if ( sCandidate.ToUpperAscii() == sKey.ToUpperAscii()) {
            return GetObject( nStart ); // found !!!
        }
        else {
            // requested key not found
            return NULL;
        }
    }

    // search binary in existing list
    sal_uIntPtr nActPos = nStart + (( nEnd - nStart ) / 2 );
    rPos = nActPos;
    ByteString sCandidate = ByteString( *GetObject( nActPos ));

    if ( sCandidate.ToUpperAscii()  == sKey.ToUpperAscii())
        return GetObject( nActPos ); // found !!!

    // split the list at ActPos
    if ( sCandidate < sKey )
        return Search( rPos, sKey, nActPos + 1, nEnd );
    else
        return Search( rPos, sKey, nStart, nActPos );
}

/*****************************************************************************/
GenericInformation *GenericInformationList::GetInfo( ByteString &rKey,
                             sal_Bool bSearchByPath,
                             sal_Bool bCreatePath )
/*****************************************************************************/
{

    rKey.EraseLeadingChars( '/' );
    rKey.EraseTrailingChars( '/' );

    ByteString sKey;
    if ( bSearchByPath )
        sKey = rKey.GetToken( 0, '/' );
    else
        sKey = rKey;

    sal_uIntPtr nPos = 0;
    GenericInformation *pReturnInfo = Search( nPos, sKey, 0, Count() - 1 );
    /* wenn kein Searchpath gesetzt und kein Returninfo vorhanden,
     *   gib NULL zurueck
     * wenn Searchpath gesetzt und returninfo vorhanden,
     *   suche weiter nach unten
     * wenn searchpath gesetzt kein returninfo vorhanden und newpath gesetzt,
     *   mache neues Verzeichniss
     */
    sal_uInt16 nTokenCount = rKey.GetTokenCount('/');
    // search for next key of path in next level of tree
    if ( bSearchByPath && (nTokenCount > 1)) {
      ByteString sPath = ByteString(rKey.Copy( sKey.Len() + 1 ));
      if ( !pReturnInfo ) { // wenn kein Return, dann muss man es anlegen
        if ( !bCreatePath ) // wenn aber kein Create, dann nicht anlegen
          return NULL;
        pReturnInfo = new GenericInformation( sKey, "", this, NULL);
        pReturnInfo->SetSubList( new GenericInformationList( pReturnInfo ));
      }
      return pReturnInfo->GetSubInfo( sPath, sal_True, bCreatePath );
    }
    if ( !pReturnInfo && bCreatePath ) {
      pReturnInfo = new GenericInformation ( sKey, "", this, NULL);
    }

    return pReturnInfo; // kann durchaus NULL sein.
}

/*****************************************************************************/
sal_uIntPtr GenericInformationList::InsertSorted( GenericInformation *pInfo,
                                        sal_Bool bOverwrite,
                                        sal_uIntPtr nStart, sal_uIntPtr nEnd )
/*****************************************************************************/
{
    if ( Count() == 0 ) {
        // empty list, so insert at first pos
        Insert( pInfo, LIST_APPEND );
        return 0;
    }

    ByteString sKey( pInfo->GetBuffer());
    sKey.ToUpperAscii();

    // Check to sppeed up reading a (partially) sorted list
    if ( nStart == 0 && Count()-1 == nEnd )
    {
        ByteString sCandidate( *GetObject( nEnd ));
        if ( sCandidate.ToUpperAscii() < sKey )
        {
            Insert( pInfo, LIST_APPEND );
            return nEnd+1;
        }
    }

// ### GH: dieser Block schein �berfl�ssig zu sein
    if ( Count() == 1 ) {
        ByteString sCandidate( *GetObject( 0 ));
        if ( sCandidate.ToUpperAscii() == sKey ) {
            // key allready exists in list
            if ( bOverwrite )
                Replace( pInfo, sal_uIntPtr(0));    // ### Laut NF scheint hier ein Memory Leak zu sein
            return 0;
        }
        else if ( sCandidate > sKey ) {
            Insert( pInfo, sal_uIntPtr(0));
            return 0;
        }
        else {
            Insert( pInfo, LIST_APPEND );
            return 1;
        }
    }
// ### GH: /ENDE/ dieser Block schein �berfl�ssig zu sein

    sal_uIntPtr nActPos = nStart + (( nEnd - nStart ) / 2 );
    ByteString sCandidate = ByteString( *GetObject( nActPos ));

    if ( sCandidate.ToUpperAscii() == sKey ) {
        // key allready exists in list
        if ( bOverwrite )
            Replace( pInfo, nActPos );  // ### Laut NF scheint hier ein Memory Leak zu sein
        return nActPos;
    }

    if ( nStart == nEnd ) {
        // now more ways to search for key -> insert here
        if ( sCandidate > sKey ) {
            Insert( pInfo, nStart );
            return nStart;
        }
        else {
            Insert( pInfo, nStart + 1 );
            return ( nStart + 1 );
        }
    }

    if ( nActPos == Count() - 1 ) {
        // reached end of list -> insert here
        Insert( pInfo, LIST_APPEND );
        return ( nActPos + 1 );
    }

    ByteString sSecondCand = ByteString( *GetObject( nActPos + 1 ));
    if (( sCandidate < sKey ) && ( sSecondCand.ToUpperAscii() > sKey )) {
        // optimal position to insert object
        Insert( pInfo, nActPos + 1 );
        return ( nActPos + 1 );
    }

    if ( sCandidate < sKey )
        return InsertSorted( pInfo, bOverwrite, nActPos + 1, nEnd );
    else
        return InsertSorted( pInfo, bOverwrite, nStart, nActPos );
}

/*****************************************************************************/
sal_Bool GenericInformationList::InsertInfo( GenericInformation *pInfo,
                                sal_Bool bOverwrite )
/*****************************************************************************/
{
    if ( !pInfo->Len())
        return sal_False;

    InsertSorted( pInfo, bOverwrite, 0, Count() - 1 );
    return sal_True;
}


/*****************************************************************************/
sal_Bool GenericInformationList::InsertInfo( const ByteString &rPathKey, const ByteString &rValue,
                     sal_Bool bSearchByPath, sal_Bool bNewPath )
/*****************************************************************************/
{
  GenericInformation *pInfo;
  ByteString sPathKey ( rPathKey );
  sPathKey.EraseLeadingChars( '/' );
  sPathKey.EraseTrailingChars( '/' );

  pInfo = GetInfo( sPathKey, bSearchByPath, bNewPath );

  if ( pInfo ) {
    pInfo->SetValue( rValue );
    return sal_True;
  }
  return sal_False;
}

/*****************************************************************************/
void GenericInformationList::RemoveInfo( GenericInformation *pInfo,
                                sal_Bool bDelete )
/*****************************************************************************/
{
    Remove( pInfo );
    if ( bDelete )
        delete pInfo;
/*  if ( Count() == 0 && pOwner )   // Leere Listen entfernen;
    {
        SetOwner( NULL );
        delete this;
    } Rausgepatched by GH */
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



/*************************************************************************
 *
 *  $RCSfile: UnoNameItemTable.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-23 21:33:15 $
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

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#include "svdmodel.hxx"

#ifndef _SVX_UNONAMEITEMTABLE_HXX_
#include "UnoNameItemTable.hxx"
#endif

#include "unoapi.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

SvxUnoNameItemTable::SvxUnoNameItemTable( SdrModel* pModel, USHORT nWhich, BYTE nMemberId ) throw()
: mpModel( pModel ),
  mpModelPool( pModel ? &pModel->GetItemPool() : NULL ),
  mpStylePool( ( pModel && pModel->GetStyleSheetPool()) ? &pModel->GetStyleSheetPool()->GetPool() : NULL ),
  mnWhich( nWhich ), mnMemberId( nMemberId )
{
}

SvxUnoNameItemTable::~SvxUnoNameItemTable() throw()
{
    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    while( aIter != aEnd )
    {
        delete (*aIter).first;
        delete (*aIter++).second;
    }
}

sal_Bool SAL_CALL SvxUnoNameItemTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
}

// XNameContainer
void SAL_CALL SvxUnoNameItemTable::insertByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( hasByName( aApiName ) )
        throw container::ElementExistException();

    String aName;
    SvxUnogetInternalNameForItem( mnWhich, aApiName, aName );

    SfxItemSet* mpInSet1 = new SfxItemSet( *mpModelPool, mnWhich, mnWhich );
    SfxItemSet* mpInSet2 = mpStylePool ? new SfxItemSet( *mpStylePool, mnWhich, mnWhich ) : NULL;
    maItemSetVector.push_back( std::pair< SfxItemSet*, SfxItemSet*>( mpInSet1, mpInSet2 ) );

    NameOrIndex* pNewItem = createItem();
    pNewItem->SetName( String( aName ) );
    pNewItem->PutValue( aElement, mnMemberId );

    mpInSet1->Put( *pNewItem, mnWhich );
    if( mpInSet2 )
        mpInSet2->Put( *pNewItem, mnWhich );
    delete pNewItem;
}



void SAL_CALL SvxUnoNameItemTable::removeByName( const OUString& aApiName )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    String Name;
    SvxUnogetInternalNameForItem( mnWhich, aApiName, Name );

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;
    const String aSearchName( Name );

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter).first->Get( mnWhich ) );
        if( pItem->GetName() == aSearchName )
        {
            delete (*aIter).first;
            delete (*aIter).second;
            maItemSetVector.erase( aIter );
            return;
        }
        aIter++;
    }

    if( !hasByName( Name ) )
        throw container::NoSuchElementException();
}

// XNameReplace
void SAL_CALL SvxUnoNameItemTable::replaceByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    String aName;
    SvxUnogetInternalNameForItem( mnWhich, aApiName, aName );

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;
    const String aSearchName( aName );

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter).first->Get( mnWhich ) );
        if( pItem->GetName() == aSearchName )
        {
            NameOrIndex* pNewItem = createItem();
            pNewItem->SetName( aSearchName );
            if( !pNewItem->PutValue( aElement, mnMemberId ) )
                throw lang::IllegalArgumentException();

            (*aIter).first->Put( *pNewItem );
            if( (*aIter).second )
                (*aIter).second->Put( *pNewItem );
            return;
        }
        aIter++;
    }

    if( !hasByName( aName ) )
        throw container::NoSuchElementException();
}

// XNameAccess
uno::Any SAL_CALL SvxUnoNameItemTable::getByName( const OUString& aApiName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    String aName;
    SvxUnogetInternalNameForItem( mnWhich, aApiName, aName );

    uno::Any aAny;

    if( mpModelPool && aName.Len() != 0 )
    {
        const String aSearchName( aName );
        NameOrIndex *pItem;
        sal_Int32 nSurrogate;

        sal_Int32 nSurrogateCount = mpModelPool ? (sal_Int32)mpModelPool->GetItemCount( mnWhich ) : 0;
        for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
        {
            pItem = (NameOrIndex*)mpModelPool->GetItem( mnWhich, (USHORT)nSurrogate );

            if( pItem && pItem->GetName() == aSearchName )
            {
                pItem->QueryValue( aAny, mnMemberId );
                return aAny;
            }
        }

        nSurrogateCount = mpStylePool ? (sal_Int32)mpStylePool->GetItemCount( mnWhich ) : 0;
        for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
        {
            pItem = (NameOrIndex*)mpStylePool->GetItem( mnWhich, (USHORT)nSurrogate );

            if( pItem && pItem->GetName() == aSearchName )
            {
                pItem->QueryValue( aAny, mnMemberId );
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
    return aAny;
}

uno::Sequence< OUString > SAL_CALL SvxUnoNameItemTable::getElementNames(  )
    throw( uno::RuntimeException )
{
    const sal_Int32 nSurrogateCount1 = mpModelPool ? (sal_Int32)mpModelPool->GetItemCount( mnWhich ) : 0;
    const sal_Int32 nSurrogateCount2 = mpStylePool ? (sal_Int32)mpStylePool->GetItemCount( mnWhich ) : 0;

    sal_Int32 nCount = 0;

    uno::Sequence< OUString > aSeq( nSurrogateCount1 + nSurrogateCount2 );
    OUString* pStrings = aSeq.getArray();
    sal_Int32 nSurrogate;

    NameOrIndex *pItem;

    for( nSurrogate = 0; nSurrogate < nSurrogateCount1; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( mnWhich, (USHORT)nSurrogate );

        if( pItem == NULL || pItem->GetName().Len() == 0 )
            continue;

        // check if there is already an item with this name
        const OUString aSearchName( pItem->GetName() );
        OUString* pStartNames = aSeq.getArray();
        sal_Bool bFound = sal_False;
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            if( *pStartNames++ == aSearchName )
            {
                bFound = sal_True;
                break;
            }
        }

        if( !bFound )
        {
            nCount++;

            SvxUnogetApiNameForItem( mnWhich, pItem->GetName(), *pStrings );
            pStrings++;
        }
    }

    for( nSurrogate = 0; nSurrogate < nSurrogateCount2; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpStylePool->GetItem( mnWhich, (USHORT)nSurrogate );

        if( pItem == NULL || pItem->GetName().Len() == 0 )
            continue;

        // check if there is already an item with this name
        const OUString aSearchName( pItem->GetName() );
        OUString* pStartNames = aSeq.getArray();
        sal_Bool bFound = sal_False;
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            if( *pStartNames++ == aSearchName )
            {
                bFound = sal_True;
                break;
            }
        }

        if( !bFound )
        {
            nCount++;

            SvxUnogetApiNameForItem( mnWhich, pItem->GetName(), *pStrings );
            pStrings++;
        }
    }

    if( nCount < aSeq.getLength() )
        aSeq.realloc( nCount );

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasByName( const OUString& aApiName )
    throw( uno::RuntimeException )
{
    String aName;
    SvxUnogetInternalNameForItem( mnWhich, aApiName, aName );

    if( aName.Len() == 0 )
        return sal_False;

    const String aSearchName( aName );
    USHORT nSurrogate;

    const NameOrIndex *pItem;

    USHORT nCount = mpModelPool ? mpModelPool->GetItemCount( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( mnWhich, nSurrogate );
        if( pItem && pItem->GetName() == aSearchName )
            return sal_True;
    }

    nCount = mpStylePool ? mpStylePool->GetItemCount( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpStylePool->GetItem( mnWhich, nSurrogate );
        if( pItem && pItem->GetName() == aSearchName )
            return sal_True;
    }

    return sal_False;
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasElements(  )
    throw( uno::RuntimeException )
{
    const NameOrIndex *pItem;

    sal_Int32 nSurrogate;
    sal_Int32 nSurrogateCount = mpModelPool ? (sal_Int32)mpModelPool->GetItemCount( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( mnWhich, (USHORT)nSurrogate );

        if( pItem && pItem->GetName().Len() != 0 )
            return sal_True;
    }

    nSurrogateCount = mpStylePool ? (sal_Int32)mpStylePool->GetItemCount( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpStylePool->GetItem( mnWhich, (USHORT)nSurrogate );

        if( pItem && pItem->GetName().Len() != 0 )
            return sal_True;
    }

    return sal_False;
}

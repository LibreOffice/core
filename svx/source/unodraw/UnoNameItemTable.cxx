/*************************************************************************
 *
 *  $RCSfile: UnoNameItemTable.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-18 13:09:26 $
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

#include <set>

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#include "svdmodel.hxx"

#ifndef _SVX_UNONAMEITEMTABLE_HXX_
#include "UnoNameItemTable.hxx"
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "unoapi.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::vos;

SvxUnoNameItemTable::SvxUnoNameItemTable( SdrModel* pModel, USHORT nWhich, BYTE nMemberId ) throw()
: mpModel( pModel ),
  mpModelPool( pModel ? &pModel->GetItemPool() : NULL ),
  mnWhich( nWhich ), mnMemberId( nMemberId )
{
    if( pModel )
        StartListening( *pModel );
}

SvxUnoNameItemTable::~SvxUnoNameItemTable() throw()
{
    if( mpModel )
        EndListening( *mpModel );
    dispose();
}

bool SvxUnoNameItemTable::isValid( const NameOrIndex* pItem ) const
{
    return pItem && (pItem->GetName().Len() != 0);
}

void SvxUnoNameItemTable::dispose()
{
    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    while( aIter != aEnd )
    {
        delete (*aIter++);
    }

    maItemSetVector.clear();
}

void SvxUnoNameItemTable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw()
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint && HINT_MODELCLEARED == pSdrHint->GetKind() )
        dispose();
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

void SAL_CALL SvxUnoNameItemTable::ImplInsertByName( const OUString& aName, const uno::Any& aElement )
{
    SfxItemSet* mpInSet = new SfxItemSet( *mpModelPool, mnWhich, mnWhich );
    maItemSetVector.push_back( mpInSet );

    NameOrIndex* pNewItem = createItem();
    pNewItem->SetName( String( aName ) );
    pNewItem->PutValue( aElement, mnMemberId );
    mpInSet->Put( *pNewItem, mnWhich );
    delete pNewItem;
}

// XNameContainer
void SAL_CALL SvxUnoNameItemTable::insertByName( const OUString& aApiName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( hasByName( aApiName ) )
        throw container::ElementExistException();

    String aName;
    SvxUnogetInternalNameForItem( mnWhich, aApiName, aName );

    ImplInsertByName( aName, aElement );
}



void SAL_CALL SvxUnoNameItemTable::removeByName( const OUString& aApiName )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    // a little quickfix for 2.0 to let applications clear api
    // created items that are not used
    if( aApiName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("~clear~") ) )
    {
        dispose();
        return;
    }

    String Name;
    SvxUnogetInternalNameForItem( mnWhich, aApiName, Name );

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;
    const String aSearchName( Name );

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter)->Get( mnWhich ) );
        if( pItem->GetName() == aSearchName )
        {
            delete (*aIter);
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
    OGuard aGuard( Application::GetSolarMutex() );

    String aName;
    SvxUnogetInternalNameForItem( mnWhich, aApiName, aName );

    ItemPoolVector::iterator aIter = maItemSetVector.begin();
    const ItemPoolVector::iterator aEnd = maItemSetVector.end();

    NameOrIndex *pItem;
    const String aSearchName( aName );

    while( aIter != aEnd )
    {
        pItem = (NameOrIndex *)&((*aIter)->Get( mnWhich ) );
        if( pItem->GetName() == aSearchName )
        {
            NameOrIndex* pNewItem = createItem();
            pNewItem->SetName( aSearchName );
            if( !pNewItem->PutValue( aElement, mnMemberId ) || !isValid( pNewItem ) )
                throw lang::IllegalArgumentException();

            (*aIter)->Put( *pNewItem );
            return;
        }
        aIter++;
    }

    // if it is not in our own sets, modify the pool!
    sal_Bool bFound = sal_False;

    USHORT nSurrogate;
    USHORT nCount = mpModelPool ? mpModelPool->GetItemCount( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( mnWhich, nSurrogate);
        if( pItem && pItem->GetName() == aSearchName )
        {
            pItem->PutValue( aElement, mnMemberId );
            bFound = sal_True;
            break;
        }
    }

    if( bFound )
        ImplInsertByName( aName, aElement );
    else
        throw container::NoSuchElementException();

    if( !hasByName( aName ) )
        throw container::NoSuchElementException();
}

// XNameAccess
uno::Any SAL_CALL SvxUnoNameItemTable::getByName( const OUString& aApiName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

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

            if( isValid( pItem ) && (pItem->GetName() == aSearchName) )
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
    OGuard aGuard( Application::GetSolarMutex() );

    std::set< OUString, comphelper::UStringLess > aNameSet;

    NameOrIndex *pItem;
    OUString aApiName;

    const sal_Int32 nSurrogateCount = mpModelPool ? (sal_Int32)mpModelPool->GetItemCount( mnWhich ) : 0;
    sal_Int32 nSurrogate;
    for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( mnWhich, (USHORT)nSurrogate );

        if( !isValid( pItem ) )
            continue;

        SvxUnogetApiNameForItem( mnWhich, pItem->GetName(), aApiName );
        aNameSet.insert( aApiName );
    }

    uno::Sequence< OUString > aSeq( aNameSet.size() );
    OUString* pNames = aSeq.getArray();

    std::set< OUString, comphelper::UStringLess >::iterator aIter( aNameSet.begin() );
    const std::set< OUString, comphelper::UStringLess >::iterator aEnd( aNameSet.end() );

    while( aIter != aEnd )
    {
        *pNames++ = *aIter++;
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasByName( const OUString& aApiName )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

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
        if( isValid( pItem ) && (pItem->GetName() == aSearchName) )
            return sal_True;
    }

    return sal_False;
}

sal_Bool SAL_CALL SvxUnoNameItemTable::hasElements(  )
    throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    const NameOrIndex *pItem;

    sal_Int32 nSurrogate;
    const sal_Int32 nSurrogateCount = mpModelPool ? (sal_Int32)mpModelPool->GetItemCount( mnWhich ) : 0;
    for( nSurrogate = 0; nSurrogate < nSurrogateCount; nSurrogate++ )
    {
        pItem = (NameOrIndex*)mpModelPool->GetItem( mnWhich, (USHORT)nSurrogate );

        if( isValid( pItem ) )
            return sal_True;
    }

    return sal_False;
}

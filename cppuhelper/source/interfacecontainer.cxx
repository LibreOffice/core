/*************************************************************************
 *
 *  $RCSfile: interfacecontainer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:15:28 $
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

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <hash_map>

#include <assert.h>

#include <com/sun/star/lang/XEventListener.hpp>


using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace cppu
{

//===================================================================
//===================================================================
//===================================================================
/**
 * Reallocate the sequence.
 */
static void realloc( Sequence< Reference< XInterface > > & rSeq, sal_Int32 nNewLen )
    SAL_THROW( () )
{
    Sequence< Reference< XInterface > > aNewSeq( nNewLen );
    Reference< XInterface > * pDest = aNewSeq.getArray();
    // getArray on a const sequence is faster
    const Reference< XInterface > * pSource = ((const Sequence< Reference< XInterface > > &)rSeq).getConstArray();
    for( sal_Int32 i = (nNewLen < rSeq.getLength() ? nNewLen : rSeq.getLength()) -1; i >= 0; i-- )
        pDest[i] = pSource[i];
    rSeq = aNewSeq;
}

/**
 * Remove an element from an interface sequence.
 */
static void sequenceRemoveElementAt( Sequence< Reference< XInterface > > & rSeq, sal_Int32 index )
    SAL_THROW( () )
{
    sal_Int32 nNewLen = rSeq.getLength() - 1;

    Sequence< Reference< XInterface > > aDestSeq( rSeq.getLength() - 1 );
    // getArray on a const sequence is faster
    const Reference< XInterface > * pSource = ((const Sequence< Reference< XInterface > > &)rSeq).getConstArray();
    Reference< XInterface > * pDest = aDestSeq.getArray();
    sal_Int32 i = 0;
    for( ; i < index; i++ )
        pDest[i] = pSource[i];
    for( sal_Int32 j = i ; j < nNewLen; j++ )
        pDest[j] = pSource[j+1];
    rSeq = aDestSeq;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef SAL_WNT
#pragma warning( disable: 4786 )
#endif

//===================================================================
//===================================================================
//===================================================================
OInterfaceIteratorHelper::OInterfaceIteratorHelper( OInterfaceContainerHelper & rCont_ )
    SAL_THROW( () )
    : rCont( rCont_ )
{
    MutexGuard aGuard( rCont.rMutex );
    if( rCont.bInUse )
        // worst case, two iterators at the same time
        rCont.copyAndResetInUse();
    bIsList = rCont_.bIsList;
    pData = rCont_.pData;
    if( bIsList )
    {
        rCont.bInUse = sal_True;
        nRemain = ((Sequence< Reference< XInterface > >*)pData)->getLength();
    }
    else if( pData )
    {
        ((XInterface *)pData)->acquire();
        nRemain = 1;
    }
    else
        nRemain = 0;
}

OInterfaceIteratorHelper::~OInterfaceIteratorHelper() SAL_THROW( () )
{
    sal_Bool bShared;
    {
    MutexGuard aGuard( rCont.rMutex );
    // bResetInUse protect the iterator against recursion
    bShared = pData == rCont.pData && rCont.bIsList;
    if( bShared )
    {
        OSL_ENSHURE( rCont.bInUse, "OInterfaceContainerHelper must be in use" );
        rCont.bInUse = sal_False;
    }
    }

    if( !bShared )
    {
        if( bIsList )
            // Sequence owned by the iterator
            delete (Sequence< Reference< XInterface > >*)pData;
        else if( pData )
            // Interface is acquired by the iterator
            ((XInterface*)pData)->release();
    }
}

XInterface * OInterfaceIteratorHelper::next() SAL_THROW( () )
{
    if( nRemain )
    {
        nRemain--;
        if( bIsList )
            // typecase to const,so the getArray method is faster
            return ((const Sequence< Reference< XInterface > >*)pData)->getConstArray()[nRemain].get();
        else if( pData )
            return (XInterface*)pData;
    }
    // exception
    return 0;
}

void OInterfaceIteratorHelper::remove() SAL_THROW( () )
{
    if( bIsList )
    {
        OSL_ASSERT( nRemain >= 0 &&
                    nRemain < ((const Sequence< Reference< XInterface > >*)pData)->getLength() );
        XInterface * p =
            ((const Sequence< Reference< XInterface > >*)pData)->getConstArray()[nRemain].get();
        rCont.removeInterface( * reinterpret_cast< const Reference< XInterface > * >( &p ) );
    }
    else
    {
        OSL_ASSERT( 0 == nRemain );
        rCont.removeInterface( * reinterpret_cast< const Reference< XInterface > * >(&pData));
    }
}

//===================================================================
//===================================================================
//===================================================================


OInterfaceContainerHelper::OInterfaceContainerHelper( Mutex & rMutex_ ) SAL_THROW( () )
    : rMutex( rMutex_ )
    , bInUse( sal_False )
    , bIsList( sal_False )
    , pData( 0 )
{
}

OInterfaceContainerHelper::~OInterfaceContainerHelper() SAL_THROW( () )
{
    OSL_ENSHURE( !bInUse, "~OInterfaceContainerHelper but is in use" );
    if( bIsList )
        delete (Sequence< Reference< XInterface > >*)pData;
    else if( pData )
        ((XInterface*)pData)->release();
}

sal_Int32 OInterfaceContainerHelper::getLength() const SAL_THROW( () )
{
    MutexGuard aGuard( rMutex );
    if( bIsList )
        return ((Sequence< Reference< XInterface > >*)pData)->getLength();
    else if( pData )
        return 1;
    return 0;
}

Sequence< Reference<XInterface> > OInterfaceContainerHelper::getElements() const SAL_THROW( () )
{
    MutexGuard aGuard( rMutex );
    if( bIsList )
        return *(Sequence< Reference< XInterface > >*)pData;
    else if( pData )
    {
        Reference<XInterface> x( (XInterface *)pData );
        return Sequence< Reference< XInterface > >( &x, 1 );
    }
    return Sequence< Reference< XInterface > >();
}

void OInterfaceContainerHelper::copyAndResetInUse() SAL_THROW( () )
{
    OSL_ENSHURE( bInUse, "OInterfaceContainerHelper not in use" );
    if( bInUse )
    {
        // this should be the worst case. If a iterator is active
        // and a new Listener is added.
        if( bIsList )
            pData = new Sequence< Reference< XInterface > >( *(Sequence< Reference< XInterface > >*)pData );
        else if( pData )
            ((XInterface*)pData)->acquire();

        bInUse = sal_False;
    }
}

sal_Int32 OInterfaceContainerHelper::addInterface( const Reference<XInterface> & rListener ) SAL_THROW( () )
{
    assert( rListener.is() );
    MutexGuard aGuard( rMutex );
    if( bInUse )
        copyAndResetInUse();

    if( bIsList )
    {
        sal_Int32 nLen = ((Sequence< Reference< XInterface > >*)pData)->getLength();
        realloc( *(Sequence< Reference< XInterface > >*)pData, nLen +1 );
        ((Sequence< Reference< XInterface > >*)pData)->getArray()[ nLen ] = rListener;
        return nLen +1;
    }
    else if( pData )
    {
        Sequence< Reference< XInterface > > * pSeq = new Sequence< Reference< XInterface > >( 2 );
        Reference<XInterface> * pArray = pSeq->getArray();
        pArray[0] = (XInterface *)pData;
        pArray[1] = rListener;
        ((XInterface *)pData)->release();
        pData = pSeq;
        bIsList = sal_True;
        return 2;
    }
    else
    {
        pData = rListener.get();
        if( rListener.is() )
            rListener->acquire();
        return 1;
    }
}

sal_Int32 OInterfaceContainerHelper::removeInterface( const Reference<XInterface> & rListener ) SAL_THROW( () )
{
    assert( rListener.is() );
    MutexGuard aGuard( rMutex );
    if( bInUse )
        copyAndResetInUse();

    if( bIsList )
    {
        const Reference<XInterface> * pL = ((const Sequence< Reference< XInterface > >*)pData)->getConstArray();
        sal_Int32 nLen = ((Sequence< Reference< XInterface > >*)pData)->getLength();
        sal_Int32 i;
        for( i = 0; i < nLen; i++ )
        {
            // It is not valid to compare the Pointer direkt, but is is is much
            // more faster.
            if( pL[i].get() == rListener.get() )
            {
                sequenceRemoveElementAt( *(Sequence< Reference< XInterface > >*)pData, i );
                break;
            }
        }

        if( i == nLen )
        {
            // interface not found, use the correct compare method
            for( i = 0; i < nLen; i++ )
            {
                if( pL[i] == rListener )
                {
                    sequenceRemoveElementAt(*(Sequence< Reference< XInterface > >*)pData, i );
                    break;
                }
            }
        }

        if( ((Sequence< Reference< XInterface > >*)pData)->getLength() == 1 )
        {
            XInterface * pL = ((const Sequence< Reference< XInterface > >*)pData)->getConstArray()[0].get();
            pL->acquire();
            delete (Sequence< Reference< XInterface > >*)pData;
            pData = pL;
            bIsList = sal_False;
            return 1;
        }
        else
            return ((Sequence< Reference< XInterface > >*)pData)->getLength();
    }
    else if( Reference<XInterface>( (XInterface*)pData ) == rListener )
    {
        ((XInterface *)pData)->release();
        pData = 0;
    }
    return pData ? 1 : 0;
}

void OInterfaceContainerHelper::disposeAndClear( const EventObject & rEvt ) SAL_THROW( () )
{
    ClearableMutexGuard aGuard( rMutex );
    OInterfaceIteratorHelper aIt( *this );
    // Container freigeben, falls im disposing neue Einträge kommen
    OSL_ENSHURE( !bIsList || bInUse, "OInterfaceContainerHelper not in use" );
    if( !bIsList && pData )
        ((XInterface *)pData)->release();
    // set the member to null, the iterator delete the values
    pData = NULL;
    bIsList = sal_False;
    bInUse = sal_False;
    aGuard.clear();
    while( aIt.hasMoreElements() )
    {
        try
        {
            Reference<XEventListener > xLst( aIt.next(), UNO_QUERY );
            if( xLst.is() )
                xLst->disposing( rEvt );
        }
        catch ( RuntimeException & )
        {
            // be robust, if e.g. a remote bridge has disposed already.
            // there is no way, to delegate the error to the caller :o(.
        }
    }
}


void OInterfaceContainerHelper::clear() SAL_THROW( () )
{
    ClearableMutexGuard aGuard( rMutex );
    OInterfaceIteratorHelper aIt( *this );
    // Container freigeben, falls im disposing neue Einträge kommen
    OSL_ENSHURE( !bIsList || bInUse, "OInterfaceContainerHelper not in use" );
    if( !bIsList && pData )
        ((XInterface *)pData)->release();
    // set the member to null, the iterator delete the values
    pData = 0;
    bIsList = sal_False;
    bInUse = sal_False;
    // release mutex before aIt destructor call
    aGuard.clear();
}


}


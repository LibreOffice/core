/*************************************************************************
 *
 *  $RCSfile: stdtabcontrollermodel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
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

#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif

#include <toolkit/controls/stdtabcontrollermodel.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/property.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <tools/debug.hxx>

#define UNOCONTROL_STREAMVERSION    (short)2

//  ----------------------------------------------------
//  class UnoControlModelEntryList
//  ----------------------------------------------------
UnoControlModelEntryList::UnoControlModelEntryList()
{
}

UnoControlModelEntryList::~UnoControlModelEntryList()
{
    Reset();
}

void UnoControlModelEntryList::Reset()
{
    for ( sal_uInt32 n = Count(); n; )
        DestroyEntry( --n );
}

void UnoControlModelEntryList::DestroyEntry( sal_uInt32 nEntry )
{
    UnoControlModelEntry* pEntry = GetObject( nEntry );

    if ( pEntry->bGroup )
        delete pEntry->pGroup;
    else
        delete pEntry->pxControl;

    Remove( nEntry );
    delete pEntry;
}

//  ----------------------------------------------------
//  class StdTabControllerModel
//  ----------------------------------------------------
StdTabControllerModel::StdTabControllerModel()
{
    mbGroupControl = sal_True;
}

StdTabControllerModel::~StdTabControllerModel()
{
}

sal_uInt32 StdTabControllerModel::ImplGetControlCount( const UnoControlModelEntryList& rList ) const
{
    sal_uInt32 nCount = 0;
    sal_uInt32 nEntries = rList.Count();
    for ( sal_uInt32 n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = rList.GetObject( n );
        if ( pEntry->bGroup )
            nCount += ImplGetControlCount( *pEntry->pGroup );
        else
            nCount++;
    }
    return nCount;
}

void StdTabControllerModel::ImplGetControlModels( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > ** ppRefs, const UnoControlModelEntryList& rList ) const
{
    sal_uInt32 nEntries = rList.Count();
    for ( sal_uInt32 n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = rList.GetObject( n );
        if ( pEntry->bGroup )
            ImplGetControlModels( ppRefs, *pEntry->pGroup );
        else
        {
            **ppRefs = *pEntry->pxControl;
            (*ppRefs)++;
        }
    }
}

void StdTabControllerModel::ImplSetControlModels( UnoControlModelEntryList& rList, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) const
{
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = Controls.getConstArray();
    sal_uInt32 nControls = Controls.getLength();
    for ( sal_uInt32 n = 0; n < nControls; n++ )
    {
        UnoControlModelEntry* pNewEntry = new UnoControlModelEntry;
        pNewEntry->bGroup = sal_False;
        pNewEntry->pxControl = new ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > ;
        *pNewEntry->pxControl = pRefs[n];
        rList.Insert( pNewEntry, LIST_APPEND );
    }
}

sal_uInt32 StdTabControllerModel::ImplGetControlPos( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xCtrl, const UnoControlModelEntryList& rList ) const
{
    for ( sal_uInt32 n = rList.Count(); n; )
    {
        UnoControlModelEntry* pEntry = rList.GetObject( --n );
        if ( !pEntry->bGroup && ( *pEntry->pxControl == xCtrl ) )
            return n;
    }
    return CONTROLPOS_NOTFOUND;
}

void ImplWriteControls( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream > & OutStream, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rCtrls )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream >  xMark( OutStream, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    sal_uInt32 nStoredControls = 0;
    sal_Int32 nDataBeginMark = xMark->createMark();

    OutStream->writeLong( 0L ); // DataLen
    OutStream->writeLong( 0L ); // nStoredControls

    sal_uInt32 nCtrls = rCtrls.getLength();
    for ( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xI = rCtrls.getConstArray()[n];
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject >  xPO( xI, ::com::sun::star::uno::UNO_QUERY );
        DBG_ASSERT( xPO.is(), "write: Control doesn't support XPersistObject" );
        if ( xPO.is() )
        {
            OutStream->writeObject( xPO );
            nStoredControls++;
        }
    }
    sal_Int32 nDataLen = xMark->offsetToMark( nDataBeginMark );
    xMark->jumpToMark( nDataBeginMark );
    OutStream->writeLong( nDataLen );
    OutStream->writeLong( nStoredControls );
    xMark->jumpToFurthest();
    xMark->deleteMark(nDataBeginMark);
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > ImplReadControls( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream > & InStream )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream >  xMark( InStream, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    sal_Int32 nDataBeginMark = xMark->createMark();

    sal_Int32 nDataLen = InStream->readLong();
    sal_uInt32 nCtrls = InStream->readLong();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq( nCtrls );
    for ( sal_uInt32 n = 0; n < nCtrls; n++ )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject >  xObj = InStream->readObject();
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xI( xObj, ::com::sun::star::uno::UNO_QUERY );
        aSeq.getArray()[n] = xI;
    }

    // Falls bereits mehr drinsteht als diese Version kennt:
    xMark->jumpToMark( nDataBeginMark );
    InStream->skipBytes( nDataLen );
    xMark->deleteMark(nDataBeginMark);
    return aSeq;
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any StdTabControllerModel::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTabControllerModel*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::io::XPersistObject*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : OWeakAggObject::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( StdTabControllerModel )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject>* ) NULL )
IMPL_XTYPEPROVIDER_END

sal_Bool StdTabControllerModel::getGroupControl(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return mbGroupControl;
}

void StdTabControllerModel::setGroupControl( sal_Bool GroupControl ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    mbGroupControl = GroupControl;
}

void StdTabControllerModel::setControlModels( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maControls.Reset();
    ImplSetControlModels( maControls, Controls );
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > StdTabControllerModel::getControlModels(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq( ImplGetControlCount( maControls ) );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = aSeq.getArray();
    ImplGetControlModels( &pRefs, maControls );
    return aSeq;
}

void StdTabControllerModel::setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, const ::rtl::OUString& GroupName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Die Controls stehen eventuel flach in der Liste und werden jetzt gruppiert.
    // Verschachtelte Gruppen sind erstmal nicht moeglich...
    // Das erste Element der Gruppe welches auch schon in der flachen Liste
    // stand bestimmt die Position der Gruppe.

    UnoControlModelEntry* pNewEntry = new UnoControlModelEntry;
    pNewEntry->bGroup = sal_True;
    pNewEntry->pGroup = new UnoControlModelEntryList;
    pNewEntry->pGroup->SetName( GroupName );
    ImplSetControlModels( *pNewEntry->pGroup, Group );

    sal_Bool bInserted = sal_False;
    sal_uInt32 nElements = pNewEntry->pGroup->Count();
    for ( sal_uInt32 n = 0; n < nElements; n++ )
    {
        UnoControlModelEntry* pEntry = pNewEntry->pGroup->GetObject( n );
        if ( !pEntry->bGroup )
        {
            sal_uInt32 nPos = ImplGetControlPos( *pEntry->pxControl, maControls );
            // Eigentlich sollten alle Controls vorher in der flachen Liste stehen
            DBG_ASSERT( nPos != CONTROLPOS_NOTFOUND, "setGroup - Element not found" );
            if ( nPos != CONTROLPOS_NOTFOUND )
            {
                maControls.DestroyEntry( nPos );
                if ( !bInserted )
                {
                    maControls.Insert( pNewEntry, nPos );
                    bInserted = sal_True;
                }
            }
        }
    }
    if ( !bInserted )
        maControls.Insert( pNewEntry, LIST_APPEND );
}

sal_Int32 StdTabControllerModel::getGroupCount(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // erstmal nur eine Ebene...
    // Das Model und die Impl-Methoden arbeiten zwar rekursiv, aber das wird
    // erstmal nich nach aussen gegeben.

    sal_Int32 nGroups = 0;
    sal_uInt32 nEntries = maControls.Count();
    for ( sal_uInt32 n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = maControls.GetObject( n );
        if ( pEntry->bGroup )
            nGroups++;
    }
    return nGroups;
}

void StdTabControllerModel::getGroup( sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rGroup, ::rtl::OUString& rName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq;
    sal_uInt32 nG = 0;
    sal_uInt32 nEntries = maControls.Count();
    for ( sal_uInt32 n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = maControls.GetObject( n );
        if ( pEntry->bGroup )
        {
            if ( nG == nGroup )
            {
                sal_uInt32 nCount = ImplGetControlCount( *pEntry->pGroup );
                aSeq = ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >( nCount );
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > * pRefs = aSeq.getArray();
                ImplGetControlModels( &pRefs, *pEntry->pGroup );
                rName = pEntry->pGroup->GetName();
                break;
            }
            nG++;
        }
    }
    rGroup = aSeq;
}

void StdTabControllerModel::getGroupByName( const ::rtl::OUString& rName, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& rGroup ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nGroup = 0;
    sal_uInt32 nEntries = maControls.Count();
    for ( sal_uInt32 n = 0; n < nEntries; n++ )
    {
        UnoControlModelEntry* pEntry = maControls.GetObject( n );
        if ( pEntry->bGroup )
        {
            if ( pEntry->pGroup->GetName() == rName )
            {
                ::rtl::OUString Dummy;
                getGroup( nGroup, rGroup, Dummy );
                break;
            }
            nGroup++;
        }
    }
}


// ::com::sun::star::io::XPersistObject
::rtl::OUString StdTabControllerModel::getServiceName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_TabControllerModel );
}

void StdTabControllerModel::write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream >  xMark( OutStream, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xMark.is(), "write: no XMarkableStream!" );

    OutStream->writeShort( UNOCONTROL_STREAMVERSION );

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aCtrls = getControlModels();
    ImplWriteControls( OutStream, aCtrls );

    sal_uInt32 nGroups = getGroupCount();
    OutStream->writeLong( nGroups );
    for ( sal_uInt32 n = 0; n < nGroups; n++ )
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aGroupCtrls;
        ::rtl::OUString aGroupName;
        getGroup( n, aGroupCtrls, aGroupName );
        OutStream->writeUTF( aGroupName );
        ImplWriteControls( OutStream, aGroupCtrls );
    }
}

void StdTabControllerModel::read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    short nVersion = InStream->readShort();

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aSeq = ImplReadControls( InStream );
    setControlModels( aSeq );

    sal_uInt32 nGroups = InStream->readLong();
    for ( sal_uInt32 n = 0; n < nGroups; n++ )
    {
        ::rtl::OUString aGroupName = InStream->readUTF();
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > aCtrlSeq = ImplReadControls( InStream );
        setGroup( aCtrlSeq, aGroupName );
    }
}






/*************************************************************************
 *
 *  $RCSfile: MasterPropertySet.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2001-07-26 12:28:53 $
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
 *  Contributor(s): Martin Gallwey ( gallwey@sun.com )
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_MASTERPROPERTYSET_HXX_
#include <comphelper/MasterPropertySet.hxx>
#endif
#ifndef _COMPHELPER_MASTERPROPERTYSETINFO_HXX_
#include <comphelper/MasterPropertySetInfo.hxx>
#endif
#ifndef _COMPHELPER_CHAINABLEPROPERTYSET_HXX_
#include <comphelper/ChainablePropertySet.hxx>
#endif
#ifndef _COMPHELPER_CHAINABLEPROPERTYSETINFO_HXX_
#include <comphelper/ChainablePropertySetInfo.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using vos::IMutex;

SlaveData::SlaveData ( ChainablePropertySet *pSlave)
: mpSlave ( pSlave )
, mbInit ( sal_False )
, mxSlave ( pSlave )
{
}

MasterPropertySet::MasterPropertySet( comphelper::MasterPropertySetInfo* pInfo, IMutex *pMutex )
    throw()
: mpInfo ( pInfo )
, mxInfo ( pInfo )
, mnLastId ( 0 )
, mpMutex ( pMutex )
{
}

void MasterPropertySet::lockMutex()
{
    if (mpMutex)
        mpMutex->acquire();
}
void MasterPropertySet::unlockMutex()
{
    if (mpMutex)
        mpMutex->release();
}

MasterPropertySet::~MasterPropertySet()
    throw()
{
    SlaveMap::iterator aEnd = maSlaveMap.end(), aIter = maSlaveMap.begin();
    while (aIter != aEnd )
    {
        delete (*aIter).second;
        aIter++;
    }
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL MasterPropertySet::getPropertySetInfo(  )
    throw(RuntimeException)
{
    return mxInfo;
}

void MasterPropertySet::registerSlave ( ChainablePropertySet *pNewSet )
    throw()
{
    maSlaveMap [ ++mnLastId ] = new SlaveData ( pNewSet );
    mpInfo->add ( pNewSet->mpInfo->maMap, mnLastId );
}

void SAL_CALL MasterPropertySet::setPropertyValue( const ::rtl::OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    lockMutex();
    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException();

    if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
    {
        _preSetValues();
        _setSingleValue( *((*aIter).second->mpInfo), rValue );
        _postSetValues();
    }
    else
    {
        ChainablePropertySet * pSlave = maSlaveMap [ (*aIter).second->mnMapId ]->mpSlave;
        pSlave->lockMutex();
        pSlave->_preSetValues();
        pSlave->_setSingleValue( *((*aIter).second->mpInfo), rValue );
        pSlave->_postSetValues();
        pSlave->unlockMutex();
    }
    unlockMutex();
}

Any SAL_CALL MasterPropertySet::getPropertyValue( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    lockMutex();
    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException();

    Any aAny;
    if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
    {
        _preGetValues();
        _getSingleValue( *((*aIter).second->mpInfo), aAny );
        _postGetValues();
    }
    else
    {
        ChainablePropertySet * pSlave = maSlaveMap [ (*aIter).second->mnMapId ]->mpSlave;
        pSlave->lockMutex();
        pSlave->_preGetValues();
        pSlave->_getSingleValue( *((*aIter).second->mpInfo), aAny );
        pSlave->_postGetValues();
        pSlave->unlockMutex();
    }
    unlockMutex();
    return aAny;
}

void SAL_CALL MasterPropertySet::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

// XMultiPropertySet
void SAL_CALL MasterPropertySet::setPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames, const Sequence< Any >& aValues )
    throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    lockMutex();
    const sal_Int32 nCount = aPropertyNames.getLength();

    if( nCount != aValues.getLength() )
        throw IllegalArgumentException();

    if( nCount )
    {
        _preSetValues();

        const Any * pAny = aValues.getConstArray();
        const OUString * pString = aPropertyNames.getConstArray();
        PropertyDataHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException();

            if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
                _setSingleValue( *((*aIter).second->mpInfo), *pAny );
            else
            {
                SlaveData * pSlave = maSlaveMap [ (*aIter).second->mnMapId ];
                if (!pSlave->IsInit())
                {
                    pSlave->mpSlave->lockMutex();
                    pSlave->mpSlave->_preSetValues();
                    pSlave->SetInit ( sal_True );
                }
                pSlave->mpSlave->_setSingleValue( *((*aIter).second->mpInfo), *pAny );
            }
        }

        _postSetValues();
        SlaveMap::const_iterator aSlaveIter = maSlaveMap.begin(), aSlaveEnd = maSlaveMap.end();
        while (aSlaveIter != aSlaveEnd)
        {
            if ( (*aSlaveIter).second->IsInit())
            {
                (*aSlaveIter).second->mpSlave->_postSetValues();
                (*aSlaveIter).second->SetInit ( sal_False );
                (*aSlaveIter).second->mpSlave->unlockMutex();
            }
            ++aSlaveIter;
        }
    }
    unlockMutex();
}

Sequence< Any > SAL_CALL MasterPropertySet::getPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames )
    throw(RuntimeException)
{
    lockMutex();
    const sal_Int32 nCount = aPropertyNames.getLength();

    Sequence < Any > aValues ( nCount );

    if( nCount )
    {
        _preGetValues();

        Any * pAny = aValues.getArray();
        const OUString * pString = aPropertyNames.getConstArray();
        PropertyDataHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pAny )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException();

            if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
                _getSingleValue( *((*aIter).second->mpInfo), *pAny );
            else
            {
                SlaveData * pSlave = maSlaveMap [ (*aIter).second->mnMapId ];
                if (!pSlave->IsInit())
                {
                    pSlave->mpSlave->lockMutex();
                    pSlave->mpSlave->_preGetValues();
                    pSlave->SetInit ( sal_True );
                }
                pSlave->mpSlave->_getSingleValue( *((*aIter).second->mpInfo), *pAny );
            }
        }

        _postSetValues();
        SlaveMap::const_iterator aSlaveIter = maSlaveMap.begin(), aSlaveEnd = maSlaveMap.end();
        while (aSlaveIter != aSlaveEnd)
        {
            if ( (*aSlaveIter).second->IsInit())
            {
                (*aSlaveIter).second->mpSlave->_postSetValues();
                (*aSlaveIter).second->mpSlave->unlockMutex();
                (*aSlaveIter).second->SetInit ( sal_False );
            }
            ++aSlaveIter;
        }
    }
    unlockMutex();
    return aValues;
}

void SAL_CALL MasterPropertySet::addPropertiesChangeListener( const Sequence< ::rtl::OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener )
    throw(RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& xListener )
    throw(RuntimeException)
{
    // todo
}

void SAL_CALL MasterPropertySet::firePropertiesChangeEvent( const Sequence< ::rtl::OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener )
    throw(RuntimeException)
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL MasterPropertySet::getPropertyState( const ::rtl::OUString& PropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    PropertyDataHash::const_iterator aIter =  mpInfo->maMap.find( PropertyName );
    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException();

    PropertyState aState;

    if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
    {
        _preGetPropertyState();
        _getPropertyState( *((*aIter).second->mpInfo), aState );
        _postGetPropertyState();
    }
    else
    {
        ChainablePropertySet * pSlave = maSlaveMap [ (*aIter).second->mnMapId ]->mpSlave;
        pSlave->lockMutex();
        pSlave->_preGetPropertyState();
        pSlave->_getPropertyState( *((*aIter).second->mpInfo), aState );
        pSlave->_postGetPropertyState();
        pSlave->unlockMutex();
    }

    return aState;
}

Sequence< PropertyState > SAL_CALL MasterPropertySet::getPropertyStates( const Sequence< ::rtl::OUString >& rPropertyNames )
    throw(UnknownPropertyException, RuntimeException)
{
    const sal_Int32 nCount = rPropertyNames.getLength();

    Sequence< PropertyState > aStates( nCount );
    if( nCount )
    {
        PropertyState * pState = aStates.getArray();
        const OUString * pString = rPropertyNames.getConstArray();
        PropertyDataHash::const_iterator aEnd = mpInfo->maMap.end(), aIter;
        _preGetPropertyState();

        for ( sal_Int32 i = 0; i < nCount; ++i, ++pString, ++pState )
        {
            aIter = mpInfo->maMap.find ( *pString );
            if ( aIter == aEnd )
                throw UnknownPropertyException();

            if ( (*aIter).second->mnMapId == 0 ) // 0 means it's one of ours !
                _getPropertyState( *((*aIter).second->mpInfo), *pState );
            else
            {
                SlaveData * pSlave = maSlaveMap [ (*aIter).second->mnMapId ];
                if (!pSlave->IsInit())
                {
                    pSlave->mpSlave->_preGetPropertyState();
                    pSlave->SetInit ( sal_True );
                }
                pSlave->mpSlave->_getPropertyState( *((*aIter).second->mpInfo), *pState );
            }
        }
        _postGetPropertyState();
        SlaveMap::const_iterator aSlaveIter = maSlaveMap.begin(), aSlaveEnd = maSlaveMap.end();
        while (aSlaveIter != aSlaveEnd)
        {
            if ( (*aSlaveIter).second->IsInit())
            {
                (*aSlaveIter).second->mpSlave->_postGetPropertyState();
                (*aSlaveIter).second->SetInit ( sal_False );
            }
            ++aSlaveIter;
        }
    }
    return aStates;
}

void SAL_CALL MasterPropertySet::setPropertyToDefault( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException();
    _setPropertyToDefault( *((*aIter).second->mpInfo) );
}

Any SAL_CALL MasterPropertySet::getPropertyDefault( const ::rtl::OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertyDataHash::const_iterator aIter = mpInfo->maMap.find ( rPropertyName );

    if( aIter == mpInfo->maMap.end())
        throw UnknownPropertyException();
    return _getPropertyDefault( *((*aIter).second->mpInfo) );
}

void MasterPropertySet::_preGetPropertyState ()
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    OSL_ENSURE( sal_False, "you have to implement this yourself!");
}

void MasterPropertySet::_getPropertyState( const comphelper::PropertyInfo& rInfo, PropertyState& rStates )
    throw(UnknownPropertyException )
{
    OSL_ENSURE( sal_False, "you have to implement this yourself!");
}

void MasterPropertySet::_postGetPropertyState ()
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException )
{
    OSL_ENSURE( sal_False, "you have to implement this yourself!");
}

void MasterPropertySet::_setPropertyToDefault( const comphelper::PropertyInfo& rInfo )
    throw(UnknownPropertyException )
{
    OSL_ENSURE( sal_False, "you have to implement this yourself!");
}

Any MasterPropertySet::_getPropertyDefault( const comphelper::PropertyInfo& rInfo )
    throw(UnknownPropertyException, WrappedTargetException )
{
    OSL_ENSURE( sal_False, "you have to implement this yourself!");
    Any aAny;
    return aAny;
}

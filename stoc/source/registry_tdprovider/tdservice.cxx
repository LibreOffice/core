/*************************************************************************
 *
 *  $RCSfile: tdservice.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2002-11-11 08:35:47 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif

using namespace com::sun::star;

namespace stoc_rdbtdp
{

//__________________________________________________________________________________________________
// virtual
ServiceTypeDescriptionImpl::~ServiceTypeDescriptionImpl()
{
    delete _pMandatoryServices;
    delete _pOptionalServices;
    delete _pMandatoryInterfaces;
    delete _pOptionalInterfaces;
    delete _pProps;

    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
// virtual
TypeClass ServiceTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_SERVICE;
}
//__________________________________________________________________________________________________
// virtual
OUString ServiceTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XServiceTypeDescription
//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XServiceTypeDescription > > SAL_CALL
ServiceTypeDescriptionImpl::getMandatoryServices()
    throw ( RuntimeException )
{
    getReferences();
    return *_pMandatoryServices;
}

//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XServiceTypeDescription > > SAL_CALL
ServiceTypeDescriptionImpl::getOptionalServices()
    throw ( RuntimeException )
{
    getReferences();
    return *_pOptionalServices;
}

//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XInterfaceTypeDescription > > SAL_CALL
ServiceTypeDescriptionImpl::getMandatoryInterfaces()
    throw ( RuntimeException )
{
    getReferences();
    return *_pMandatoryInterfaces;
}

//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XInterfaceTypeDescription > > SAL_CALL
ServiceTypeDescriptionImpl::getOptionalInterfaces()
    throw ( RuntimeException )
{
    getReferences();
    return *_pOptionalInterfaces;
}

//__________________________________________________________________________________________________
// virtual
Sequence< PropertyDescription > SAL_CALL
ServiceTypeDescriptionImpl::getProperties()
{
    if ( !_pProps )
    {
        RegistryTypeReaderLoader aLoader;
        RegistryTypeReader aReader(
            aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
            _aBytes.getLength(), sal_False );

        sal_uInt16 nFields = (sal_uInt16)aReader.getFieldCount();
        Sequence< PropertyDescription > * pTempProps =
            new Sequence< PropertyDescription >( nFields );
        PropertyDescription * pProps = pTempProps->getArray();

        while ( nFields-- )
        {
            // PropertyDescription.Name
            pProps[ nFields ].Name = aReader.getFieldName( nFields );

            // PropertyDescription.TypeDescription
            try
            {
                _xTDMgr->getByHierarchicalName(
                    aReader.getFieldType( nFields ).replace( '/', '.' ) )
                        >>= pProps[ nFields ].TypeDescription;
            }
            catch ( NoSuchElementException const & )
            {
            }
            OSL_ENSURE( pProps[ nFields ].TypeDescription.is(),
                        "### no type description for property!" );

            // PropertyDescription.Flags
            RTFieldAccess nFlags = aReader.getFieldAccess( nFields );

            sal_Int16 nAttribs = 0;
            if ( nFlags & RT_ACCESS_READONLY )
                nAttribs |= beans::PropertyAttribute::READONLY;
            if ( nFlags & RT_ACCESS_OPTIONAL )
                nAttribs |= beans::PropertyAttribute::OPTIONAL;
            if ( nFlags & RT_ACCESS_MAYBEVOID )
                nAttribs |= beans::PropertyAttribute::MAYBEVOID;
            if ( nFlags & RT_ACCESS_BOUND )
                nAttribs |= beans::PropertyAttribute::BOUND;
            if ( nFlags & RT_ACCESS_CONSTRAINED )
                nAttribs |= beans::PropertyAttribute::CONSTRAINED;
            if ( nFlags & RT_ACCESS_TRANSIENT )
                nAttribs |= beans::PropertyAttribute::TRANSIENT;
            if ( nFlags & RT_ACCESS_MAYBEAMBIGUOUS )
                nAttribs |= beans::PropertyAttribute::MAYBEAMBIGUOUS;
            if ( nFlags & RT_ACCESS_MAYBEDEFAULT )
                nAttribs |= beans::PropertyAttribute::MAYBEDEFAULT;
            if ( nFlags & RT_ACCESS_REMOVEABLE )
                nAttribs |= beans::PropertyAttribute::REMOVEABLE;

            OSL_ENSURE( !(nFlags & RT_ACCESS_PROPERTY),
                        "### RT_ACCESS_PROPERTY is unexpected here!" );
            OSL_ENSURE( !(nFlags & RT_ACCESS_ATTRIBUTE),
                        "### RT_ACCESS_ATTRIBUTE is unexpected here!" );
            OSL_ENSURE( !(nFlags & RT_ACCESS_CONST),
                        "### RT_ACCESS_CONST is unexpected here!" );
            // always set, unless RT_ACCESS_READONLY is set.
            //OSL_ENSURE( !(nFlags & RT_ACCESS_READWRITE),
            //            "### RT_ACCESS_READWRITE is unexpected here" );
            OSL_ENSURE( !(nFlags & RT_ACCESS_DEFAULT),
                        "### RT_ACCESS_DEAFAULT is unexpected here" );

            pProps[ nFields ].Flags = nAttribs;
        }

        ClearableMutexGuard aGuard( _aMutex );
        if ( _pProps )
        {
            aGuard.clear();
            delete pTempProps;
        }
        else
        {
            _pProps = pTempProps;
        }
    }

    return *_pProps;
}

//__________________________________________________________________________________________________
void ServiceTypeDescriptionImpl::getReferences()
    throw ( RuntimeException )
{
    if ( !_pMandatoryServices || !_pOptionalServices ||
         !_pMandatoryInterfaces || !_pOptionalInterfaces )
    {
        RegistryTypeReaderLoader aLoader;
        RegistryTypeReader aReader(
            aLoader, (const sal_uInt8 *)_aBytes.getConstArray(),
            _aBytes.getLength(), sal_False );

        sal_uInt32 nRefs = aReader.getReferenceCount();
        Sequence< Reference < XServiceTypeDescription > > * pTempMS
            = new Sequence< Reference < XServiceTypeDescription > >( nRefs );
        Sequence< Reference < XServiceTypeDescription > > * pTempOS
            = new Sequence< Reference < XServiceTypeDescription > >( nRefs );
        Sequence< Reference < XInterfaceTypeDescription > > * pTempMI
            = new Sequence< Reference < XInterfaceTypeDescription > >( nRefs );
        Sequence< Reference < XInterfaceTypeDescription > > * pTempOI
            = new Sequence< Reference < XInterfaceTypeDescription > >( nRefs );
        sal_uInt32 nMS = 0;
        sal_uInt32 nOS = 0;
        sal_uInt32 nMI = 0;
        sal_uInt32 nOI = 0;

        while ( nRefs-- )
        {
            RTReferenceType eType = aReader.getReferenceType( nRefs );
            switch ( eType )
            {
                case RT_REF_EXPORTS: // service
                {
                    uno::Any aTypeDesc;
                    try
                    {
                        aTypeDesc = _xTDMgr->getByHierarchicalName(
                            aReader.getReferenceName( nRefs )
                                .replace( '/', '.' ) );
                    }
                    catch ( NoSuchElementException const & )
                    {
                    }
                    OSL_ENSURE( aTypeDesc.hasValue(),
                                "### no type description!" );

                    RTFieldAccess nAccess = aReader.getReferenceAccess( nRefs );
                    if ( nAccess & RT_ACCESS_OPTIONAL )
                    {
                        // optional service
                        aTypeDesc >>= (*pTempOS)[ nOS ];
                        OSL_ENSURE( (*pTempOS)[ nOS ].is(),
                                    "### wrong type description type!" );
                        nOS++;
                    }
                    else
                    {
                        // mandatory service
                        aTypeDesc >>= (*pTempMS)[ nMS ];
                        OSL_ENSURE( (*pTempMS)[ nMS ].is(),
                                    "### wrong type description type!" );
                        nMS++;
                    }
                    break;
                }
                case RT_REF_SUPPORTS: // interface
                {
                    uno::Any aTypeDesc;
                    try
                    {
                        aTypeDesc = _xTDMgr->getByHierarchicalName(
                            aReader.getReferenceName( nRefs )
                                .replace( '/', '.' ) );
                    }
                    catch ( NoSuchElementException const & )
                    {
                    }
                    OSL_ENSURE( aTypeDesc.hasValue(),
                                "### no type description!" );

                    RTFieldAccess nAccess = aReader.getReferenceAccess( nRefs );
                    if ( nAccess & RT_ACCESS_OPTIONAL )
                    {
                        // optional interface
                        aTypeDesc >>= (*pTempOI)[ nOI ];
                        OSL_ENSURE( (*pTempOI)[ nOI ].is(),
                                    "### wrong type description type!" );
                        nOI++;
                    }
                    else
                    {
                        // mandatory interface
                        aTypeDesc >>= (*pTempMI)[ nMI ];
                        OSL_ENSURE( (*pTempMI)[ nMI ].is(),
                                    "### wrong type description type!" );
                        nMI++;
                    }
                    break;
                }
                case RT_REF_INVALID:
                case RT_REF_OBSERVES:
                case RT_REF_NEEDS:
                default:
                    OSL_ENSURE( sal_False,
                                "### unsupported reference type!" );
                    break;
            }
        }

        {
            ClearableMutexGuard aGuard( _aMutex );
            if ( _pMandatoryServices )
            {
                aGuard.clear();
                delete pTempMS;
            }
            else
            {
                pTempMS->realloc( nMS );
                _pMandatoryServices = pTempMS;
            }
        }

        {
            ClearableMutexGuard aGuard( _aMutex );
            if ( _pOptionalServices )
            {
                aGuard.clear();
                delete pTempOS;
            }
            else
            {
                pTempOS->realloc( nOS );
                _pOptionalServices = pTempOS;
            }
        }

        {
            ClearableMutexGuard aGuard( _aMutex );
            if ( _pMandatoryInterfaces )
            {
                aGuard.clear();
                delete pTempMI;
            }
            else
            {
                pTempMI->realloc( nMI );
                _pMandatoryInterfaces = pTempMI;
            }
        }

        {
            ClearableMutexGuard aGuard( _aMutex );
            if ( _pOptionalInterfaces )
            {
                aGuard.clear();
                delete pTempOI;
            }
            else
            {
                pTempOI->realloc( nOI );
                _pOptionalInterfaces = pTempOI;
            }
        }
    }
}

}



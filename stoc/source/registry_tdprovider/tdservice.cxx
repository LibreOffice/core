/*************************************************************************
 *
 *  $RCSfile: tdservice.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 15:04:58 $
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

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#include "com/sun/star/uno/RuntimeException.hpp"

#include "registry/reader.hxx"
#include "registry/version.h"

#ifndef _STOC_RDBTDP_BASE_HXX
#include "base.hxx"
#endif
#include "methoddescription.hxx"

#include <memory>

using namespace com::sun::star;

namespace {

class Constructor:
    public cppu::WeakImplHelper1< XServiceConstructorDescription >
{
public:
    Constructor(
        Reference< XHierarchicalNameAccess > const & manager,
        rtl::OUString const & name, Sequence< sal_Int8 > const & bytes,
        sal_uInt16 index):
        m_desc(manager, name, bytes, index) {}

    virtual ~Constructor() {}

    virtual sal_Bool SAL_CALL isDefaultConstructor() throw (RuntimeException)
    { return m_desc.getName().getLength() == 0; }

    virtual rtl::OUString SAL_CALL getName() throw (RuntimeException)
    { return m_desc.getName(); }

    virtual Sequence< Reference< XParameter > > SAL_CALL getParameters()
        throw (RuntimeException)
    { return m_desc.getParameters(); }

    virtual Sequence< Reference<XCompoundTypeDescription > > SAL_CALL
    getExceptions() throw (RuntimeException)
    { return m_desc.getExceptions(); }

private:
    Constructor(Constructor &); // not implemented
    void operator =(Constructor); // not implemented

    stoc::registry_tdprovider::MethodDescription m_desc;
};

}

namespace stoc_rdbtdp
{

//==================================================================================================
//
// class PropertyTypeDescriptionImpl
//
//==================================================================================================
class PropertyTypeDescriptionImpl : public WeakImplHelper1< XPropertyTypeDescription >
{
    OUString                      _aName;
    Reference< XTypeDescription > _xTD;
    sal_Int16                     _nFlags;

public:
    PropertyTypeDescriptionImpl( const OUString & rName,
                                 const Reference< XTypeDescription > & xTD,
                                 sal_Int16 nFlags )
    : _aName( rName ), _xTD( xTD ), _nFlags( nFlags )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }
    virtual ~PropertyTypeDescriptionImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass()
        throw( RuntimeException );
    virtual OUString SAL_CALL getName()
        throw( RuntimeException );

    // XPropertyTypeDescription
    virtual sal_Int16 SAL_CALL getPropertyFlags()
        throw ( RuntimeException );
    virtual Reference< XTypeDescription > SAL_CALL getPropertyTypeDescription()
        throw ( RuntimeException );
};

//__________________________________________________________________________________________________
// virtual
PropertyTypeDescriptionImpl::~PropertyTypeDescriptionImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
// virtual
TypeClass PropertyTypeDescriptionImpl::getTypeClass()
    throw ( RuntimeException )
{
    return TypeClass_PROPERTY;
}
//__________________________________________________________________________________________________
// virtual
OUString PropertyTypeDescriptionImpl::getName()
    throw ( RuntimeException )
{
    return _aName;
}

// XPropertyTypeDescription
//__________________________________________________________________________________________________
// virtual
sal_Int16 SAL_CALL PropertyTypeDescriptionImpl::getPropertyFlags()
    throw ( RuntimeException )
{
    return _nFlags;
}

//__________________________________________________________________________________________________
// virtual
Reference< XTypeDescription > SAL_CALL
PropertyTypeDescriptionImpl::getPropertyTypeDescription()
    throw ( RuntimeException )
{
    return _xTD;
}

//==================================================================================================
//
// ServiceTypeDescriptionImpl implementation
//
//==================================================================================================

//__________________________________________________________________________________________________
// virtual
ServiceTypeDescriptionImpl::~ServiceTypeDescriptionImpl()
{
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
    return _aMandatoryServices;
}

//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XServiceTypeDescription > > SAL_CALL
ServiceTypeDescriptionImpl::getOptionalServices()
    throw ( RuntimeException )
{
    getReferences();
    return _aOptionalServices;
}

//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XInterfaceTypeDescription > > SAL_CALL
ServiceTypeDescriptionImpl::getMandatoryInterfaces()
    throw ( RuntimeException )
{
    getReferences();
    return _aMandatoryInterfaces;
}

//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XInterfaceTypeDescription > > SAL_CALL
ServiceTypeDescriptionImpl::getOptionalInterfaces()
    throw ( RuntimeException )
{
    getReferences();
    return _aOptionalInterfaces;
}

//__________________________________________________________________________________________________
// virtual
Sequence< Reference< XPropertyTypeDescription > > SAL_CALL
ServiceTypeDescriptionImpl::getProperties()
    throw ( RuntimeException )
{
    {
        MutexGuard guard(getMutex());
        if (_pProps.get() != 0) {
            return *_pProps;
        }
    }

    typereg::Reader aReader(
        _aBytes.getConstArray(), _aBytes.getLength(), false, TYPEREG_VERSION_1);

    sal_uInt16 nFields = (sal_uInt16)aReader.getFieldCount();
    std::auto_ptr< Sequence< Reference< XPropertyTypeDescription > > >
        pTempProps(
            new Sequence< Reference< XPropertyTypeDescription > >(nFields));
    Reference< XPropertyTypeDescription > * pProps = pTempProps->getArray();

    while ( nFields-- )
    {
        // name
        OUStringBuffer aName( _aName );
        aName.appendAscii( "." );
        aName.append( aReader.getFieldName( nFields ) );

        // type description
        Reference< XTypeDescription > xTD;
        try
        {
            _xTDMgr->getByHierarchicalName(
                aReader.getFieldTypeName( nFields ).replace( '/', '.' ) )
                    >>= xTD;
        }
        catch ( NoSuchElementException const & )
        {
        }
        OSL_ENSURE( xTD.is(), "### no type description for property!" );

        // flags
        RTFieldAccess nFlags = aReader.getFieldFlags( nFields );

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

        pProps[ nFields ]
            = new PropertyTypeDescriptionImpl( aName.makeStringAndClear(),
                                               xTD,
                                               nAttribs );
    }

    MutexGuard guard(getMutex());
    if (_pProps.get() == 0) {
        _pProps = pTempProps;
    }
    return *_pProps;
}

sal_Bool ServiceTypeDescriptionImpl::isSingleInterfaceBased()
    throw (RuntimeException)
{
    getReferences();
    return _xInterfaceTD.is();
}

Reference< XTypeDescription > ServiceTypeDescriptionImpl::getInterface()
    throw (RuntimeException)
{
    getReferences();
    return _xInterfaceTD;
}

Sequence< Reference< XServiceConstructorDescription > >
ServiceTypeDescriptionImpl::getConstructors() throw (RuntimeException) {
    MutexGuard guard(getMutex());
    if (_pCtors.get() == 0) {
        typereg::Reader reader(
            _aBytes.getConstArray(), _aBytes.getLength(), false,
            TYPEREG_VERSION_1);
        sal_uInt16 ctorCount = reader.getMethodCount();
        std::auto_ptr< Sequence< Reference< XServiceConstructorDescription > > >
            ctors(
                new Sequence< Reference< XServiceConstructorDescription > >(
                    ctorCount));
        for (sal_uInt16 i = 0; i < ctorCount; ++i) {
            rtl::OUString name(reader.getMethodName(i));
            if (reader.getMethodFlags(i) != RT_MODE_TWOWAY
                || (!reader.getMethodReturnTypeName(i).equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM("void")))
                || (name.getLength() == 0
                    && (ctorCount != 1 || reader.getMethodParameterCount(i) != 0
                        || reader.getMethodExceptionCount(i) != 0)))
            {
                throw RuntimeException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "Service has bad constructors")),
                    static_cast< OWeakObject * >(this));
            }
            (*ctors)[i] = new Constructor(
                _xTDMgr, reader.getMethodName(i), _aBytes, i);
        }
        _pCtors = ctors;
    }
    return *_pCtors;
}

//__________________________________________________________________________________________________
void ServiceTypeDescriptionImpl::getReferences()
    throw ( RuntimeException )
{
    {
        MutexGuard guard(getMutex());
        if (_bInitReferences) {
            return;
        }
    }
    typereg::Reader aReader(
        _aBytes.getConstArray(), _aBytes.getLength(), false, TYPEREG_VERSION_1);
    sal_uInt16 superTypes = aReader.getSuperTypeCount();
    if (superTypes > 1) {
        throw RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "Service has more than one supertype")),
            static_cast< OWeakObject * >(this));
    }
    if (superTypes == 1) {
        OUString aBaseName( aReader.getSuperTypeName(0).replace( '/', '.' ) );
        if ( aReader.getReferenceCount() != 0
             || aReader.getFieldCount() != 0 )
            throw RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "Service is single-interface--based but also has"
                        " references and/or properties" ) ),
                static_cast< OWeakObject * >( this ) );
        Reference< XTypeDescription > ifc;
        try
        {
            _xTDMgr->getByHierarchicalName( aBaseName ) >>= ifc;
        }
        catch ( NoSuchElementException const & e )
        {
            throw RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.container.NoSuchElementException: " ) )
                + e.Message,
                static_cast< OWeakObject * >( this ) );
        }
        OSL_ASSERT(ifc.is());
        if (resolveTypedefs(ifc)->getTypeClass() != TypeClass_INTERFACE) {
            throw RuntimeException(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "Single-interface--based service is not based on"
                        " interface type" ) ),
                static_cast< OWeakObject * >( this ) );
        }
        MutexGuard guard(getMutex());
        if (!_bInitReferences) {
            _xInterfaceTD = ifc;
            _bInitReferences = true;
        }
    }
    else
    {
        sal_uInt16 nRefs = aReader.getReferenceCount();
        Sequence< Reference< XServiceTypeDescription > > aMandatoryServices(
            nRefs);
        Sequence< Reference< XServiceTypeDescription > > aOptionalServices(
            nRefs);
        Sequence< Reference< XInterfaceTypeDescription > > aMandatoryInterfaces(
            nRefs);
        Sequence< Reference< XInterfaceTypeDescription > > aOptionalInterfaces(
            nRefs);
        sal_uInt32 nMS = 0;
        sal_uInt32 nOS = 0;
        sal_uInt32 nMI = 0;
        sal_uInt32 nOI = 0;

        while ( nRefs-- )
        {
            RTReferenceType eType = aReader.getReferenceSort( nRefs );
            switch ( eType )
            {
            case RT_REF_EXPORTS: // service
                {
                    uno::Any aTypeDesc;
                    try
                    {
                        aTypeDesc = _xTDMgr->getByHierarchicalName(
                            aReader.getReferenceTypeName( nRefs ).replace(
                                '/', '.' ) );
                    }
                    catch ( NoSuchElementException const & e )
                    {
                        throw RuntimeException(
                            OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.container."
                                    "NoSuchElementException: " ) )
                            + e.Message,
                            static_cast< OWeakObject * >( this ) );
                    }

                    RTFieldAccess nAccess = aReader.getReferenceFlags( nRefs );
                    if ( nAccess & RT_ACCESS_OPTIONAL )
                    {
                        // optional service
                        if ( !( aTypeDesc >>= aOptionalServices[ nOS ] ) )
                            throw RuntimeException(
                                OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "Service 'export' is not a service" ) ),
                                static_cast< OWeakObject * >( this ) );
                        nOS++;
                    }
                    else
                    {
                        // mandatory service
                        if ( !( aTypeDesc >>= aMandatoryServices[ nMS ] ) )
                            throw RuntimeException(
                                OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "Service 'export' is not a service" ) ),
                                static_cast< OWeakObject * >( this ) );
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
                            aReader.getReferenceTypeName( nRefs ).replace(
                                '/', '.' ) );
                    }
                    catch ( NoSuchElementException const & e )
                    {
                        throw RuntimeException(
                            OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "com.sun.star.container."
                                    "NoSuchElementException: " ) )
                            + e.Message,
                            static_cast< OWeakObject * >( this ) );
                    }

                    RTFieldAccess nAccess = aReader.getReferenceFlags( nRefs );
                    if ( nAccess & RT_ACCESS_OPTIONAL )
                    {
                        // optional interface
                        if ( !( aTypeDesc >>= aOptionalInterfaces[ nOI ] ) )
                            throw RuntimeException(
                                OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "Service 'supports' is not an"
                                        " interface" ) ),
                                static_cast< OWeakObject * >( this ) );
                        nOI++;
                    }
                    else
                    {
                        // mandatory interface
                        if ( !( aTypeDesc >>= aMandatoryInterfaces[ nMI ] ) )
                            throw RuntimeException(
                                OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "Service 'supports' is not an"
                                        " interface" ) ),
                                static_cast< OWeakObject * >( this ) );
                        nMI++;
                    }
                    break;
                }
            case RT_REF_INVALID:
            case RT_REF_OBSERVES:
            case RT_REF_NEEDS:
            default:
                OSL_ENSURE( sal_False, "### unsupported reference type!" );
                break;
            }
        }
        aMandatoryServices.realloc( nMS );
        aOptionalServices.realloc( nOS );
        aMandatoryInterfaces.realloc( nMI );
        aOptionalInterfaces.realloc( nOI );

        MutexGuard guard(getMutex());
        if (!_bInitReferences) {
            _aMandatoryServices = aMandatoryServices;
            _aOptionalServices = aOptionalServices;
            _aMandatoryInterfaces = aMandatoryInterfaces;
            _aOptionalInterfaces = aOptionalInterfaces;
            _bInitReferences = true;
        }
    }
}


}

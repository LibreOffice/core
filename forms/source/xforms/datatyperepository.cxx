/*************************************************************************
 *
 *  $RCSfile: datatyperepository.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:50:35 $
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

#ifndef FORMS_SOURCE_XFORMS_DATATYPEREPOSITORY_HXX
#include "datatyperepository.hxx"
#endif
#ifndef FORMS_SOURCE_XFORMS_DATATYPES_HXX
#include "datatypes.hxx"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#ifndef _FRM_RESOURCE_HXX_
#include "frm_resource.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif

#include <functional>
#include <algorithm>

//........................................................................
namespace xforms
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::util::VetoException;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::container::ElementExistException;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::xsd::XDataType;
    using namespace frm;

    //====================================================================
    //= ODataTypeRepository
    //====================================================================
    DBG_NAME( ODataTypeRepository )
    //--------------------------------------------------------------------
    ODataTypeRepository::ODataTypeRepository( )
    {
        DBG_CTOR( ODataTypeRepository, NULL );

        // insert some basic types
        ::rtl::OUString sName( FRM_RES_STRING( RID_STR_DATATYPE_STRING ) );
        m_aRepository[ sName ] = new OStringType( sName, ::com::sun::star::xsd::DataTypeClass::STRING );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_URL );
        m_aRepository[ sName ] = new OStringType( sName, ::com::sun::star::xsd::DataTypeClass::anyURI );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_BOOLEAN );
        m_aRepository[ sName ] = new OBooleanType( sName );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DECIMAL );
        m_aRepository[ sName ] = new ODecimalType( sName, ::com::sun::star::xsd::DataTypeClass::DECIMAL );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_FLOAT );
        m_aRepository[ sName ] = new ODecimalType( sName, ::com::sun::star::xsd::DataTypeClass::FLOAT );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DOUBLE );
        m_aRepository[ sName ] = new ODecimalType( sName, ::com::sun::star::xsd::DataTypeClass::DOUBLE );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DATE );
        m_aRepository[ sName ] = new ODateType( sName );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_TIME );
        m_aRepository[ sName ] = new OTimeType( sName );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DATETIME );
        m_aRepository[ sName ] = new ODateTimeType( sName );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_YEAR );
        m_aRepository[ sName ] = new OShortIntegerType( sName, ::com::sun::star::xsd::DataTypeClass::gYear );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_MONTH );
        m_aRepository[ sName ] = new OByteIntegerType( sName, ::com::sun::star::xsd::DataTypeClass::gMonth );

        sName = FRM_RES_STRING( RID_STR_DATATYPE_DAY );
        m_aRepository[ sName ] = new OByteIntegerType( sName, ::com::sun::star::xsd::DataTypeClass::gDay );
    }

    //--------------------------------------------------------------------
    ODataTypeRepository::~ODataTypeRepository( )
    {
        DBG_DTOR( ODataTypeRepository, NULL );
    }

    //--------------------------------------------------------------------
    ODataTypeRepository::Repository::iterator ODataTypeRepository::implLocate( const ::rtl::OUString& _rName, bool _bAllowMiss ) SAL_THROW( ( NoSuchElementException ) )
    {
        Repository::iterator aTypePos = m_aRepository.find( _rName );
        if ( aTypePos == m_aRepository.end() && !_bAllowMiss )
            throw NoSuchElementException( ::rtl::OUString(), *this );

        return aTypePos;
    }

    //--------------------------------------------------------------------
    Reference< XDataType > SAL_CALL ODataTypeRepository::getBasicDataType( sal_Int16 dataTypeClass ) throw (NoSuchElementException, RuntimeException)
    {
        Reference< XDataType > xReturn;

        for ( Repository::const_iterator lookup = m_aRepository.begin();
              ( lookup != m_aRepository.end() ) && ! xReturn.is();
              ++lookup
            )
        {
            if ( lookup->second->getIsBasic() && ( lookup->second->getTypeClass() == dataTypeClass ) )
                xReturn = lookup->second.get();
        }

        if ( !xReturn.is() )
            throw NoSuchElementException( ::rtl::OUString(), *this );

        return xReturn;
    }

    //--------------------------------------------------------------------
    Reference< XDataType > SAL_CALL ODataTypeRepository::cloneDataType( const ::rtl::OUString& sourceName, const ::rtl::OUString& newName ) throw (NoSuchElementException, ElementExistException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Repository::iterator aTypePos = implLocate( newName, true );
        if ( aTypePos != m_aRepository.end() )
            throw ElementExistException( ::rtl::OUString(), *this );

        aTypePos = implLocate( sourceName );
        OXSDDataType* pClone = aTypePos->second->clone( newName );
        m_aRepository[ newName ] = pClone;

        return pClone;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ODataTypeRepository::revokeDataType( const ::rtl::OUString& typeName ) throw (NoSuchElementException, VetoException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Repository::iterator aTypePos = implLocate( typeName );
        if ( aTypePos->second->getIsBasic() )
            throw VetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "This is a built-in type and cannot be removed." ) ), *this );
            // TODO: localize this error message

        m_aRepository.erase( aTypePos );
    }

    //--------------------------------------------------------------------
    Reference< XDataType > SAL_CALL ODataTypeRepository::getDataType( const ::rtl::OUString& typeName ) throw (NoSuchElementException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return implLocate( typeName, false )->second.get();
    }


    //--------------------------------------------------------------------
    Reference< XEnumeration > SAL_CALL ODataTypeRepository::createEnumeration(  ) throw (RuntimeException)
    {
        return new ::comphelper::OEnumerationByName( this );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL ODataTypeRepository::getByName( const ::rtl::OUString& aName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        return makeAny( getDataType( aName ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ODataTypeRepository::getElementNames(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Sequence< ::rtl::OUString > aNames( m_aRepository.size() );
        ::std::transform(
            m_aRepository.begin(),
            m_aRepository.end(),
            aNames.getArray(),
            ::std::select1st< Repository::value_type >()
        );
        return aNames;
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL ODataTypeRepository::hasByName( const ::rtl::OUString& aName ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_aRepository.find( aName ) != m_aRepository.end();
    }

    //--------------------------------------------------------------------
    Type SAL_CALL ODataTypeRepository::getElementType(  ) throw (RuntimeException)
    {
        return ::getCppuType( static_cast< Reference< XDataType >* >( NULL ) );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL ODataTypeRepository::hasElements(  ) throw (RuntimeException)
    {
        return !m_aRepository.empty();
    }

//........................................................................
} // namespace xforms
//........................................................................


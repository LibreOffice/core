/*************************************************************************
 *
 *  $RCSfile: datatypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:51:24 $
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

#ifndef FORMS_SOURCE_XFORMS_DATATYPES_HXX
#define FORMS_SOURCE_XFORMS_DATATYPES_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_XSD_XDATATYPE_HPP_
#include <com/sun/star/xsd/XDataType.hpp>
#endif
#ifndef _COM_SUN_STAR_XSD_DATATYPECLASS_HPP_
#include <com/sun/star/xsd/DataTypeClass.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

#ifndef REGEX_H
#include <unicode/regex.h>
#endif

#include <memory>

//........................................................................
namespace xforms
{
//........................................................................

    //====================================================================
    //= OXSDDataType
    //====================================================================
    typedef ::cppu::WeakImplHelper1             <   ::com::sun::star::xsd::XDataType
                                                >   OXSDDataType_Base;
    typedef ::comphelper::OMutexAndBroadcastHelper  OXSDDataType_BBase;
    typedef ::comphelper::OPropertyContainer        OXSDDataType_PBase;

    class OXSDDataType  :public OXSDDataType_Base
                        ,public OXSDDataType_BBase         // order matters: OMutexAndBroadcastHelper before
                        ,public OXSDDataType_PBase         // OPropertyContainer
    {
    private:
        // <properties>
        sal_Bool        m_bIsBasic;
        sal_Int16       m_nTypeClass;
        ::rtl::OUString m_sName;
        ::rtl::OUString m_sPattern;
        sal_uInt16      m_nWST;
        // </properties>

        ::std::auto_ptr< U_NAMESPACE_QUALIFIER RegexMatcher >
                        m_pPatternMatcher;
        bool            m_bPatternMatcherDirty;

    protected:

        sal_Bool    isBasic() const         { return m_bIsBasic; }
        sal_Int16   getTypeClass() const    { return m_nTypeClass; }
        const ::rtl::OUString&
                    getName() const         { return m_sName; }

    private:
        OXSDDataType( );                                    // never implemented
        OXSDDataType( const OXSDDataType& );                // never implemented
        OXSDDataType& operator=( const OXSDDataType& );     // never implemented

    protected:
        // create basic data type
        OXSDDataType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass );
        ~OXSDDataType();

    public:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        virtual ::rtl::OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::util::VetoException);
        virtual ::rtl::OUString SAL_CALL getPattern() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPattern( const ::rtl::OUString& _pattern ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getWhiteSpaceTreatment() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setWhiteSpaceTreatment( sal_Int16 _whitespacetreatment ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);
        virtual sal_Bool SAL_CALL getIsBasic() throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getTypeClass() throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL validate( const ::rtl::OUString& value ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL explainInvalid( const ::rtl::OUString& value ) throw (::com::sun::star::uno::RuntimeException);

        // XPropertySet - is a base of XDataType and needs to be disambiguated
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    public:
        OXSDDataType* clone( const ::rtl::OUString& _rNewName ) const;

    protected:
        // XPropertySet and friends
        virtual sal_Bool SAL_CALL   convertFastPropertyValue( ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw(::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL       setFastPropertyValue_NoBroadcast(
                                        sal_Int32 nHandle,
                                        const ::com::sun::star::uno::Any& rValue
                                    )
                                    throw (::com::sun::star::uno::Exception);

        // --- own overridables ---
        // helper for implementing cloning of data types
        virtual OXSDDataType*   createClone( const ::rtl::OUString& _rName ) const = 0;
        virtual void            initializeClone( const OXSDDataType& _rCloneSource );

        // helper method for validate and explainInvalid
        virtual sal_uInt16  _validate( const ::rtl::OUString& value );
        virtual ::rtl::OUString _explainInvalid( sal_uInt16 nReason );

        // helper method for checking properties values which are to be set
        virtual bool        checkPropertySanity( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rNewValue, ::rtl::OUString& _rErrorMessage );

        // register properties implemented by this instance - call the base class when overriding
        virtual void        registerProperties();
    };

    //====================================================================
    //= helper for deriving from OXSDDataType
    //====================================================================
#define DECLARE_DEFAULT_CLONING( classname )        \
    virtual OXSDDataType* createClone( const ::rtl::OUString& _rName ) const;    \
    virtual void       initializeClone( const OXSDDataType& _rCloneSource ); \
            void       initializeTypedClone( const classname& _rCloneSource );

#define IMPLEMENT_DEFAULT_CLONING( classname, baseclass )   \
    OXSDDataType* classname::createClone( const ::rtl::OUString& _rName ) const              \
    {                                                       \
        return new classname( _rName );                     \
    }                                                       \
    void classname::initializeClone( const OXSDDataType& _rCloneSource ) \
    { \
        baseclass::initializeClone( _rCloneSource );        \
        initializeTypedClone( static_cast< const classname& >( _rCloneSource ) ); \
    } \

#define IMPLEMENT_DEFAULT_TYPED_CLONING( classname, baseclass )   \
    OXSDDataType* classname::createClone( const ::rtl::OUString& _rName ) const              \
    {                                                       \
        return new classname( _rName, getTypeClass() );     \
    }                                                       \
    void classname::initializeClone( const OXSDDataType& _rCloneSource ) \
    { \
        baseclass::initializeClone( _rCloneSource );        \
        initializeTypedClone( static_cast< const classname& >( _rCloneSource ) ); \
    } \

#define REGISTER_VOID_PROP( prop, memberAny, type ) \
    registerMayBeVoidProperty( PROPERTY_##prop, PROPERTY_ID_##prop, ::com::sun::star::beans::PropertyAttribute::BOUND | ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, \
        &memberAny, ::getCppuType( static_cast< type* >( NULL ) ) );

    //====================================================================
    //= OComparableType
    //====================================================================
    template < typename VALUE_TYPE >
    class OComparableType : public OXSDDataType
    {
    protected:
        typedef     VALUE_TYPE  ValueType;

    protected:
        ::com::sun::star::uno::Any m_aMaxInclusive;
        ::com::sun::star::uno::Any m_aMaxExclusive;
        ::com::sun::star::uno::Any m_aMinInclusive;
        ::com::sun::star::uno::Any m_aMinExclusive;

    protected:
        OComparableType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass );

        virtual void       initializeClone( const OXSDDataType& _rCloneSource );
                void       initializeTypedClone( const OComparableType& _rCloneSource );

        // OXSDDataType overridables
        virtual void            registerProperties();
        virtual bool            _getValue( const ::rtl::OUString& value, double& fValue );
        virtual sal_uInt16      _validate( const ::rtl::OUString& value );
        virtual ::rtl::OUString _explainInvalid( sal_uInt16 nReason );

        // own overridables
        /** translate a given value into a human-readable string

            The value is guaranteed to be not <NULL/>, and is of type <member>ValueType</member>
        */
        virtual ::rtl::OUString typedValueAsString( const ::com::sun::star::uno::Any& _rValue ) const = 0;
    };

    //====================================================================
    //= ODerivedDataType
    //====================================================================
    /** helper class for implementing interfaces derived from XDataType
    */
    template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS = OXSDDataType >
    class ODerivedDataType  :public SUPERCLASS
                            ,::comphelper::OPropertyArrayUsageHelper< CONCRETE_DATA_TYPE_IMPL >
    {
    private:
        bool    m_bPropertiesRegistered;

    protected:
        ODerivedDataType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass );

    protected:
        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // XPropertySet
        virtual com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    };

    //====================================================================
    //= OBooleanType
    //====================================================================
    class OBooleanType;
    typedef ODerivedDataType< OBooleanType > OBooleanType_Base;
    class OBooleanType : public OBooleanType_Base
    {
    public:
        OBooleanType( const ::rtl::OUString& _rName );

    protected:
        DECLARE_DEFAULT_CLONING( OBooleanType )

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const ::rtl::OUString& value );
        virtual ::rtl::OUString _explainInvalid( sal_uInt16 nReason );
    };

    //====================================================================
    //= OStringType
    //====================================================================
    class OStringType;
    typedef ODerivedDataType< OStringType > OStringType_Base;
    class OStringType   :public OStringType_Base
    {
    protected:
        // <properties>
        ::com::sun::star::uno::Any m_aLength;
        ::com::sun::star::uno::Any m_aMinLength;
        ::com::sun::star::uno::Any m_aMaxLength;
        // </properties>

    public:
        OStringType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass /* = ::com::sun::star::xsd::DataTypeClass::STRING */ );

    protected:
        DECLARE_DEFAULT_CLONING( OStringType )

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const ::rtl::OUString& value );
        virtual ::rtl::OUString _explainInvalid( sal_uInt16 nReason );
        virtual bool            checkPropertySanity( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rNewValue, ::rtl::OUString& _rErrorMessage );
        virtual void            registerProperties();
    };

    //====================================================================
    //= ODecimalType
    //====================================================================
    class ODecimalType;
    typedef ODerivedDataType< ODecimalType, OComparableType< double > > ODecimalType_Base;
    class ODecimalType : public ODecimalType_Base
    {
    protected:
        ::com::sun::star::uno::Any m_aTotalDigits;
        ::com::sun::star::uno::Any m_aFractionDigits;

    public:
        ODecimalType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass /* = ::com::sun::star::xsd::DataTypeClass::DECIMAL */ );

    protected:
        DECLARE_DEFAULT_CLONING( ODecimalType )

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const ::rtl::OUString& value );
        virtual ::rtl::OUString _explainInvalid( sal_uInt16 nReason );
        virtual void            registerProperties();

        // OComparableType overridables
        virtual ::rtl::OUString typedValueAsString( const ::com::sun::star::uno::Any& _rValue ) const;
    };

    //====================================================================
    //=
    //====================================================================
#define DEFAULT_DECLARE_SUBTYPE( classname, valuetype )         \
    class classname;                                            \
    typedef ODerivedDataType< classname, OComparableType< valuetype > > classname##_Base;  \
    class classname : public classname##_Base                   \
    {                                                           \
    public:                                                     \
        classname( const ::rtl::OUString& _rName );             \
                                                                \
    protected:                                                  \
        DECLARE_DEFAULT_CLONING( classname )                    \
                                                                \
        /* OXSDDataType overridables */                         \
        virtual sal_uInt16          _validate( const ::rtl::OUString& value );  \
        virtual bool                _getValue( const ::rtl::OUString& value, double& fValue );  \
                                                                \
        /* OComparableType overridables */                      \
        virtual ::rtl::OUString     typedValueAsString( const ::com::sun::star::uno::Any& _rValue ) const;  \
    };

    //====================================================================
    //= ODateType
    //====================================================================
    DEFAULT_DECLARE_SUBTYPE( ODateType, ::com::sun::star::util::Date )

    //====================================================================
    //= OTimeType
    //====================================================================
    DEFAULT_DECLARE_SUBTYPE( OTimeType, ::com::sun::star::util::Time )

    //====================================================================
    //= ODateTimeType
    //====================================================================
    DEFAULT_DECLARE_SUBTYPE( ODateTimeType, ::com::sun::star::util::DateTime )

    //====================================================================
    //= OShortIntegerType
    //====================================================================
    class OShortIntegerType;
    typedef ODerivedDataType< OShortIntegerType, OComparableType< sal_Int16 > > OShortIntegerType_Base;
    class OShortIntegerType : public OShortIntegerType_Base
    {
    public:
        OShortIntegerType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass );

    protected:
        DECLARE_DEFAULT_CLONING( OShortIntegerType )

        // OXSDDataType overridables
        virtual bool            _getValue( const ::rtl::OUString& value, double& fValue );

        // OComparableType overridables
        virtual ::rtl::OUString typedValueAsString( const ::com::sun::star::uno::Any& _rValue ) const;
    };

    //====================================================================
    //= OByteIntegerType
    //====================================================================
    class OByteIntegerType;
    typedef ODerivedDataType< OByteIntegerType, OComparableType< sal_Int8 > > OByteIntegerType_Base;
    class OByteIntegerType : public OByteIntegerType_Base
    {
    public:
        OByteIntegerType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass );

    protected:
        DECLARE_DEFAULT_CLONING( OByteIntegerType )

        // OXSDDataType overridables
        virtual bool            _getValue( const ::rtl::OUString& value, double& fValue );

        // OComparableType overridables
        virtual ::rtl::OUString typedValueAsString( const ::com::sun::star::uno::Any& _rValue ) const;
    };

//........................................................................
} // namespace xforms
//........................................................................

#endif // FORMS_SOURCE_XFORMS_DATATYPES_HXX


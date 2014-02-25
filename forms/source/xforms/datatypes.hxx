/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_DATATYPES_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_DATATYPES_HXX

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xsd/XDataType.hpp>
#include <com/sun/star/xsd/DataTypeClass.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <unicode/regex.h>

#include <memory>


namespace xforms
{



    //= OXSDDataType

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
        OUString m_sName;
        OUString m_sPattern;
        sal_uInt16      m_nWST;
        // </properties>

        ::std::auto_ptr< U_NAMESPACE_QUALIFIER RegexMatcher >
                        m_pPatternMatcher;
        bool            m_bPatternMatcherDirty;

    protected:

        sal_Bool    isBasic() const         { return m_bIsBasic; }
        sal_Int16   getTypeClass() const    { return m_nTypeClass; }
        const OUString&
                    getName() const         { return m_sName; }

    private:
        OXSDDataType( );                                    // never implemented
        OXSDDataType( const OXSDDataType& );                // never implemented
        OXSDDataType& operator=( const OXSDDataType& );     // never implemented

    protected:
        // create basic data type
        OXSDDataType( const OUString& _rName, sal_Int16 _nTypeClass );
        ~OXSDDataType();

    public:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        virtual OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setName( const OUString& aName ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::util::VetoException, std::exception);
        virtual OUString SAL_CALL getPattern() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setPattern( const OUString& _pattern ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int16 SAL_CALL getWhiteSpaceTreatment() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setWhiteSpaceTreatment( sal_Int16 _whitespacetreatment ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception);
        virtual sal_Bool SAL_CALL getIsBasic() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int16 SAL_CALL getTypeClass() throw (::com::sun::star::uno::RuntimeException, std::exception);

        virtual sal_Bool SAL_CALL validate( const OUString& value ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual OUString SAL_CALL explainInvalid( const OUString& value ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XPropertySet - is a base of XDataType and needs to be disambiguated
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    public:
        OXSDDataType* clone( const OUString& _rNewName ) const;

    protected:
        // XPropertySet and friends
        virtual sal_Bool SAL_CALL   convertFastPropertyValue( ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw(::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL       setFastPropertyValue_NoBroadcast(
                                        sal_Int32 nHandle,
                                        const ::com::sun::star::uno::Any& rValue
                                    )
                                    throw (::com::sun::star::uno::Exception, std::exception);

        // --- own overridables ---
        // helper for implementing cloning of data types
        virtual OXSDDataType*   createClone( const OUString& _rName ) const = 0;
        virtual void            initializeClone( const OXSDDataType& _rCloneSource );

        // helper method for validate and explainInvalid
        virtual sal_uInt16  _validate( const OUString& value );
        virtual OUString _explainInvalid( sal_uInt16 nReason );

        // helper method for checking properties values which are to be set
        virtual bool        checkPropertySanity( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rNewValue, OUString& _rErrorMessage );

        // register properties implemented by this instance - call the base class when overriding
        virtual void        registerProperties();
    };


    //= helper for deriving from OXSDDataType

#define DECLARE_DEFAULT_CLONING( classname )        \
    virtual OXSDDataType* createClone( const OUString& _rName ) const;    \
    virtual void       initializeClone( const OXSDDataType& _rCloneSource ); \
            void       initializeTypedClone( const classname& _rCloneSource );

#define IMPLEMENT_DEFAULT_CLONING( classname, baseclass )   \
    OXSDDataType* classname::createClone( const OUString& _rName ) const              \
    {                                                       \
        return new classname( _rName );                     \
    }                                                       \
    void classname::initializeClone( const OXSDDataType& _rCloneSource ) \
    { \
        baseclass::initializeClone( _rCloneSource );        \
        initializeTypedClone( static_cast< const classname& >( _rCloneSource ) ); \
    } \

#define IMPLEMENT_DEFAULT_TYPED_CLONING( classname, baseclass )   \
    OXSDDataType* classname::createClone( const OUString& _rName ) const              \
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


    //= OValueLimitedType_Base

    class OValueLimitedType_Base : public OXSDDataType
    {
    protected:
        ::com::sun::star::uno::Any m_aMaxInclusive;
        ::com::sun::star::uno::Any m_aMaxExclusive;
        ::com::sun::star::uno::Any m_aMinInclusive;
        ::com::sun::star::uno::Any m_aMinExclusive;

        double  m_fCachedMaxInclusive;
        double  m_fCachedMaxExclusive;
        double  m_fCachedMinInclusive;
        double  m_fCachedMinExclusive;

    protected:
        OValueLimitedType_Base( const OUString& _rName, sal_Int16 _nTypeClass );

        virtual void       initializeClone( const OXSDDataType& _rCloneSource );
                void       initializeTypedClone( const OValueLimitedType_Base& _rCloneSource );

        // XPropertySet and friends
        virtual void SAL_CALL       setFastPropertyValue_NoBroadcast(
                                        sal_Int32 nHandle,
                                        const ::com::sun::star::uno::Any& rValue
                                    )
                                    throw (::com::sun::star::uno::Exception, std::exception);

        // OXSDDataType overridables
        virtual bool            _getValue( const OUString& value, double& fValue );
        virtual sal_uInt16      _validate( const OUString& value );
        virtual OUString _explainInvalid( sal_uInt16 nReason );

        // own overridables
        /** translate a given value into a human-readable string

            The value is guaranteed to be not <NULL/>, and is of type <member>ValueType</member>
        */
        virtual OUString typedValueAsHumanReadableString( const ::com::sun::star::uno::Any& _rValue ) const = 0;

        /** translates a <member>ValueType</member> value into a double value

            The normalization must respect the "<" and "==" relations on the value
            space. That is, if two values are equal, their normalizations must be equal, too.
            Similarily, if <code>foo</code> is less than <code>bar</code>, the same
            must hold for their normalizations.

            @param _rValue
                the value to translate. Guranteed to be not <NULL/>, and of type <member>ValueType</member>
            @param _rDoubleValue
                output parameter to hold the resulting double value
        */
        virtual void normalizeValue( const ::com::sun::star::uno::Any& _rValue, double& _rDoubleValue ) const = 0;
    };


    //= OValueLimitedType

    template < typename VALUE_TYPE >
    class OValueLimitedType : public OValueLimitedType_Base
    {
    protected:
        typedef     VALUE_TYPE  ValueType;
        inline const ::com::sun::star::uno::Type&
            getCppuType() const { return ::getCppuType( static_cast< ValueType* >( NULL ) ); }

    protected:
        OValueLimitedType( const OUString& _rName, sal_Int16 _nTypeClass );

        // OXSDDataType overridables
        virtual void            registerProperties();
    };


    //= ODerivedDataType

    /** helper class for implementing interfaces derived from XDataType
    */
    template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS = OXSDDataType >
    class ODerivedDataType  :public SUPERCLASS
                            ,::comphelper::OPropertyArrayUsageHelper< CONCRETE_DATA_TYPE_IMPL >
    {
    private:
        bool    m_bPropertiesRegistered;

    protected:
        ODerivedDataType( const OUString& _rName, sal_Int16 _nTypeClass );

    protected:
        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // XPropertySet
        virtual com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    };


    //= OBooleanType

    class OBooleanType;
    typedef ODerivedDataType< OBooleanType > OBooleanType_Base;
    class OBooleanType : public OBooleanType_Base
    {
    public:
        OBooleanType( const OUString& _rName );

    protected:
        DECLARE_DEFAULT_CLONING( OBooleanType )

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const OUString& value );
        virtual OUString _explainInvalid( sal_uInt16 nReason );
    };


    //= OStringType

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
        OStringType( const OUString& _rName, sal_Int16 _nTypeClass /* = ::com::sun::star::xsd::DataTypeClass::STRING */ );

    protected:
        DECLARE_DEFAULT_CLONING( OStringType )

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const OUString& value );
        virtual OUString _explainInvalid( sal_uInt16 nReason );
        virtual bool            checkPropertySanity( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rNewValue, OUString& _rErrorMessage );
        virtual void            registerProperties();
    };


    //= ODecimalType

    class ODecimalType;
    typedef ODerivedDataType< ODecimalType, OValueLimitedType< double > > ODecimalType_Base;
    class ODecimalType : public ODecimalType_Base
    {
    protected:
        ::com::sun::star::uno::Any m_aTotalDigits;
        ::com::sun::star::uno::Any m_aFractionDigits;

    public:
        ODecimalType( const OUString& _rName, sal_Int16 _nTypeClass /* = ::com::sun::star::xsd::DataTypeClass::DECIMAL */ );

    protected:
        DECLARE_DEFAULT_CLONING( ODecimalType )

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const OUString& value );
        virtual OUString _explainInvalid( sal_uInt16 nReason );
        virtual void            registerProperties();

        // OValueLimitedType overridables
        virtual OUString typedValueAsHumanReadableString( const ::com::sun::star::uno::Any& _rValue ) const;
        virtual void normalizeValue( const ::com::sun::star::uno::Any& _rValue, double& _rDoubleValue ) const;

    private:
        using ODecimalType_Base::initializeTypedClone;
    };


    //=

#define DEFAULT_DECLARE_SUBTYPE( classname, valuetype )         \
    class classname;                                            \
    typedef ODerivedDataType< classname, OValueLimitedType< valuetype > > classname##_Base;  \
    class classname : public classname##_Base                   \
    {                                                           \
    public:                                                     \
        classname( const OUString& _rName );             \
                                                                \
    protected:                                                  \
        DECLARE_DEFAULT_CLONING( classname )                    \
                                                                \
        /* OXSDDataType overridables */                         \
        virtual sal_uInt16          _validate( const OUString& value );  \
        virtual bool                _getValue( const OUString& value, double& fValue );  \
                                                                \
        /* OValueLimitedType overridables */                    \
        virtual OUString     typedValueAsHumanReadableString( const ::com::sun::star::uno::Any& _rValue ) const;  \
        virtual void normalizeValue( const ::com::sun::star::uno::Any& _rValue, double& _rDoubleValue ) const; \
                                                                \
    private:                                                    \
        using classname##_Base::initializeTypedClone;          \
    };


    //= ODateType

    DEFAULT_DECLARE_SUBTYPE( ODateType, ::com::sun::star::util::Date )


    //= OTimeType

    DEFAULT_DECLARE_SUBTYPE( OTimeType, ::com::sun::star::util::Time )


    //= ODateTimeType

    DEFAULT_DECLARE_SUBTYPE( ODateTimeType, ::com::sun::star::util::DateTime )


    //= OShortIntegerType

    class OShortIntegerType;
    typedef ODerivedDataType< OShortIntegerType, OValueLimitedType< sal_Int16 > > OShortIntegerType_Base;
    class OShortIntegerType : public OShortIntegerType_Base
    {
    public:
        OShortIntegerType( const OUString& _rName, sal_Int16 _nTypeClass );

    protected:
        DECLARE_DEFAULT_CLONING( OShortIntegerType )

        // OXSDDataType overridables
        virtual bool            _getValue( const OUString& value, double& fValue );

        // OValueLimitedType overridables
        virtual OUString typedValueAsHumanReadableString( const ::com::sun::star::uno::Any& _rValue ) const;
        virtual void normalizeValue( const ::com::sun::star::uno::Any& _rValue, double& _rDoubleValue ) const;

    private:
        using OShortIntegerType_Base::initializeTypedClone;
    };


} // namespace xforms


#endif // INCLUDED_FORMS_SOURCE_XFORMS_DATATYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

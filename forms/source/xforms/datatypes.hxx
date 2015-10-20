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
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <unicode/regex.h>

#include <memory>


namespace xforms
{



    //= OXSDDataType

    typedef ::cppu::WeakImplHelper             <   css::xsd::XDataType
                                                >   OXSDDataType_Base;
    typedef ::comphelper::OMutexAndBroadcastHelper  OXSDDataType_BBase;
    typedef ::comphelper::OPropertyContainer        OXSDDataType_PBase;

    class OXSDDataType  :public OXSDDataType_Base
                        ,public OXSDDataType_BBase         // order matters: OMutexAndBroadcastHelper before
                        ,public OXSDDataType_PBase         // OPropertyContainer
    {
    private:
        // <properties>
        bool        m_bIsBasic;
        sal_Int16       m_nTypeClass;
        OUString m_sName;
        OUString m_sPattern;
        sal_uInt16      m_nWST;
        // </properties>

        ::std::unique_ptr< U_NAMESPACE_QUALIFIER RegexMatcher >
                        m_pPatternMatcher;
        bool            m_bPatternMatcherDirty;

    protected:
        sal_Int16   getTypeClass() const    { return m_nTypeClass; }

    private:
        OXSDDataType( const OXSDDataType& ) = delete;
        OXSDDataType& operator=( const OXSDDataType& ) = delete;

    protected:
        // create basic data type
        OXSDDataType( const OUString& _rName, sal_Int16 _nTypeClass );
        virtual ~OXSDDataType();

    public:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        virtual OUString SAL_CALL getName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setName( const OUString& aName ) throw (css::uno::RuntimeException, css::util::VetoException, std::exception) override;
        virtual OUString SAL_CALL getPattern() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPattern( const OUString& _pattern ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getWhiteSpaceTreatment() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setWhiteSpaceTreatment( sal_Int16 _whitespacetreatment ) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;
        virtual sal_Bool SAL_CALL getIsBasic() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getTypeClass() throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL validate( const OUString& value ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL explainInvalid( const OUString& value ) throw (css::uno::RuntimeException, std::exception) override;

        // XPropertySet - is a base of XDataType and needs to be disambiguated
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    public:
        OXSDDataType* clone( const OUString& _rNewName ) const;

    protected:
        // XPropertySet and friends
        virtual sal_Bool SAL_CALL   convertFastPropertyValue( css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue ) throw(css::lang::IllegalArgumentException) override;
        virtual void SAL_CALL       setFastPropertyValue_NoBroadcast(
                                        sal_Int32 nHandle,
                                        const css::uno::Any& rValue
                                    )
                                    throw (css::uno::Exception, std::exception) override;

        // --- own overridables ---
        // helper for implementing cloning of data types
        virtual OXSDDataType*   createClone( const OUString& _rName ) const = 0;
        virtual void            initializeClone( const OXSDDataType& _rCloneSource );

        // helper method for validate and explainInvalid
        virtual sal_uInt16  _validate( const OUString& value );
        virtual OUString _explainInvalid( sal_uInt16 nReason );

        // helper method for checking properties values which are to be set
        virtual bool        checkPropertySanity( sal_Int32 _nHandle, const css::uno::Any& _rNewValue, OUString& _rErrorMessage );

        // register properties implemented by this instance - call the base class when overriding
        virtual void        registerProperties();
    };


    //= helper for deriving from OXSDDataType

#define DECLARE_DEFAULT_CLONING( classname )        \
    virtual OXSDDataType* createClone( const OUString& _rName ) const override;    \
    virtual void       initializeClone( const OXSDDataType& _rCloneSource ) override;

#define IMPLEMENT_DEFAULT_TYPED_CLONING( classname, baseclass )   \
    OXSDDataType* classname::createClone( const OUString& _rName ) const              \
    {                                                       \
        return new classname( _rName, getTypeClass() );     \
    }                                                       \
    void classname::initializeClone( const OXSDDataType& _rCloneSource ) \
    { \
        baseclass::initializeClone( _rCloneSource );        \
        initializeTypedClone( static_cast< const classname& >( _rCloneSource ) ); \
    }

#define REGISTER_VOID_PROP( prop, memberAny, type ) \
    registerMayBeVoidProperty( PROPERTY_##prop, PROPERTY_ID_##prop, css::beans::PropertyAttribute::BOUND | css::beans::PropertyAttribute::MAYBEVOID, \
        &memberAny, cppu::UnoType<type>::get() );

    class OValueLimitedType_Base : public OXSDDataType
    {
    protected:
        css::uno::Any m_aMaxInclusive;
        css::uno::Any m_aMaxExclusive;
        css::uno::Any m_aMinInclusive;
        css::uno::Any m_aMinExclusive;

        double  m_fCachedMaxInclusive;
        double  m_fCachedMaxExclusive;
        double  m_fCachedMinInclusive;
        double  m_fCachedMinExclusive;

    protected:
        OValueLimitedType_Base( const OUString& _rName, sal_Int16 _nTypeClass );

        virtual void       initializeClone( const OXSDDataType& _rCloneSource ) override;
                void       initializeTypedClone( const OValueLimitedType_Base& _rCloneSource );

        // XPropertySet and friends
        virtual void SAL_CALL       setFastPropertyValue_NoBroadcast(
                                        sal_Int32 nHandle,
                                        const css::uno::Any& rValue
                                    )
                                    throw (css::uno::Exception, std::exception) override;

        // OXSDDataType overridables
        virtual bool            _getValue( const OUString& value, double& fValue );
        virtual sal_uInt16      _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( sal_uInt16 nReason ) override;

        // own overridables
        /** translate a given value into a human-readable string

            The value is guaranteed to be not <NULL/>, and is of type <member>ValueType</member>
        */
        virtual OUString typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const = 0;

        /** translates a <member>ValueType</member> value into a double value

            The normalization must respect the "<" and "==" relations on the value
            space. That is, if two values are equal, their normalizations must be equal, too.
            Similarly, if <code>foo</code> is less than <code>bar</code>, the same
            must hold for their normalizations.

            @param _rValue
                the value to translate. Guaranteed to be not <NULL/>, and of type <member>ValueType</member>
            @param _rDoubleValue
                output parameter to hold the resulting double value
        */
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const = 0;
    };

    template < typename VALUE_TYPE >
    class OValueLimitedType : public OValueLimitedType_Base
    {
    protected:
        typedef     VALUE_TYPE  ValueType;
        inline const css::uno::Type&
            getCppuType() const { return cppu::UnoType<ValueType>::get(); }

    protected:
        OValueLimitedType( const OUString& _rName, sal_Int16 _nTypeClass );

        // OXSDDataType overridables
        virtual void            registerProperties() override;
    };

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
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;

        // XPropertySet
        virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException) SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
    };

    class OBooleanType;
    typedef ODerivedDataType< OBooleanType > OBooleanType_Base;
    class OBooleanType : public OBooleanType_Base
    {
    public:
        explicit OBooleanType( const OUString& _rName );

    protected:
        DECLARE_DEFAULT_CLONING( OBooleanType )

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( sal_uInt16 nReason ) override;
    };

    class OStringType;
    typedef ODerivedDataType< OStringType > OStringType_Base;
    class OStringType   :public OStringType_Base
    {
    protected:
        // <properties>
        css::uno::Any m_aLength;
        css::uno::Any m_aMinLength;
        css::uno::Any m_aMaxLength;
        // </properties>

    public:
        OStringType( const OUString& _rName, sal_Int16 _nTypeClass /* = css::xsd::DataTypeClass::STRING */ );

    protected:
        DECLARE_DEFAULT_CLONING( OStringType )
        void       initializeTypedClone( const OStringType& _rCloneSource );

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( sal_uInt16 nReason ) override;
        virtual bool            checkPropertySanity( sal_Int32 _nHandle, const css::uno::Any& _rNewValue, OUString& _rErrorMessage ) override;
        virtual void            registerProperties() override;
    };

    class ODecimalType;
    typedef ODerivedDataType< ODecimalType, OValueLimitedType< double > > ODecimalType_Base;
    class ODecimalType : public ODecimalType_Base
    {
    protected:
        css::uno::Any m_aTotalDigits;
        css::uno::Any m_aFractionDigits;

    public:
        ODecimalType( const OUString& _rName, sal_Int16 _nTypeClass /* = css::xsd::DataTypeClass::DECIMAL */ );

    protected:
        DECLARE_DEFAULT_CLONING( ODecimalType )
        void       initializeTypedClone( const ODecimalType& _rCloneSource );

        // OXSDDataType overridables
        virtual sal_uInt16      _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( sal_uInt16 nReason ) override;
        virtual void            registerProperties() override;

        // OValueLimitedType overridables
        virtual OUString typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const override;
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const override;

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
        explicit classname( const OUString& _rName );           \
                                                                \
    protected:                                                  \
        DECLARE_DEFAULT_CLONING( classname )                    \
                                                                \
        /* OXSDDataType overridables */                         \
        virtual sal_uInt16          _validate( const OUString& value ) override;  \
        virtual bool                _getValue( const OUString& value, double& fValue ) override;  \
                                                                \
        /* OValueLimitedType overridables */                    \
        virtual OUString     typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const override;  \
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const override; \
                                                                \
    private:                                                    \
        using classname##_Base::initializeTypedClone;          \
    };


    //= ODateType

    DEFAULT_DECLARE_SUBTYPE( ODateType, css::util::Date )


    //= OTimeType

    DEFAULT_DECLARE_SUBTYPE( OTimeType, css::util::Time )


    //= ODateTimeType

    DEFAULT_DECLARE_SUBTYPE( ODateTimeType, css::util::DateTime )

    class OShortIntegerType;
    typedef ODerivedDataType< OShortIntegerType, OValueLimitedType< sal_Int16 > > OShortIntegerType_Base;
    class OShortIntegerType : public OShortIntegerType_Base
    {
    public:
        OShortIntegerType( const OUString& _rName, sal_Int16 _nTypeClass );

    protected:
        DECLARE_DEFAULT_CLONING( OShortIntegerType )

        // OXSDDataType overridables
        virtual bool            _getValue( const OUString& value, double& fValue ) override;

        // OValueLimitedType overridables
        virtual OUString typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const override;
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const override;

    private:
        using OShortIntegerType_Base::initializeTypedClone;
    };


} // namespace xforms


#endif // INCLUDED_FORMS_SOURCE_XFORMS_DATATYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#pragma once

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xsd/XDataType.hpp>
#include <comphelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propertycontainer2.hxx>
#include <comphelper/proparrhlp.hxx>
#include <rtl/ref.hxx>
#include <unotools/resmgr.hxx>

#include <unicode/regex.h>

#include <memory>

#include <com/sun/star/util/URLTransformer.hpp>


namespace xforms
{


    //= OXSDDataType

    typedef ::comphelper::WeakImplHelper        <   css::xsd::XDataType
                                                >   OXSDDataType_Base;

    class OXSDDataType  :public OXSDDataType_Base
                        ,public ::comphelper::OPropertyContainer2
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
        OXSDDataType( OUString _aName, sal_Int16 _nTypeClass );
        virtual ~OXSDDataType() override;

    public:
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        virtual OUString SAL_CALL getName(  ) override;
        virtual void SAL_CALL setName( const OUString& aName ) override;
        virtual OUString SAL_CALL getPattern() override;
        virtual void SAL_CALL setPattern( const OUString& _pattern ) override;
        virtual sal_Int16 SAL_CALL getWhiteSpaceTreatment() override;
        virtual void SAL_CALL setWhiteSpaceTreatment( sal_Int16 _whitespacetreatment ) override;
        virtual sal_Bool SAL_CALL getIsBasic() override;
        virtual sal_Int16 SAL_CALL getTypeClass() override;

        virtual sal_Bool SAL_CALL validate( const OUString& value ) override;
        virtual OUString SAL_CALL explainInvalid( const OUString& value ) override;

        // XPropertySet - is a base of XDataType and needs to be disambiguated
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    public:
        rtl::Reference<OXSDDataType> clone( const OUString& _rNewName ) const;

    protected:
        // XPropertySet and friends
        virtual bool convertFastPropertyValue( std::unique_lock<std::mutex>& rGuard, css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue ) override;
        virtual void setFastPropertyValue_NoBroadcast(
                                        std::unique_lock<std::mutex>& rGuard,
                                        sal_Int32 nHandle,
                                        const css::uno::Any& rValue
                                    ) override;

        // --- own overridables ---
        // helper for implementing cloning of data types
        virtual rtl::Reference<OXSDDataType> createClone( const OUString& _rName ) const = 0;
        virtual void            initializeClone( const OXSDDataType& _rCloneSource );

        // helper method for validate and explainInvalid
        virtual TranslateId _validate( const OUString& value );
        virtual OUString _explainInvalid( TranslateId rReason );

        // helper method for checking properties values which are to be set
        virtual bool        checkPropertySanity( sal_Int32 _nHandle, const css::uno::Any& _rNewValue, OUString& _rErrorMessage );

        // register properties implemented by this instance - call the base class when overriding
        virtual void        registerProperties();
    };


    //= helper for deriving from OXSDDataType

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
        virtual void setFastPropertyValue_NoBroadcast(
                                        std::unique_lock<std::mutex>& rGuard,
                                        sal_Int32 nHandle,
                                        const css::uno::Any& rValue
                                    ) override;

        // OXSDDataType overridables
        virtual bool            _getValue( const OUString& value, double& fValue );
        virtual TranslateId     _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( TranslateId rReason ) override;

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
        const css::uno::Type&
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
                            ,public ::comphelper::OPropertyArrayUsageHelper< CONCRETE_DATA_TYPE_IMPL >
    {
    private:
        bool    m_bPropertiesRegistered;

    protected:
        ODerivedDataType( const OUString& _rName, sal_Int16 _nTypeClass );

    protected:
        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

        // XPropertySet
        virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;
    };

    class OBooleanType;
    typedef ODerivedDataType< OBooleanType > OBooleanType_Base;
    class OBooleanType : public OBooleanType_Base
    {
    public:
        explicit OBooleanType( const OUString& _rName );

    protected:
        virtual rtl::Reference<OXSDDataType> createClone(const OUString& _rName) const override;
        virtual void initializeClone(const OXSDDataType& _rCloneSource) override;

        // OXSDDataType overridables
        virtual TranslateId _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( TranslateId rReason ) override;
    };

    class OStringType;
    typedef ODerivedDataType< OStringType > OStringType_Base;
    class OStringType   :public OStringType_Base
    {
        // <properties>
        css::uno::Any m_aLength;
        css::uno::Any m_aMinLength;
        css::uno::Any m_aMaxLength;
        // </properties>

    public:
        OStringType( const OUString& _rName, sal_Int16 _nTypeClass /* = css::xsd::DataTypeClass::STRING */ );

    protected:
        virtual rtl::Reference<OXSDDataType> createClone(const OUString& _rName) const override;
        virtual void initializeClone(const OXSDDataType& _rCloneSource) override;
        void       initializeTypedClone( const OStringType& _rCloneSource );

        // OXSDDataType overridables
        virtual TranslateId     _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( TranslateId rReason ) override;
        virtual bool            checkPropertySanity( sal_Int32 _nHandle, const css::uno::Any& _rNewValue, OUString& _rErrorMessage ) override;
        virtual void            registerProperties() override;
    };

    class OAnyURIType;
    typedef ODerivedDataType< OAnyURIType > OAnyURIType_Base;
    class OAnyURIType   :public OAnyURIType_Base
    {
        // <properties>
        css::uno::Any m_aLength;
        css::uno::Any m_aMinLength;
        css::uno::Any m_aMaxLength;
        // </properties>
        // helper to check URL validity
        css::uno::Reference<css::util::XURLTransformer> m_xURLTransformer;

    public:
        OAnyURIType( const OUString& _rName, sal_Int16 _nTypeClass /* = css::xsd::DataTypeClass::anyURI */ );

    protected:
        virtual rtl::Reference<OXSDDataType> createClone(const OUString& _rName) const override;
        virtual void initializeClone(const OXSDDataType& _rCloneSource) override;
        void       initializeTypedClone( const OAnyURIType& _rCloneSource );

        // OXSDDataType overridables
        virtual TranslateId     _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( TranslateId rReason ) override;
        virtual bool            checkPropertySanity( sal_Int32 _nHandle, const css::uno::Any& _rNewValue, OUString& _rErrorMessage ) override;
        virtual void            registerProperties() override;
    };

    class ODecimalType;
    typedef ODerivedDataType< ODecimalType, OValueLimitedType< double > > ODecimalType_Base;
    class ODecimalType : public ODecimalType_Base
    {
        css::uno::Any m_aTotalDigits;
        css::uno::Any m_aFractionDigits;

    public:
        ODecimalType( const OUString& _rName, sal_Int16 _nTypeClass /* = css::xsd::DataTypeClass::DECIMAL */ );

    protected:
        virtual rtl::Reference<OXSDDataType> createClone(const OUString& _rName) const override;
        virtual void initializeClone(const OXSDDataType& _rCloneSource) override;
        void       initializeTypedClone( const ODecimalType& _rCloneSource );

        // OXSDDataType overridables
        virtual TranslateId     _validate( const OUString& value ) override;
        virtual OUString _explainInvalid( TranslateId rReason ) override;
        virtual void            registerProperties() override;

        // OValueLimitedType overridables
        virtual OUString typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const override;
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const override;

    private:
        using ODecimalType_Base::initializeTypedClone;
    };


    //=

    class ODateType;
    typedef ODerivedDataType< ODateType, OValueLimitedType< css::util::Date > > ODateType_Base;
    class ODateType : public ODateType_Base
    {
    public:
        explicit ODateType( const OUString& _rName );

    protected:
        virtual rtl::Reference<OXSDDataType> createClone(const OUString& _rName) const override;
        virtual void initializeClone(const OXSDDataType& _rCloneSource) override;

        /* OXSDDataType overridables */
        virtual TranslateId         _validate( const OUString& value ) override;
        virtual bool                _getValue( const OUString& value, double& fValue ) override;

        /* OValueLimitedType overridables */
        virtual OUString     typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const override;
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const override;

    private:
        using ODateType_Base::initializeTypedClone;
    };


    class OTimeType;
    typedef ODerivedDataType< OTimeType, OValueLimitedType< css::util::Time > > OTimeType_Base;
    class OTimeType : public OTimeType_Base
    {
    public:
        explicit OTimeType( const OUString& _rName );

    protected:
        virtual rtl::Reference<OXSDDataType> createClone(const OUString& _rName) const override;
        virtual void initializeClone(const OXSDDataType& _rCloneSource) override;

        /* OXSDDataType overridables */
        virtual TranslateId         _validate( const OUString& value ) override;
        virtual bool                _getValue( const OUString& value, double& fValue ) override;

        /* OValueLimitedType overridables */
        virtual OUString     typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const override;
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const override;

    private:
        using OTimeType_Base::initializeTypedClone;
    };



    class ODateTimeType;
    typedef ODerivedDataType< OTimeType, OValueLimitedType< css::util::DateTime > > ODateTimeType_Base;
    class ODateTimeType : public ODateTimeType_Base
    {
    public:
        explicit ODateTimeType( const OUString& _rName );

    protected:
        virtual rtl::Reference<OXSDDataType> createClone(const OUString& _rName) const override;
        virtual void initializeClone(const OXSDDataType& _rCloneSource) override;

        /* OXSDDataType overridables */
        virtual TranslateId         _validate( const OUString& value ) override;
        virtual bool                _getValue( const OUString& value, double& fValue ) override;

        /* OValueLimitedType overridables */
        virtual OUString     typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const override;
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const override;

    private:
        using ODateTimeType_Base::initializeTypedClone;
    };


    class OShortIntegerType;
    typedef ODerivedDataType< OShortIntegerType, OValueLimitedType< sal_Int16 > > OShortIntegerType_Base;
    class OShortIntegerType : public OShortIntegerType_Base
    {
    public:
        OShortIntegerType( const OUString& _rName, sal_Int16 _nTypeClass );

    protected:
        virtual rtl::Reference<OXSDDataType> createClone(const OUString& _rName) const override;
        virtual void initializeClone(const OXSDDataType& _rCloneSource) override;

        // OXSDDataType overridables
        virtual bool            _getValue( const OUString& value, double& fValue ) override;

        // OValueLimitedType overridables
        virtual OUString typedValueAsHumanReadableString( const css::uno::Any& _rValue ) const override;
        virtual void normalizeValue( const css::uno::Any& _rValue, double& _rDoubleValue ) const override;

    private:
        using OShortIntegerType_Base::initializeTypedClone;
    };


} // namespace xforms


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

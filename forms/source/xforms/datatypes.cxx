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

#include "datatypes.hxx"
#include "resourcehelper.hxx"
#include "property.hrc"
#include "convert.hxx"

#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#include <tools/datetime.hxx>
#include <rtl/math.hxx>


namespace xforms
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::util::Date;
    using ::com::sun::star::util::Time;
    using ::com::sun::star::util::DateTime;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::beans::UnknownPropertyException;
    using ::com::sun::star::beans::PropertyVetoException;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::beans::XVetoableChangeListener;

    using ::com::sun::star::beans::PropertyAttribute::BOUND;
    using ::com::sun::star::beans::PropertyAttribute::READONLY;

    using namespace ::com::sun::star::xsd;
    using namespace ::frm;
    U_NAMESPACE_USE

    OXSDDataType::OXSDDataType( const OUString& _rName, sal_Int16 _nTypeClass )
        :OXSDDataType_PBase( m_aBHelper )
        ,m_bIsBasic( true )
        ,m_nTypeClass( _nTypeClass )
        ,m_sName( _rName )
        ,m_nWST( WhiteSpaceTreatment::Preserve )
        ,m_bPatternMatcherDirty( true )
    {
    }


    OXSDDataType::~OXSDDataType()
    {
    }


    void OXSDDataType::registerProperties()
    {
        registerProperty( PROPERTY_NAME,            PROPERTY_ID_NAME,           BOUND, &m_sName,    cppu::UnoType<decltype(m_sName)>::get() );
        registerProperty( PROPERTY_XSD_WHITESPACE,  PROPERTY_ID_XSD_WHITESPACE, BOUND, &m_nWST,     cppu::UnoType<cppu::UnoUnsignedShortType>::get() );
        registerProperty( PROPERTY_XSD_PATTERN,     PROPERTY_ID_XSD_PATTERN,    BOUND, &m_sPattern, cppu::UnoType<decltype(m_sPattern)>::get() );

        registerProperty( PROPERTY_XSD_IS_BASIC,    PROPERTY_ID_XSD_IS_BASIC,   READONLY, &m_bIsBasic,      cppu::UnoType<decltype(m_bIsBasic)>::get() );
        registerProperty( PROPERTY_XSD_TYPE_CLASS,  PROPERTY_ID_XSD_TYPE_CLASS, READONLY, &m_nTypeClass,    cppu::UnoType<decltype(m_nTypeClass)>::get() );
    }


    void OXSDDataType::initializeClone( const OXSDDataType& _rCloneSource )
    {
        m_bIsBasic   = false;
        m_nTypeClass = _rCloneSource.m_nTypeClass;
        m_sPattern   = _rCloneSource.m_sPattern;
        m_nWST       = _rCloneSource.m_nWST;
    }


    OXSDDataType* OXSDDataType::clone( const OUString& _rNewName ) const
    {
        OXSDDataType* pClone = createClone( _rNewName );
        pClone->initializeClone( *this );
        return pClone;
    }


    IMPLEMENT_FORWARD_XINTERFACE2( OXSDDataType, OXSDDataType_Base, ::comphelper::OPropertyContainer )


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OXSDDataType, OXSDDataType_Base, ::comphelper::OPropertyContainer )

#define SET_PROPERTY( propertyid, value, member ) \
    setFastPropertyValue( PROPERTY_ID_##propertyid, makeAny( value ) ); \
    SAL_WARN_IF( member != value, "forms.misc", "OXSDDataType::setFoo: inconsistency!" );


    OUString SAL_CALL OXSDDataType::getName(  )
    {
        return m_sName;
    }


    void SAL_CALL OXSDDataType::setName( const OUString& aName )
    {
        // TODO: check the name for conflicts in the repository
        SET_PROPERTY( NAME, aName, m_sName );
    }


    OUString SAL_CALL OXSDDataType::getPattern()
    {
        return m_sPattern;
    }


    void SAL_CALL OXSDDataType::setPattern( const OUString& _pattern )
    {
        SET_PROPERTY( XSD_PATTERN, _pattern, m_sPattern );
    }


    sal_Int16 SAL_CALL OXSDDataType::getWhiteSpaceTreatment()
    {
        return m_nWST;
    }


    void SAL_CALL OXSDDataType::setWhiteSpaceTreatment( sal_Int16 _whitespacetreatment )
    {
        SET_PROPERTY( XSD_WHITESPACE, _whitespacetreatment, m_nWST );
    }


    sal_Bool SAL_CALL OXSDDataType::getIsBasic()
    {
        return m_bIsBasic;
    }


    sal_Int16 SAL_CALL OXSDDataType::getTypeClass()
    {
        return m_nTypeClass;
    }


    sal_Bool OXSDDataType::validate( const OUString& sValue )
    {
        return ( _validate( sValue ) == 0 );
    }


  OUString OXSDDataType::explainInvalid( const OUString& sValue )
    {
        // get reason
        sal_uInt16 nReason = _validate( sValue );

        // get resource and return localized string
        return ( nReason == 0 )
            ? OUString()
            : getResource( nReason, sValue,
                                   _explainInvalid( nReason ) );
    }


    OUString OXSDDataType::_explainInvalid( sal_uInt16 nReason )
    {
        if ( RID_STR_XFORMS_PATTERN_DOESNT_MATCH == nReason )
        {
            OSL_ENSURE( !m_sPattern.isEmpty(), "OXSDDataType::_explainInvalid: how can this error occur without a regular expression?" );
            return m_sPattern;
        }
        return OUString();
    }


    namespace
    {
        void lcl_initializePatternMatcher( ::std::unique_ptr< RegexMatcher >& _rpMatcher, const OUString& _rPattern )
        {
            UErrorCode nMatchStatus = U_ZERO_ERROR;
            UnicodeString aIcuPattern( reinterpret_cast<const UChar *>(_rPattern.getStr()), _rPattern.getLength() );
            _rpMatcher.reset( new RegexMatcher( aIcuPattern, 0, nMatchStatus ) );
            OSL_ENSURE( U_SUCCESS( nMatchStatus ), "lcl_initializePatternMatcher: invalid pattern property!" );
                // if asserts, then something changed our pattern without going to convertFastPropertyValue/checkPropertySanity
        }

        bool lcl_matchString( RegexMatcher& _rMatcher, const OUString& _rText )
        {
            UErrorCode nMatchStatus = U_ZERO_ERROR;
            UnicodeString aInput( reinterpret_cast<const UChar *>(_rText.getStr()), _rText.getLength() );
            _rMatcher.reset( aInput );
            if ( _rMatcher.matches( nMatchStatus ) )
            {
                int32_t nStart = _rMatcher.start( nMatchStatus );
                int32_t nEnd   = _rMatcher.end  ( nMatchStatus );
                if ( ( nStart == 0 ) && ( nEnd == _rText.getLength() ) )
                    return true;
            }

            return false;
        }
    }


    sal_uInt16 OXSDDataType::_validate( const OUString& _rValue )
    {
        // care for the regular expression
        if ( !m_sPattern.isEmpty() )
        {
            // ensure our pattern matcher is up to date
            if ( m_bPatternMatcherDirty )
            {
                lcl_initializePatternMatcher( m_pPatternMatcher, m_sPattern );
                m_bPatternMatcherDirty = false;
            }

            // let it match the string
            if ( !lcl_matchString( *m_pPatternMatcher.get(), _rValue ) )
                return RID_STR_XFORMS_PATTERN_DOESNT_MATCH;
        }

        return 0;
    }


    sal_Bool OXSDDataType::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
    {
        // let the base class do the conversion
        if ( !OXSDDataType_PBase::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue ) )
            return false;

        // sanity checks
        OUString sErrorMessage;
        if ( !checkPropertySanity( _nHandle, _rConvertedValue, sErrorMessage ) )
        {
            IllegalArgumentException aException;
            aException.Message = sErrorMessage;
            aException.Context = *this;
            throw IllegalArgumentException( aException );
        }

        return true;
    }


    void SAL_CALL OXSDDataType::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
    {
        OXSDDataType_PBase::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        if ( _nHandle == PROPERTY_ID_XSD_PATTERN )
            m_bPatternMatcherDirty = true;
    }


    bool OXSDDataType::checkPropertySanity( sal_Int32 _nHandle, const css::uno::Any& _rNewValue, OUString& _rErrorMessage )
    {
        if ( _nHandle == PROPERTY_ID_XSD_PATTERN )
        {
            OUString sPattern;
            OSL_VERIFY( _rNewValue >>= sPattern );

            UnicodeString aIcuPattern( reinterpret_cast<const UChar *>(sPattern.getStr()), sPattern.getLength() );
            UErrorCode nMatchStatus = U_ZERO_ERROR;
            RegexMatcher aMatcher( aIcuPattern, 0, nMatchStatus );
            if ( U_FAILURE( nMatchStatus ) )
            {
                _rErrorMessage = "This is no valid pattern.";
                return false;
            }
        }
        return true;
    }


    void SAL_CALL OXSDDataType::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
    {
        OXSDDataType_PBase::setPropertyValue( aPropertyName, aValue );
    }


    Any SAL_CALL OXSDDataType::getPropertyValue( const OUString& PropertyName )
    {
        return OXSDDataType_PBase::getPropertyValue( PropertyName );
    }


    void SAL_CALL OXSDDataType::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
    {
        OXSDDataType_PBase::addPropertyChangeListener( aPropertyName, xListener );
    }


    void SAL_CALL OXSDDataType::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
    {
        OXSDDataType_PBase::removePropertyChangeListener( aPropertyName, aListener );
    }


    void SAL_CALL OXSDDataType::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
    {
        OXSDDataType_PBase::addVetoableChangeListener( PropertyName, aListener );
    }


    void SAL_CALL OXSDDataType::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
    {
        OXSDDataType_PBase::removeVetoableChangeListener( PropertyName, aListener );
    }

    OValueLimitedType_Base::OValueLimitedType_Base( const OUString& _rName, sal_Int16 _nTypeClass )
        :OXSDDataType( _rName, _nTypeClass )
        ,m_fCachedMaxInclusive( 0 )
        ,m_fCachedMaxExclusive( 0 )
        ,m_fCachedMinInclusive( 0 )
        ,m_fCachedMinExclusive( 0 )
    {
    }


    void OValueLimitedType_Base::initializeClone( const OXSDDataType& _rCloneSource )
    {
        OXSDDataType::initializeClone( _rCloneSource );
        initializeTypedClone( static_cast< const OValueLimitedType_Base& >( _rCloneSource ) );
    }


    void OValueLimitedType_Base::initializeTypedClone( const OValueLimitedType_Base& _rCloneSource )
    {
        m_aMaxInclusive   = _rCloneSource.m_aMaxInclusive;
        m_aMaxExclusive   = _rCloneSource.m_aMaxExclusive;
        m_aMinInclusive   = _rCloneSource.m_aMinInclusive;
        m_aMinExclusive   = _rCloneSource.m_aMinExclusive;
        m_fCachedMaxInclusive   = _rCloneSource.m_fCachedMaxInclusive;
        m_fCachedMaxExclusive   = _rCloneSource.m_fCachedMaxExclusive;
        m_fCachedMinInclusive   = _rCloneSource.m_fCachedMinInclusive;
        m_fCachedMinExclusive   = _rCloneSource.m_fCachedMinExclusive;
    }


    void SAL_CALL OValueLimitedType_Base::setFastPropertyValue_NoBroadcast(
        sal_Int32 _nHandle, const css::uno::Any& _rValue )
    {
        OXSDDataType::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );

        // if one of our limit properties has been set, translate it into a double
        // value, for later efficient validation
        switch ( _nHandle )
        {
        case PROPERTY_ID_XSD_MAX_INCLUSIVE_INT:
        case PROPERTY_ID_XSD_MAX_INCLUSIVE_DOUBLE:
        case PROPERTY_ID_XSD_MAX_INCLUSIVE_DATE:
        case PROPERTY_ID_XSD_MAX_INCLUSIVE_TIME:
        case PROPERTY_ID_XSD_MAX_INCLUSIVE_DATE_TIME:
            if ( m_aMaxInclusive.hasValue() )
                normalizeValue( m_aMaxInclusive, m_fCachedMaxInclusive );
            else
                m_fCachedMaxInclusive = 0;
            break;
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_INT:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_DOUBLE:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_DATE:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_TIME:
        case PROPERTY_ID_XSD_MAX_EXCLUSIVE_DATE_TIME:
            if ( m_aMaxExclusive.hasValue() )
                normalizeValue( m_aMaxExclusive, m_fCachedMaxExclusive );
            else
                m_fCachedMaxExclusive = 0;
            break;
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_INT:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_DOUBLE:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_DATE:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_TIME:
        case PROPERTY_ID_XSD_MIN_INCLUSIVE_DATE_TIME:
            if ( m_aMinInclusive.hasValue() )
                normalizeValue( m_aMinInclusive, m_fCachedMinInclusive );
            else
                m_fCachedMinInclusive = 0;
            break;
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_INT:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_DOUBLE:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_DATE:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_TIME:
        case PROPERTY_ID_XSD_MIN_EXCLUSIVE_DATE_TIME:
            if ( m_aMinExclusive.hasValue() )
                normalizeValue( m_aMinExclusive, m_fCachedMinExclusive );
            else
                m_fCachedMinExclusive = 0;
            break;
        }
    }


    bool OValueLimitedType_Base::_getValue( const OUString& rValue, double& fValue )
    {
        // convert to double
        rtl_math_ConversionStatus eStatus;
        sal_Int32 nEnd;
        double f = ::rtl::math::stringToDouble(
            rValue, '.', sal_Unicode(0), &eStatus, &nEnd );

        // error checking...
        bool bReturn = false;
        if( eStatus == rtl_math_ConversionStatus_Ok
            && nEnd == rValue.getLength() )
        {
            bReturn = true;
            fValue = f;
        }
        return bReturn;
    }


    sal_uInt16 OValueLimitedType_Base::_validate( const OUString& rValue )
    {
        sal_uInt16 nReason = OXSDDataType::_validate( rValue );
        if( nReason == 0 )
        {

            // convert value and check format
            double f;
            if( ! _getValue( rValue, f ) )
                nReason = RID_STR_XFORMS_VALUE_IS_NOT_A;

            // check range
            else if( ( m_aMaxInclusive.hasValue() ) && f > m_fCachedMaxInclusive )
                nReason = RID_STR_XFORMS_VALUE_MAX_INCL;
            else if( ( m_aMaxExclusive.hasValue() ) && f >= m_fCachedMaxExclusive )
                nReason = RID_STR_XFORMS_VALUE_MAX_EXCL;
            else if( ( m_aMinInclusive.hasValue() ) && f < m_fCachedMinInclusive )
                nReason = RID_STR_XFORMS_VALUE_MIN_INCL;
            else if( ( m_aMinExclusive.hasValue() ) && f <= m_fCachedMinExclusive )
                nReason = RID_STR_XFORMS_VALUE_MIN_EXCL;
        }
        return nReason;
    }


    OUString OValueLimitedType_Base::_explainInvalid( sal_uInt16 nReason )
    {
        OUStringBuffer sInfo;
        switch( nReason )
        {
        case 0:
            // nothing to do!
            break;

        case RID_STR_XFORMS_VALUE_IS_NOT_A:
            sInfo.append( getName() );
            break;

        case RID_STR_XFORMS_VALUE_MAX_INCL:
            sInfo.append( typedValueAsHumanReadableString( m_aMaxInclusive ) );
            break;

        case RID_STR_XFORMS_VALUE_MAX_EXCL:
            sInfo.append( typedValueAsHumanReadableString( m_aMaxExclusive ) );
            break;

        case RID_STR_XFORMS_VALUE_MIN_INCL:
            sInfo.append( typedValueAsHumanReadableString( m_aMinInclusive ) );
            break;

        case RID_STR_XFORMS_VALUE_MIN_EXCL:
            sInfo.append( typedValueAsHumanReadableString( m_aMinExclusive ) );
            break;

        default:
            OSL_FAIL( "OValueLimitedType::_explainInvalid: unknown reason!" );
            break;
        }

        return sInfo.makeStringAndClear();
    }

    OStringType::OStringType( const OUString& _rName, sal_Int16 _nTypeClass )
        :OStringType_Base( _rName, _nTypeClass )
    {
    }


    void OStringType::registerProperties()
    {
        OStringType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_LENGTH,     m_aLength,    sal_Int32 );
        REGISTER_VOID_PROP( XSD_MIN_LENGTH, m_aMinLength, sal_Int32 );
        REGISTER_VOID_PROP( XSD_MAX_LENGTH, m_aMaxLength, sal_Int32 );
    }


    IMPLEMENT_DEFAULT_TYPED_CLONING( OStringType, OStringType_Base )


    void OStringType::initializeTypedClone( const OStringType& _rCloneSource )
    {
        m_aLength       = _rCloneSource.m_aLength;
        m_aMinLength    = _rCloneSource.m_aMinLength;
        m_aMaxLength    = _rCloneSource.m_aMaxLength;
    }


    bool OStringType::checkPropertySanity( sal_Int32 _nHandle, const Any& _rNewValue, OUString& _rErrorMessage )
    {
        // let the base class do the conversion
        if ( !OStringType_Base::checkPropertySanity( _nHandle, _rNewValue, _rErrorMessage ) )
            return false;

        _rErrorMessage.clear();
        switch ( _nHandle )
        {
            case PROPERTY_ID_XSD_LENGTH:
            case PROPERTY_ID_XSD_MIN_LENGTH:
            case PROPERTY_ID_XSD_MAX_LENGTH:
            {
                sal_Int32 nValue( 0 );
                OSL_VERIFY( _rNewValue >>= nValue );
                if ( nValue <= 0 )
                    _rErrorMessage = "Length limits must denote positive integer values.";
                        // TODO/eforms: localize the error message
            }
            break;
        }

        return _rErrorMessage.isEmpty();
    }


    sal_uInt16 OStringType::_validate( const OUString& rValue )
    {
        // check regexp, whitespace etc. in parent class
        sal_uInt16 nReason = OStringType_Base::_validate( rValue );

        if( nReason == 0 )
        {
            // check string constraints
            sal_Int32 nLength = rValue.getLength();
            sal_Int32 nLimit = 0;
            if ( m_aLength >>= nLimit )
            {
                if ( nLimit != nLength )
                    nReason = RID_STR_XFORMS_VALUE_LENGTH;
            }
            else
            {
                if ( ( m_aMaxLength >>= nLimit ) && ( nLength > nLimit ) )
                    nReason = RID_STR_XFORMS_VALUE_MAX_LENGTH;
                else if ( ( m_aMinLength >>= nLimit ) && ( nLength < nLimit ) )
                    nReason = RID_STR_XFORMS_VALUE_MIN_LENGTH;
            }
        }
        return nReason;
    }


    OUString OStringType::_explainInvalid( sal_uInt16 nReason )
    {
        sal_Int32 nValue = 0;
        OUStringBuffer sInfo;
        switch( nReason )
        {
        case 0:
            // nothing to do!
            break;

        case RID_STR_XFORMS_VALUE_LENGTH:
            if( m_aLength >>= nValue )
                sInfo.append( nValue );
            break;

        case RID_STR_XFORMS_VALUE_MAX_LENGTH:
            if( m_aMaxLength >>= nValue )
                sInfo.append( nValue );
            break;

        case RID_STR_XFORMS_VALUE_MIN_LENGTH:
            if( m_aMinLength >>= nValue )
                sInfo.append( nValue );
            break;

        default:
            sInfo.append( OStringType_Base::_explainInvalid( nReason ) );
            break;
        }
        return sInfo.makeStringAndClear();
    }

    OBooleanType::OBooleanType( const OUString& _rName )
        :OBooleanType_Base( _rName, DataTypeClass::BOOLEAN )
    {
    }

    OXSDDataType* OBooleanType::createClone( const OUString& _rName ) const
    {
        return new OBooleanType( _rName );
    }

    void OBooleanType::initializeClone( const OXSDDataType& _rCloneSource )
    {
        OBooleanType_Base::initializeClone( _rCloneSource );
    }

    sal_uInt16 OBooleanType::_validate( const OUString& sValue )
    {
        sal_uInt16 nInvalidityReason = OBooleanType_Base::_validate( sValue );
        if ( nInvalidityReason )
            return nInvalidityReason;

        bool bValid = sValue == "0" || sValue == "1" || sValue == "true" || sValue == "false";
        return bValid ? 0 : RID_STR_XFORMS_INVALID_VALUE;
    }


    OUString OBooleanType::_explainInvalid( sal_uInt16 nReason )
    {
        return ( nReason == 0 ) ? OUString() : getName();
    }

    ODecimalType::ODecimalType( const OUString& _rName, sal_Int16 _nTypeClass )
        :ODecimalType_Base( _rName, _nTypeClass )
    {
    }


    IMPLEMENT_DEFAULT_TYPED_CLONING( ODecimalType, ODecimalType_Base )


    void ODecimalType::initializeTypedClone( const ODecimalType& _rCloneSource )
    {
        m_aTotalDigits    = _rCloneSource.m_aTotalDigits;
        m_aFractionDigits = _rCloneSource.m_aFractionDigits;
    }


    void ODecimalType::registerProperties()
    {
        ODecimalType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_TOTAL_DIGITS,    m_aTotalDigits,    sal_Int32 );
        REGISTER_VOID_PROP( XSD_FRACTION_DIGITS, m_aFractionDigits, sal_Int32 );
    }


    // validate decimals and return code for which facets failed
    // to be used by: ODecimalType::validate and ODecimalType::explainInvalid
    sal_uInt16 ODecimalType::_validate( const OUString& rValue )
    {
        sal_Int16 nReason = ODecimalType_Base::_validate( rValue );

        // check digits (if no other cause is available so far)
        if( nReason == 0 )
        {
            sal_Int32 nLength = rValue.getLength();
            sal_Int32 n = 0;
            sal_Int32 nTotalDigits = 0;
            sal_Int32 nFractionDigits = 0;
            const sal_Unicode* pValue = rValue.getStr();
            for( ; n < nLength && pValue[n] != '.'; n++ )
                if( pValue[n] >= '0'
                    && pValue[n] <= '9')
                    nTotalDigits++;
            for( ; n < nLength; n++ )
                if( pValue[n] >= '0'
                    && pValue[n] <= '9')
                    nFractionDigits++;
            nTotalDigits += nFractionDigits;

            sal_Int32 nValue = 0;
            if( ( m_aTotalDigits >>= nValue ) &&  nTotalDigits > nValue )
                nReason = RID_STR_XFORMS_VALUE_TOTAL_DIGITS;
            else if( ( m_aFractionDigits >>= nValue ) &&
                     ( nFractionDigits > nValue ) )
                nReason = RID_STR_XFORMS_VALUE_FRACTION_DIGITS;
        }

        return nReason;
    }


    OUString ODecimalType::_explainInvalid( sal_uInt16 nReason )
    {
        sal_Int32 nValue = 0;
        OUStringBuffer sInfo;
        switch( nReason )
        {
        case RID_STR_XFORMS_VALUE_TOTAL_DIGITS:
            if( m_aTotalDigits >>= nValue )
                sInfo.append( nValue );
            break;

        case RID_STR_XFORMS_VALUE_FRACTION_DIGITS:
            if( m_aFractionDigits >>= nValue )
                sInfo.append( nValue );
            break;

        default:
            sInfo.append( ODecimalType_Base::_explainInvalid( nReason ) );
            break;
        }
        return sInfo.makeStringAndClear();
    }


    OUString ODecimalType::typedValueAsHumanReadableString( const Any& _rValue ) const
    {
        double fValue( 0 );
        normalizeValue( _rValue, fValue );
        return OUString::number( fValue );
    }


    void ODecimalType::normalizeValue( const Any& _rValue, double& _rDoubleValue ) const
    {
        OSL_VERIFY( _rValue >>= _rDoubleValue );
    }


    //=

#define DEFAULT_IMPLEMNENT_SUBTYPE( classname, typeclass )      \
    classname::classname( const OUString& _rName )       \
        :classname##_Base( _rName, DataTypeClass::typeclass )   \
    {                                                           \
    }                                                           \
    OXSDDataType* classname::createClone( const OUString& _rName ) const \
    {                                                       \
        return new classname( _rName );                     \
    }                                                       \
    void classname::initializeClone( const OXSDDataType& _rCloneSource ) \
    { \
         classname##_Base::initializeClone( _rCloneSource );        \
        initializeTypedClone( static_cast< const classname& >( _rCloneSource ) ); \
    } \


    //= ODateType


    DEFAULT_IMPLEMNENT_SUBTYPE( ODateType, DATE )


    sal_uInt16 ODateType::_validate( const OUString& _rValue )
    {
        return ODateType_Base::_validate( _rValue );
    }


    bool ODateType::_getValue( const OUString& value, double& fValue )
    {
        Any aTypeValue = Convert::get().toAny( value, getCppuType() );

        Date aValue;
        if ( !( aTypeValue >>= aValue ) )
            return false;

        ::Date aToolsDate( aValue.Day, aValue.Month, aValue.Year );
        fValue = aToolsDate.GetDate();
        return true;
    }


    OUString ODateType::typedValueAsHumanReadableString( const Any& _rValue ) const
    {
        OSL_PRECOND( _rValue.getValueType().equals( getCppuType() ), "ODateType::typedValueAsHumanReadableString: unexpected type" );
        return Convert::get().toXSD( _rValue );
    }


    void ODateType::normalizeValue( const Any& _rValue, double& _rDoubleValue ) const
    {
        Date aValue;
        OSL_VERIFY( _rValue >>= aValue );
        ::Date aToolsDate( aValue.Day, aValue.Month, aValue.Year );
        _rDoubleValue = aToolsDate.GetDate();
    }


    //= OTimeType


    DEFAULT_IMPLEMNENT_SUBTYPE( OTimeType, TIME )


    sal_uInt16 OTimeType::_validate( const OUString& _rValue )
    {
        return OTimeType_Base::_validate( _rValue );
    }


    bool OTimeType::_getValue( const OUString& value, double& fValue )
    {
        Any aTypedValue = Convert::get().toAny( value, getCppuType() );

        css::util::Time aValue;
        if ( !( aTypedValue >>= aValue ) )
            return false;

        ::tools::Time aToolsTime( aValue );
        // no loss/rounding; IEEE 754 double-precision floating-point
        // has a mantissa of 53 bits; we need at the very most 50 bits:
        // format of aToolsTime.GetTime() is (in decimal) hhmmssnnnnnnnnn
        // and 999999999999999 = 0x38D7EA4C67FFF
        // in reality I doubt we need (much) more than
        //     240000000000000 = 0x0DA475ABF0000
        // that is 48 bits
        fValue = aToolsTime.GetTime();
        return true;
    }


    OUString OTimeType::typedValueAsHumanReadableString( const Any& _rValue ) const
    {
        OSL_PRECOND( _rValue.getValueType().equals( getCppuType() ), "OTimeType::typedValueAsHumanReadableString: unexpected type" );
        return Convert::get().toXSD( _rValue );
    }


    void OTimeType::normalizeValue( const Any& _rValue, double& _rDoubleValue ) const
    {
        css::util::Time aValue;
        OSL_VERIFY( _rValue >>= aValue );
        ::tools::Time aToolsTime( aValue );
        _rDoubleValue = aToolsTime.GetTime();
    }


    //= ODateTimeType


    DEFAULT_IMPLEMNENT_SUBTYPE( ODateTimeType, DATETIME )


    sal_uInt16 ODateTimeType::_validate( const OUString& _rValue )
    {
        return ODateTimeType_Base::_validate( _rValue );
    }


    namespace
    {
        double lcl_normalizeDateTime( const DateTime& _rValue )
        {
            ::DateTime aToolsValue(_rValue);

            double fValue = 0;
            // days since 1.1.1900 (which is relatively arbitrary but fixed date)
            fValue += ::Date( aToolsValue ) - ::Date( 1, 1, 1900 );
            // time
            fValue += aToolsValue.GetTimeInDays();
            return fValue;
        }
    }


    bool ODateTimeType::_getValue( const OUString& value, double& fValue )
    {
        Any aTypedValue = Convert::get().toAny( value, getCppuType() );

        DateTime aValue;
        if ( !( aTypedValue >>= aValue ) )
            return false;

        fValue = lcl_normalizeDateTime( aValue );
        return true;
    }


    OUString ODateTimeType::typedValueAsHumanReadableString( const Any& _rValue ) const
    {
        OSL_PRECOND( _rValue.getValueType().equals( getCppuType() ), "OTimeType::typedValueAsHumanReadableString: unexpected type" );
        OUString sString = Convert::get().toXSD( _rValue );

        // ISO 8601 notation has a "T" to separate between date and time. Our only concession
        // to the "human readable" in the method name is to replace this T with a whitespace.
        OSL_ENSURE( sString.indexOf( 'T' ) != -1, "ODateTimeType::typedValueAsHumanReadableString: hmm - no ISO notation?" );
        return sString.replace( 'T', ' ' );
    }


    void ODateTimeType::normalizeValue( const Any& _rValue, double& _rDoubleValue ) const
    {
        DateTime aValue;
        OSL_VERIFY( _rValue >>= aValue );
        _rDoubleValue = lcl_normalizeDateTime( aValue );
    }

    OShortIntegerType::OShortIntegerType( const OUString& _rName, sal_Int16 _nTypeClass )
        :OShortIntegerType_Base( _rName, _nTypeClass )
    {
    }


    IMPLEMENT_DEFAULT_TYPED_CLONING( OShortIntegerType, OShortIntegerType_Base )


    bool OShortIntegerType::_getValue( const OUString& value, double& fValue )
    {
        fValue = (double)(sal_Int16)value.toInt32();
        // TODO/eforms
        // this does not care for values which do not fit into a sal_Int16, but simply
        // cuts them down. A better implementation here should probably return <FALSE/>
        // for those values.
        // Else, we may have a situation where the UI claims an input to be valid
        // (say "12345678"), while internally, and at submission time, this is cut to
        // some smaller value.

        // Additionally, this of course does not care for strings which are no numbers...
        return true;
    }


    OUString OShortIntegerType::typedValueAsHumanReadableString( const Any& _rValue ) const
    {
        sal_Int16 nValue( 0 );
        OSL_VERIFY( _rValue >>= nValue );
        return OUString::number( nValue );
    }


    void OShortIntegerType::normalizeValue( const Any& _rValue, double& _rDoubleValue ) const
    {
        sal_Int16 nValue( 0 );
        OSL_VERIFY( _rValue >>= nValue );
        _rDoubleValue = nValue;
    }


template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::ODerivedDataType( const OUString& _rName, sal_Int16 _nTypeClass )
    :SUPERCLASS( _rName, _nTypeClass )
    ,m_bPropertiesRegistered( false )
{
}


template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
::cppu::IPropertyArrayHelper* ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::createArrayHelper( ) const
{
    css::uno::Sequence< css::beans::Property > aProps;
    ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}


template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getPropertySetInfo()
{
        return ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
}


template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
::cppu::IPropertyArrayHelper& SAL_CALL ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getInfoHelper()
{
    if ( !m_bPropertiesRegistered )
    {
        this->registerProperties();
        m_bPropertiesRegistered = true;
    }

    return *ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getArrayHelper();
}


template< typename VALUE_TYPE >
OValueLimitedType< VALUE_TYPE >::OValueLimitedType( const OUString& _rName, sal_Int16 _nTypeClass )
    :OValueLimitedType_Base( _rName, _nTypeClass )
{
}

} // namespace xforms


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

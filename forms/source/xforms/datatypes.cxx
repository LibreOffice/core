/*************************************************************************
 *
 *  $RCSfile: datatypes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:51:08 $
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
#include "datatypes.hxx"
#endif
#ifndef _RESOURCEHELPER_HXX
#include "resourcehelper.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _CONVERT_HXX
#include "convert.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_XSD_WHITESPACETREATMENT_HPP_
#include <com/sun/star/xsd/WhiteSpaceTreatment.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

//........................................................................
namespace xforms
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::util::VetoException;
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

    //====================================================================
    //= OXSDDataType
    //====================================================================
    //--------------------------------------------------------------------
    namespace
    {
        void lcl_throwIllegalArgumentException( const sal_Char* _pAsciiErrorMessage ) SAL_THROW( (IllegalArgumentException) )
        {
            IllegalArgumentException aException;
            aException.Message = ::rtl::OUString::createFromAscii( _pAsciiErrorMessage );
            throw IllegalArgumentException( aException );
        }
    }
    //====================================================================
    //= OXSDDataType
    //====================================================================
    DBG_NAME( OXSDDataType )
    //--------------------------------------------------------------------
    OXSDDataType::OXSDDataType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
        :OXSDDataType_PBase( m_aBHelper )
        ,m_sName( _rName )
        ,m_nTypeClass( _nTypeClass )
        ,m_nWST( WhiteSpaceTreatment::Preserve )
        ,m_bIsBasic( sal_True )
        ,m_bPatternMatcherDirty( true )
    {
        DBG_CTOR( OXSDDataType, NULL );
    }

    //--------------------------------------------------------------------
    OXSDDataType::~OXSDDataType()
    {
        DBG_DTOR( OXSDDataType, NULL );
    }

    //--------------------------------------------------------------------
    void OXSDDataType::registerProperties()
    {
        registerProperty( PROPERTY_NAME,            PROPERTY_ID_NAME,           BOUND, &m_sName,    ::getCppuType( &m_sName ) );
        registerProperty( PROPERTY_XSD_WHITESPACE,  PROPERTY_ID_XSD_WHITESPACE, BOUND, &m_nWST,     ::getCppuType( &m_nWST ) );
        registerProperty( PROPERTY_XSD_PATTERN,     PROPERTY_ID_XSD_PATTERN,    BOUND, &m_sPattern, ::getCppuType( &m_sPattern ) );

        registerProperty( PROPERTY_XSD_IS_BASIC,    PROPERTY_ID_XSD_IS_BASIC,   READONLY, &m_bIsBasic,      ::getCppuType( &m_bIsBasic ) );
        registerProperty( PROPERTY_XSD_TYPE_CLASS,  PROPERTY_ID_XSD_TYPE_CLASS, READONLY, &m_nTypeClass,    ::getCppuType( &m_nTypeClass ) );
    }

    //--------------------------------------------------------------------
    void OXSDDataType::initializeClone( const OXSDDataType& _rCloneSource )
    {
        m_bIsBasic   = sal_False;
        m_nTypeClass = _rCloneSource.m_nTypeClass;
        m_sPattern   = _rCloneSource.m_sPattern;
        m_nWST       = _rCloneSource.m_nWST;
    }

    //--------------------------------------------------------------------
    OXSDDataType* OXSDDataType::clone( const ::rtl::OUString& _rNewName ) const
    {
        OXSDDataType* pClone = createClone( _rNewName );
        pClone->initializeClone( *this );
        return pClone;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OXSDDataType, OXSDDataType_Base, ::comphelper::OPropertyContainer )

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OXSDDataType, OXSDDataType_Base, ::comphelper::OPropertyContainer )

#define SET_PROPERTY( propertyid, value, member ) \
    setFastPropertyValue( PROPERTY_ID_##propertyid, makeAny( value ) ); \
    OSL_POSTCOND( member == value, "OXSDDataType::setFoo: inconsistency!" );

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OXSDDataType::getName(  ) throw (RuntimeException)
    {
        return m_sName;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::setName( const ::rtl::OUString& aName ) throw (RuntimeException, VetoException)
    {
        // TODO: check the name for conflicts in the repository
        SET_PROPERTY( NAME, aName, m_sName );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OXSDDataType::getPattern() throw (RuntimeException)
    {
        return m_sPattern;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::setPattern( const ::rtl::OUString& _pattern ) throw (RuntimeException)
    {
        SET_PROPERTY( XSD_PATTERN, _pattern, m_sPattern );
    }

    //--------------------------------------------------------------------
    sal_Int16 SAL_CALL OXSDDataType::getWhiteSpaceTreatment() throw (RuntimeException)
    {
        return m_nWST;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::setWhiteSpaceTreatment( sal_Int16 _whitespacetreatment ) throw (RuntimeException, IllegalArgumentException)
    {
        SET_PROPERTY( XSD_WHITESPACE, _whitespacetreatment, m_nWST );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OXSDDataType::getIsBasic() throw (RuntimeException)
    {
        return m_bIsBasic;
    }


    //--------------------------------------------------------------------
    sal_Int16 SAL_CALL OXSDDataType::getTypeClass() throw (RuntimeException)
    {
        return m_nTypeClass;
    }

    //--------------------------------------------------------------------
    sal_Bool OXSDDataType::validate( const ::rtl::OUString& sValue ) throw( RuntimeException )
    {
        return ( _validate( sValue ) == 0 );
    }

    //--------------------------------------------------------------------
  ::rtl::OUString OXSDDataType::explainInvalid( const ::rtl::OUString& sValue ) throw( RuntimeException )
    {
        // get reason
        sal_uInt16 nReason = _validate( sValue );

        // get resource and return localized string
        return ( nReason == 0 )
            ? ::rtl::OUString()
            : getResource( nReason, sValue,
                                   _explainInvalid( nReason ) );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OXSDDataType::_explainInvalid( sal_uInt16 nReason )
    {
        if ( RID_STR_XFORMS_PATTERN_DOESNT_MATCH == nReason )
        {
            OSL_ENSURE( m_sPattern.getLength(), "OXSDDataType::_explainInvalid: how can this error occur without a regular expression?" );
            return m_sPattern;
        }
        return ::rtl::OUString();
    }

    //--------------------------------------------------------------------
    namespace
    {
        static void lcl_initializePatternMatcher( ::std::auto_ptr< RegexMatcher >& _rpMatcher, const ::rtl::OUString& _rPattern )
        {
            UErrorCode nMatchStatus = U_ZERO_ERROR;
            UnicodeString aIcuPattern( _rPattern.getStr(), _rPattern.getLength() );
            _rpMatcher.reset( new RegexMatcher( aIcuPattern, 0, nMatchStatus ) );
            OSL_ENSURE( U_SUCCESS( nMatchStatus ), "lcl_initializePatternMatcher: invalid pattern property!" );
                // if asserts, then something changed our pattern without going to convertFastPropertyValue/checkPropertySanity
        }

        static bool lcl_matchString( RegexMatcher& _rMatcher, const ::rtl::OUString& _rText )
        {
            UErrorCode nMatchStatus = U_ZERO_ERROR;
            UnicodeString aInput( _rText.getStr(), _rText.getLength() );
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

    //--------------------------------------------------------------------
    sal_uInt16 OXSDDataType::_validate( const ::rtl::OUString& _rValue )
    {
        // care for the whitespaces
        ::rtl::OUString sConverted = Convert::convertWhitespace( _rValue, m_nWST );

        // care for the regular expression
        if ( m_sPattern.getLength() )
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

    //--------------------------------------------------------------------
    sal_Bool OXSDDataType::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw(IllegalArgumentException)
    {
        // let the base class do the conversion
        if ( !OXSDDataType_PBase::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue ) )
            return sal_False;

        // sanity checks
        ::rtl::OUString sErrorMessage;
        if ( !checkPropertySanity( _nHandle, _rConvertedValue, sErrorMessage ) )
        {
            IllegalArgumentException aException;
            aException.Message = sErrorMessage;
            aException.Context = *this;
            throw IllegalArgumentException( aException );
        }

        return sal_True;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw (Exception)
    {
        OXSDDataType_PBase::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        if ( _nHandle == PROPERTY_ID_XSD_PATTERN )
            m_bPatternMatcherDirty = true;
    }

    //--------------------------------------------------------------------
    bool OXSDDataType::checkPropertySanity( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rNewValue, ::rtl::OUString& _rErrorMessage )
    {
        if ( _nHandle == PROPERTY_ID_XSD_PATTERN )
        {
            ::rtl::OUString sPattern;
            OSL_VERIFY( _rNewValue >>= sPattern );

            UnicodeString aIcuPattern( sPattern.getStr(), sPattern.getLength() );
            UErrorCode nMatchStatus = U_ZERO_ERROR;
            RegexMatcher aMatcher( aIcuPattern, 0, nMatchStatus );
            if ( U_FAILURE( nMatchStatus ) )
            {
                _rErrorMessage = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "This is no valid pattern." ) );
                return false;
            }
        }
        return true;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::setPropertyValue( const ::rtl::OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
    {
        OXSDDataType_PBase::setPropertyValue( aPropertyName, aValue );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OXSDDataType::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        return OXSDDataType_PBase::getPropertyValue( PropertyName );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        OXSDDataType_PBase::addPropertyChangeListener( aPropertyName, xListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        OXSDDataType_PBase::removePropertyChangeListener( aPropertyName, aListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        OXSDDataType_PBase::addVetoableChangeListener( PropertyName, aListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OXSDDataType::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
    {
        OXSDDataType_PBase::removeVetoableChangeListener( PropertyName, aListener );
    }

    //====================================================================
    //= OStringType
    //====================================================================
     //--------------------------------------------------------------------
    OStringType::OStringType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
        :OStringType_Base( _rName, _nTypeClass )
    {
    }

    //--------------------------------------------------------------------
    void OStringType::registerProperties()
    {
        OStringType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_LENGTH,     m_aLength,    sal_Int32 );
        REGISTER_VOID_PROP( XSD_MIN_LENGTH, m_aMinLength, sal_Int32 );
        REGISTER_VOID_PROP( XSD_MAX_LENGTH, m_aMaxLength, sal_Int32 );
    }

    //--------------------------------------------------------------------
    IMPLEMENT_DEFAULT_TYPED_CLONING( OStringType, OStringType_Base )

    //--------------------------------------------------------------------
    void OStringType::initializeTypedClone( const OStringType& _rCloneSource )
    {
        m_aLength       = _rCloneSource.m_aLength;
        m_aMinLength    = _rCloneSource.m_aMinLength;
        m_aMaxLength    = _rCloneSource.m_aMaxLength;
    }

    //--------------------------------------------------------------------
    bool OStringType::checkPropertySanity( sal_Int32 _nHandle, const Any& _rNewValue, ::rtl::OUString& _rErrorMessage )
    {
        // let the base class do the conversion
        if ( !OStringType_Base::checkPropertySanity( _nHandle, _rNewValue, _rErrorMessage ) )
            return false;

        _rErrorMessage = ::rtl::OUString();
        switch ( _nHandle )
        {
            case PROPERTY_ID_XSD_LENGTH:
            case PROPERTY_ID_XSD_MIN_LENGTH:
            case PROPERTY_ID_XSD_MAX_LENGTH:
            {
                sal_Int32 nValue( 0 );
                OSL_VERIFY( _rNewValue >>= nValue );
                if ( nValue <= 0 )
                    _rErrorMessage = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Length limits must denote positive integer values." ) );
                        // TODO/eforms: localize the error message
            }
            break;
        }

        return _rErrorMessage.getLength() == 0;
    }

    //--------------------------------------------------------------------
    sal_uInt16 OStringType::_validate( const ::rtl::OUString& rValue )
    {
        // check regexp, whitespace etc. in parent class
        sal_uInt16 nReason = OStringType_Base::_validate( rValue );

        if( nReason == 0 )
        {
            // check string constraints
            sal_Int32 nLength = rValue.getLength();
            sal_Int32 nLimit;
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

    //--------------------------------------------------------------------
    ::rtl::OUString OStringType::_explainInvalid( sal_uInt16 nReason )
    {
        sal_Int32 nValue;
        ::rtl::OUStringBuffer sInfo;
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

    //====================================================================
    //= OBooleanType
    //====================================================================
    //--------------------------------------------------------------------
    OBooleanType::OBooleanType( const ::rtl::OUString& _rName )
        :OBooleanType_Base( _rName, DataTypeClass::BOOLEAN )
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_DEFAULT_CLONING( OBooleanType, OBooleanType_Base )

    //--------------------------------------------------------------------
    void OBooleanType::initializeTypedClone( const OBooleanType& _rCloneSource )
    {
    }

    //--------------------------------------------------------------------
    sal_uInt16 OBooleanType::_validate( const ::rtl::OUString& sValue )
    {
        sal_uInt16 nInvalidityReason = OBooleanType_Base::_validate( sValue );
        if ( nInvalidityReason )
            return nInvalidityReason;

        bool bValid =
            sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("0")) ||
            sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("1")) ||
            sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("true")) ||
            sValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("false"));
        return bValid ? 0 : RID_STR_XFORMS_INVALID_VALUE;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OBooleanType::_explainInvalid( sal_uInt16 nReason )
    {
        return ( nReason == 0 ) ? ::rtl::OUString() : getName();
    }

    //====================================================================
    //= ODecimalType
    //====================================================================
    //--------------------------------------------------------------------
    ODecimalType::ODecimalType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
        :ODecimalType_Base( _rName, _nTypeClass )
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_DEFAULT_TYPED_CLONING( ODecimalType, ODecimalType_Base )

    //--------------------------------------------------------------------
    void ODecimalType::initializeTypedClone( const ODecimalType& _rCloneSource )
    {
        m_aTotalDigits    = _rCloneSource.m_aTotalDigits;
        m_aFractionDigits = _rCloneSource.m_aFractionDigits;
    }

    //--------------------------------------------------------------------
    void ODecimalType::registerProperties()
    {
        ODecimalType_Base::registerProperties();

        REGISTER_VOID_PROP( XSD_TOTAL_DIGITS,    m_aTotalDigits,    sal_Int32 );
        REGISTER_VOID_PROP( XSD_FRACTION_DIGITS, m_aFractionDigits, sal_Int32 );
    }

    //--------------------------------------------------------------------

    // validate decimals and return code for which facets failed
    // to be used by: ODecimalType::validate and ODecimalType::explainInvalid
    sal_uInt16 ODecimalType::_validate( const ::rtl::OUString& rValue )
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
            for( ; pValue[n] != sal_Unicode('.') && n < nLength; n++ )
                if( pValue[n] >= sal_Unicode('0')
                    && pValue[n] <= sal_Unicode('9'))
                    nTotalDigits++;
            for( ; n < nLength; n++ )
                if( pValue[n] >= sal_Unicode('0')
                    && pValue[n] <= sal_Unicode('9'))
                    nFractionDigits++;
            nTotalDigits += nFractionDigits;

            sal_Int32 nValue;
            if( ( m_aTotalDigits >>= nValue ) &&  nTotalDigits > nValue )
                nReason = RID_STR_XFORMS_VALUE_TOTAL_DIGITS;
            else if( ( m_aFractionDigits >>= nValue ) &&
                     ( nFractionDigits > nValue ) )
                nReason = RID_STR_XFORMS_VALUE_FRACTION_DIGITS;
        }

        return nReason;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString ODecimalType::_explainInvalid( sal_uInt16 nReason )
    {
        sal_Int32 nValue;
        ::rtl::OUStringBuffer sInfo;
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

    //--------------------------------------------------------------------
    ::rtl::OUString ODecimalType::typedValueAsString( const Any& _rValue ) const
    {
        double nValue( 0 );
        OSL_VERIFY( _rValue >>= nValue );
        return ::rtl::OUString::valueOf( nValue );
    }

    //====================================================================
    //=
    //====================================================================
#define DEFAULT_IMPLEMNENT_SUBTYPE( classname, typeclass )      \
    classname::classname( const ::rtl::OUString& _rName )       \
        :classname##_Base( _rName, DataTypeClass::typeclass )   \
    {                                                           \
    }                                                           \
                                                                \
    IMPLEMENT_DEFAULT_CLONING( classname, classname##_Base )    \
                                                                \
    void classname::initializeTypedClone( const classname& _rCloneSource )  \
    {                                                           \
    }                                                           \


    //====================================================================
    //= ODateType
    //====================================================================
    //--------------------------------------------------------------------
    DEFAULT_IMPLEMNENT_SUBTYPE( ODateType, DATE )

    //--------------------------------------------------------------------
    sal_uInt16 ODateType::_validate( const ::rtl::OUString& _rValue )
    {
        return ODateType_Base::_validate( _rValue );
    }

    //--------------------------------------------------------------------
    bool ODateType::_getValue( const ::rtl::OUString& value, double& fValue )
    {
        // TODO/eforms
        fValue = 0;
        return true;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString ODateType::typedValueAsString( const Any& _rValue ) const
    {
        Date aDate;
        OSL_VERIFY( _rValue >>= aDate );

        ::rtl::OUStringBuffer sInfo;
        sInfo.append( (sal_Int32)aDate.Year );
        sInfo.appendAscii( "-" );
        sInfo.append( (sal_Int32)aDate.Month );
        sInfo.appendAscii( "-" );
        sInfo.append( (sal_Int32)aDate.Day );

        return sInfo.makeStringAndClear();
    }

    //====================================================================
    //= OTimeType
    //====================================================================
    //--------------------------------------------------------------------
    DEFAULT_IMPLEMNENT_SUBTYPE( OTimeType, TIME )

    //--------------------------------------------------------------------
    sal_uInt16 OTimeType::_validate( const ::rtl::OUString& _rValue )
    {
        return OTimeType_Base::_validate( _rValue );
    }

    //--------------------------------------------------------------------
    bool OTimeType::_getValue( const ::rtl::OUString& value, double& fValue )
    {
        // TODO/eforms
        fValue = 0;
        return true;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OTimeType::typedValueAsString( const Any& _rValue ) const
    {
        Time aTime;
        OSL_VERIFY( _rValue >>= aTime );

        ::rtl::OUStringBuffer sInfo;
        sInfo.append( (sal_Int32)aTime.Hours );
        sInfo.appendAscii( ":" );
        sInfo.append( (sal_Int32)aTime.Minutes );
        sInfo.appendAscii( ":" );
        sInfo.append( (sal_Int32)aTime.Seconds );
        sInfo.appendAscii( "." );
        sInfo.append( (sal_Int32)aTime.HundredthSeconds );

        return sInfo.makeStringAndClear();
    }

    //====================================================================
    //= ODateTimeType
    //====================================================================
    //--------------------------------------------------------------------
    DEFAULT_IMPLEMNENT_SUBTYPE( ODateTimeType, DATETIME )

    //--------------------------------------------------------------------
    sal_uInt16 ODateTimeType::_validate( const ::rtl::OUString& _rValue )
    {
        return ODateTimeType_Base::_validate( _rValue );
    }

    //--------------------------------------------------------------------
    bool ODateTimeType::_getValue( const ::rtl::OUString& value, double& fValue )
    {
        // TODO/eforms
        fValue = 0;
        return true;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString ODateTimeType::typedValueAsString( const Any& _rValue ) const
    {
        DateTime aDateTime;
        OSL_VERIFY( _rValue >>= aDateTime );

        // TODO/eforms

        return ::rtl::OUString();
    }

    //====================================================================
    //= OShortIntegerType
    //====================================================================
    //--------------------------------------------------------------------
    OShortIntegerType::OShortIntegerType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
        :OShortIntegerType_Base( _rName, _nTypeClass )
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_DEFAULT_TYPED_CLONING( OShortIntegerType, OShortIntegerType_Base )

    //--------------------------------------------------------------------
    void OShortIntegerType::initializeTypedClone( const OShortIntegerType& _rCloneSource )
    {
    }

    //--------------------------------------------------------------------
    bool OShortIntegerType::_getValue( const ::rtl::OUString& value, double& fValue )
    {
        fValue = (double)(sal_Int16)value.toInt32();
        // TODO/eforms
        // this does not care for values which do not fit into a sal_Int16, but simply
        // cuts them down. A better implementation here should probably return <FALSE/>
        // for those values.
        // Else, we may have a situation where the UI claims an input to be valid
        // (say "12345678"), while internally, and at submission time, this is cut to
        // some smaller value.
        //
        // Additionally, this of course does not care for strings which are no numers ...
        return true;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OShortIntegerType::typedValueAsString( const Any& _rValue ) const
    {
        sal_Int16 nValue( 0 );
        OSL_VERIFY( _rValue >>= nValue );
        return ::rtl::OUString::valueOf( (sal_Int32)nValue );
    }

    //====================================================================
    //= OByteIntegerType
    //====================================================================
    //--------------------------------------------------------------------
    OByteIntegerType::OByteIntegerType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
        :OByteIntegerType_Base( _rName, _nTypeClass )
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_DEFAULT_TYPED_CLONING( OByteIntegerType, OByteIntegerType_Base )

    //--------------------------------------------------------------------
    void OByteIntegerType::initializeTypedClone( const OByteIntegerType& _rCloneSource )
    {
    }

    //--------------------------------------------------------------------
    bool OByteIntegerType::_getValue( const ::rtl::OUString& value, double& fValue )
    {
        fValue = (double)(sal_Int8)value.toInt32();
        // TODO/eforms
        // this does not care for values which do not fit into a sal_Int8, but simply
        // cuts them down. A better implementation here should probably return <FALSE/>
        // for those values.
        // Else, we may have a situation where the UI claims an input to be valid
        // (say "12345678"), while internally, and at submission time, this is cut to
        // some smaller value.
        //
        // Additionally, this of course does not care for strings which are no numers ...
        return true;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OByteIntegerType::typedValueAsString( const Any& _rValue ) const
    {
        sal_Int8 nValue( 0 );
        OSL_VERIFY( _rValue >>= nValue );
        return ::rtl::OUString::valueOf( (sal_Int32)nValue );
    }

    //====================================================================
    //====================================================================

#define DATATYPES_INCLUDED_BY_MASTER_HEADER
#include "datatypes_impl.hxx"
#undef DATATYPES_INCLUDED_BY_MASTER_HEADER

//........................................................................
} // namespace xforms
//........................................................................


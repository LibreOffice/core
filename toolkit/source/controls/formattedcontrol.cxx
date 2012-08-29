/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <toolkit/controls/formattedcontrol.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/property.hxx>

#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;

    // -------------------------------------------------------------------
    namespace
    {
        // ...............................................................
        ::osl::Mutex& getDefaultFormatsMutex()
        {
            static ::osl::Mutex s_aDefaultFormatsMutex;
            return s_aDefaultFormatsMutex;
        }

        // ...............................................................
        Reference< XNumberFormatsSupplier >& lcl_getDefaultFormatsAccess_nothrow()
        {
            static Reference< XNumberFormatsSupplier > s_xDefaultFormats;
            return s_xDefaultFormats;
        }

        // ...............................................................
        bool& lcl_getTriedCreation()
        {
            static bool s_bTriedCreation = false;
            return s_bTriedCreation;
        }

        // ...............................................................
        const Reference< XNumberFormatsSupplier >& lcl_getDefaultFormats_throw()
        {
            ::osl::MutexGuard aGuard( getDefaultFormatsMutex() );

            bool& rbTriedCreation = lcl_getTriedCreation();
            Reference< XNumberFormatsSupplier >& rDefaultFormats( lcl_getDefaultFormatsAccess_nothrow() );
            if ( !rDefaultFormats.is() && !rbTriedCreation )
            {
                rbTriedCreation = true;
                rDefaultFormats = Reference< XNumberFormatsSupplier >(
                    ::comphelper::createProcessComponent(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.NumberFormatsSupplier" ) ) ),
                    UNO_QUERY_THROW
                );
            }
            if ( !rDefaultFormats.is() )
                throw RuntimeException();

            return rDefaultFormats;
        }

        // ...............................................................
        static oslInterlockedCount  s_refCount(0);

        // ...............................................................
        void    lcl_registerDefaultFormatsClient()
        {
            osl_incrementInterlockedCount( &s_refCount );
        }

        // ...............................................................
        void    lcl_revokeDefaultFormatsClient()
        {
            ::osl::ClearableMutexGuard aGuard( getDefaultFormatsMutex() );
            if ( 0 == osl_decrementInterlockedCount( &s_refCount ) )
            {
                Reference< XNumberFormatsSupplier >& rDefaultFormats( lcl_getDefaultFormatsAccess_nothrow() );
                Reference< XNumberFormatsSupplier > xReleasePotentialLastReference( rDefaultFormats );
                rDefaultFormats.clear();
                lcl_getTriedCreation() = false;

                aGuard.clear();
                xReleasePotentialLastReference.clear();
            }
        }
    }

    // ===================================================================
    // = UnoControlFormattedFieldModel
    // ===================================================================
    // -------------------------------------------------------------------
    UnoControlFormattedFieldModel::UnoControlFormattedFieldModel( const Reference< XMultiServiceFactory >& i_factory )
        :UnoControlModel( i_factory )
        ,m_bRevokedAsClient( false )
        ,m_bSettingValueAndText( false )
    {
        ImplRegisterProperty( BASEPROPERTY_ALIGN );
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
        ImplRegisterProperty( BASEPROPERTY_BORDER );
        ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_DEFAULT );
        ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_VALUE );
        ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_MAX );
        ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_MIN );
        ImplRegisterProperty( BASEPROPERTY_ENABLED );
        ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
        ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
        ImplRegisterProperty( BASEPROPERTY_FORMATKEY );
        ImplRegisterProperty( BASEPROPERTY_FORMATSSUPPLIER );
        ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
        ImplRegisterProperty( BASEPROPERTY_HELPURL );
        ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
        ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
        ImplRegisterProperty( BASEPROPERTY_REPEAT );
        ImplRegisterProperty( BASEPROPERTY_REPEAT_DELAY );
        ImplRegisterProperty( BASEPROPERTY_READONLY );
        ImplRegisterProperty( BASEPROPERTY_SPIN );
        ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
        ImplRegisterProperty( BASEPROPERTY_TABSTOP );
        ImplRegisterProperty( BASEPROPERTY_TEXT );
        ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
        ImplRegisterProperty( BASEPROPERTY_HIDEINACTIVESELECTION );
        ImplRegisterProperty( BASEPROPERTY_ENFORCE_FORMAT );
        ImplRegisterProperty( BASEPROPERTY_VERTICALALIGN );
        ImplRegisterProperty( BASEPROPERTY_WRITING_MODE );
        ImplRegisterProperty( BASEPROPERTY_CONTEXT_WRITING_MODE );
        ImplRegisterProperty( BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR );

        Any aTreatAsNumber;
        aTreatAsNumber <<= (sal_Bool) sal_True;
        ImplRegisterProperty( BASEPROPERTY_TREATASNUMBER, aTreatAsNumber );

        lcl_registerDefaultFormatsClient();
    }

    // -------------------------------------------------------------------
    UnoControlFormattedFieldModel::~UnoControlFormattedFieldModel()
    {
    }

    // -------------------------------------------------------------------
    ::rtl::OUString UnoControlFormattedFieldModel::getServiceName() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedFieldModel );
    }

    // -------------------------------------------------------------------
    void SAL_CALL UnoControlFormattedFieldModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
    {
        UnoControlModel::setFastPropertyValue_NoBroadcast( nHandle, rValue );

        switch ( nHandle )
        {
        case BASEPROPERTY_EFFECTIVE_VALUE:
            if ( !m_bSettingValueAndText )
                impl_updateTextFromValue_nothrow();
            break;
        case BASEPROPERTY_FORMATSSUPPLIER:
            impl_updateCachedFormatter_nothrow();
            impl_updateTextFromValue_nothrow();
            break;
        case BASEPROPERTY_FORMATKEY:
            impl_updateCachedFormatKey_nothrow();
            impl_updateTextFromValue_nothrow();
            break;
        }
    }

    // -------------------------------------------------------------------
    void UnoControlFormattedFieldModel::impl_updateTextFromValue_nothrow()
    {
        if ( !m_xCachedFormatter.is() )
            impl_updateCachedFormatter_nothrow();
        if ( !m_xCachedFormatter.is() )
            return;

        try
        {
            Any aEffectiveValue;
            getFastPropertyValue( aEffectiveValue, BASEPROPERTY_EFFECTIVE_VALUE );

            ::rtl::OUString sStringValue;
            if ( !( aEffectiveValue >>= sStringValue ) )
            {
                double nDoubleValue(0);
                if ( aEffectiveValue >>= nDoubleValue )
                {
                    sal_Int32 nFormatKey( 0 );
                    if ( m_aCachedFormat.hasValue() )
                        m_aCachedFormat >>= nFormatKey;
                    sStringValue = m_xCachedFormatter->convertNumberToString( nFormatKey, nDoubleValue );
                }
            }

            Reference< XPropertySet > xThis( *this, UNO_QUERY );
            xThis->setPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), makeAny( sStringValue ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // -------------------------------------------------------------------
    void UnoControlFormattedFieldModel::impl_updateCachedFormatter_nothrow()
    {
        Any aFormatsSupplier;
        getFastPropertyValue( aFormatsSupplier, BASEPROPERTY_FORMATSSUPPLIER );
        try
        {
            Reference< XNumberFormatsSupplier > xSupplier( aFormatsSupplier, UNO_QUERY );
            if ( !xSupplier.is() )
                xSupplier = lcl_getDefaultFormats_throw();

            if ( !m_xCachedFormatter.is() )
            {
                m_xCachedFormatter = Reference< XNumberFormatter >(
                    NumberFormatter::create(::comphelper::getProcessComponentContext()),
                    UNO_QUERY_THROW
                );
            }
            m_xCachedFormatter->attachNumberFormatsSupplier( xSupplier );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // -------------------------------------------------------------------
    void UnoControlFormattedFieldModel::impl_updateCachedFormatKey_nothrow()
    {
        Any aFormatKey;
        getFastPropertyValue( aFormatKey, BASEPROPERTY_FORMATKEY );
        m_aCachedFormat = aFormatKey;
    }

    // -------------------------------------------------------------------
    void UnoControlFormattedFieldModel::dispose(  ) throw(RuntimeException)
    {
        UnoControlModel::dispose();

        ::osl::MutexGuard aGuard( GetMutex() );
        if ( !m_bRevokedAsClient )
        {
            lcl_revokeDefaultFormatsClient();
            m_bRevokedAsClient = true;
        }
    }

    // -------------------------------------------------------------------
    void UnoControlFormattedFieldModel::ImplNormalizePropertySequence( const sal_Int32 _nCount, sal_Int32* _pHandles,
        Any* _pValues, sal_Int32* _pValidHandles ) const SAL_THROW(())
    {
        ImplEnsureHandleOrder( _nCount, _pHandles, _pValues, BASEPROPERTY_EFFECTIVE_VALUE, BASEPROPERTY_TEXT );

        UnoControlModel::ImplNormalizePropertySequence( _nCount, _pHandles, _pValues, _pValidHandles );
    }

    // -------------------------------------------------------------------
    namespace
    {
        class ResetFlagOnExit
        {
        private:
            bool&   m_rFlag;

        public:
            ResetFlagOnExit( bool& _rFlag )
                :m_rFlag( _rFlag )
            {
            }
            ~ResetFlagOnExit()
            {
                m_rFlag = false;
            }
        };
    }

    // -------------------------------------------------------------------
    void SAL_CALL UnoControlFormattedFieldModel::setPropertyValues( const Sequence< ::rtl::OUString >& _rPropertyNames, const Sequence< Any >& _rValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
    {
        bool bSettingValue = false;
        bool bSettingText = false;
        for (   const ::rtl::OUString* pPropertyNames = _rPropertyNames.getConstArray();
                pPropertyNames != _rPropertyNames.getConstArray() + _rPropertyNames.getLength();
                ++pPropertyNames
            )
        {
            if ( BASEPROPERTY_EFFECTIVE_VALUE == GetPropertyId( *pPropertyNames ) )
                bSettingValue = true;

            if ( BASEPROPERTY_TEXT == GetPropertyId( *pPropertyNames ) )
                bSettingText = true;
        }

        m_bSettingValueAndText = ( bSettingValue && bSettingText );
        ResetFlagOnExit aResetFlag( m_bSettingValueAndText );
        UnoControlModel::setPropertyValues( _rPropertyNames, _rValues );
    }

    // -------------------------------------------------------------------
    sal_Bool UnoControlFormattedFieldModel::convertFastPropertyValue(
                Any& rConvertedValue, Any& rOldValue, sal_Int32 nPropId,
                const Any& rValue ) throw (IllegalArgumentException)
    {
        if ( BASEPROPERTY_EFFECTIVE_DEFAULT == nPropId && rValue.hasValue() )
        {
            double dVal = 0;
            ::rtl::OUString sVal;
            sal_Bool bStreamed = (rValue >>= dVal);
            if ( bStreamed )
            {
                rConvertedValue <<= dVal;
            }
            else
            {
                sal_Int32  nVal = 0;
                bStreamed = (rValue >>= nVal);
                if ( bStreamed )
                {
                    rConvertedValue <<= static_cast<double>(nVal);
                }
                else
                {
                    bStreamed = (rValue >>= sVal);
                    if ( bStreamed )
                    {
                        rConvertedValue <<= sVal;
                    }
                }
            }

            if ( bStreamed )
            {
                getFastPropertyValue( rOldValue, nPropId );
                return !CompareProperties( rConvertedValue, rOldValue );
            }

            throw IllegalArgumentException(
                        ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unable to convert the given value for the property "))
                    +=  GetPropertyName((sal_uInt16)nPropId) )
                    +=  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (double, integer, or string expected).")),
                static_cast< XPropertySet* >(this),
                1);
        }

        return UnoControlModel::convertFastPropertyValue( rConvertedValue, rOldValue, nPropId, rValue );
    }

    // -------------------------------------------------------------------
    Any UnoControlFormattedFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        Any aReturn;
        switch (nPropId)
        {
            case BASEPROPERTY_DEFAULTCONTROL: aReturn <<= ::rtl::OUString( ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedField ) ); break;

            case BASEPROPERTY_TREATASNUMBER: aReturn <<= (sal_Bool)sal_True; break;

            case BASEPROPERTY_EFFECTIVE_DEFAULT:
            case BASEPROPERTY_EFFECTIVE_VALUE:
            case BASEPROPERTY_EFFECTIVE_MAX:
            case BASEPROPERTY_EFFECTIVE_MIN:
            case BASEPROPERTY_FORMATKEY:
            case BASEPROPERTY_FORMATSSUPPLIER:
                // (void)
                break;

            default : aReturn = UnoControlModel::ImplGetDefaultValue( nPropId ); break;
        }

        return aReturn;
    }

    // -------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoControlFormattedFieldModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    // beans::XMultiPropertySet
    // -------------------------------------------------------------------
    Reference< XPropertySetInfo > UnoControlFormattedFieldModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    // ===================================================================
    // = UnoFormattedFieldControl
    // ===================================================================
    // -------------------------------------------------------------------
    UnoFormattedFieldControl::UnoFormattedFieldControl( const Reference< XMultiServiceFactory >& i_factory )
        :UnoSpinFieldControl( i_factory )
    {
    }

    // -------------------------------------------------------------------
    ::rtl::OUString UnoFormattedFieldControl::GetComponentServiceName()
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FormattedField"));
    }

    // -------------------------------------------------------------------
    void UnoFormattedFieldControl::textChanged(const TextEvent& e) throw(RuntimeException)
    {
        Reference< XVclWindowPeer >  xPeer(getPeer(), UNO_QUERY);
        OSL_ENSURE(xPeer.is(), "UnoFormattedFieldControl::textChanged : what kind of peer do I have ?");

        Sequence< ::rtl::OUString > aNames( 2 );
        aNames[0] = GetPropertyName( BASEPROPERTY_EFFECTIVE_VALUE );
        aNames[1] = GetPropertyName( BASEPROPERTY_TEXT );

        Sequence< Any > aValues( 2 );
        aValues[0] = xPeer->getProperty( aNames[0] );
        aValues[1] = xPeer->getProperty( aNames[1] );

        ImplSetPropertyValues( aNames, aValues, sal_False );

        if ( GetTextListeners().getLength() )
            GetTextListeners().textChanged( e );
    }

//........................................................................
}   // namespace toolkit
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

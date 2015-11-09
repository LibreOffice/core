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

#include <toolkit/controls/formattedcontrol.hxx>
#include <toolkit/helper/property.hxx>

#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>

#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

#include "helper/unopropertyarrayhelper.hxx"

namespace toolkit
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;


    namespace
    {

        ::osl::Mutex& getDefaultFormatsMutex()
        {
            static ::osl::Mutex s_aDefaultFormatsMutex;
            return s_aDefaultFormatsMutex;
        }


        Reference< XNumberFormatsSupplier >& lcl_getDefaultFormatsAccess_nothrow()
        {
            static Reference< XNumberFormatsSupplier > s_xDefaultFormats;
            return s_xDefaultFormats;
        }


        bool& lcl_getTriedCreation()
        {
            static bool s_bTriedCreation = false;
            return s_bTriedCreation;
        }


        const Reference< XNumberFormatsSupplier >& lcl_getDefaultFormats_throw()
        {
            ::osl::MutexGuard aGuard( getDefaultFormatsMutex() );

            bool& rbTriedCreation = lcl_getTriedCreation();
            Reference< XNumberFormatsSupplier >& rDefaultFormats( lcl_getDefaultFormatsAccess_nothrow() );
            if ( !rDefaultFormats.is() && !rbTriedCreation )
            {
                rbTriedCreation = true;
                rDefaultFormats = NumberFormatsSupplier::createWithDefaultLocale( ::comphelper::getProcessComponentContext() );
            }
            if ( !rDefaultFormats.is() )
                throw RuntimeException();

            return rDefaultFormats;
        }


        static oslInterlockedCount  s_refCount(0);


        void    lcl_registerDefaultFormatsClient()
        {
            osl_atomic_increment( &s_refCount );
        }


        void    lcl_revokeDefaultFormatsClient()
        {
            ::osl::ClearableMutexGuard aGuard( getDefaultFormatsMutex() );
            if ( 0 == osl_atomic_decrement( &s_refCount ) )
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


    // = UnoControlFormattedFieldModel


    UnoControlFormattedFieldModel::UnoControlFormattedFieldModel( const Reference< XComponentContext >& rxContext )
        :UnoControlModel( rxContext )
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
        aTreatAsNumber <<= true;
        ImplRegisterProperty( BASEPROPERTY_TREATASNUMBER, aTreatAsNumber );

        lcl_registerDefaultFormatsClient();
    }


    UnoControlFormattedFieldModel::~UnoControlFormattedFieldModel()
    {
    }


    OUString UnoControlFormattedFieldModel::getServiceName() throw(RuntimeException, std::exception)
    {
        return OUString::createFromAscii( szServiceName_UnoControlFormattedFieldModel );
    }


    void SAL_CALL UnoControlFormattedFieldModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception, std::exception)
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

            OUString sStringValue;
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
                m_xCachedFormatter.set(
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


    void UnoControlFormattedFieldModel::impl_updateCachedFormatKey_nothrow()
    {
        Any aFormatKey;
        getFastPropertyValue( aFormatKey, BASEPROPERTY_FORMATKEY );
        m_aCachedFormat = aFormatKey;
    }


    void UnoControlFormattedFieldModel::dispose(  ) throw(RuntimeException, std::exception)
    {
        UnoControlModel::dispose();

        ::osl::MutexGuard aGuard( GetMutex() );
        if ( !m_bRevokedAsClient )
        {
            lcl_revokeDefaultFormatsClient();
            m_bRevokedAsClient = true;
        }
    }


    void UnoControlFormattedFieldModel::ImplNormalizePropertySequence( const sal_Int32 _nCount, sal_Int32* _pHandles,
        Any* _pValues, sal_Int32* _pValidHandles ) const
    {
        ImplEnsureHandleOrder( _nCount, _pHandles, _pValues, BASEPROPERTY_EFFECTIVE_VALUE, BASEPROPERTY_TEXT );

        UnoControlModel::ImplNormalizePropertySequence( _nCount, _pHandles, _pValues, _pValidHandles );
    }


    namespace
    {
        class ResetFlagOnExit
        {
        private:
            bool&   m_rFlag;

        public:
            explicit ResetFlagOnExit( bool& _rFlag )
                :m_rFlag( _rFlag )
            {
            }
            ~ResetFlagOnExit()
            {
                m_rFlag = false;
            }
        };
    }


    void SAL_CALL UnoControlFormattedFieldModel::setPropertyValues( const Sequence< OUString >& _rPropertyNames, const Sequence< Any >& _rValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
    {
        bool bSettingValue = false;
        bool bSettingText = false;
        for (   const OUString* pPropertyNames = _rPropertyNames.getConstArray();
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


    sal_Bool UnoControlFormattedFieldModel::convertFastPropertyValue(
                Any& rConvertedValue, Any& rOldValue, sal_Int32 nPropId,
                const Any& rValue ) throw (IllegalArgumentException, std::exception)
    {
        if ( BASEPROPERTY_EFFECTIVE_DEFAULT == nPropId && rValue.hasValue() )
        {
            double dVal = 0;
            OUString sVal;
            bool bStreamed = (rValue >>= dVal);
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
                ("Unable to convert the given value for the property "
                 + GetPropertyName((sal_uInt16)nPropId)
                 + " (double, integer, or string expected)."),
                static_cast< XPropertySet* >(this),
                1);
        }

        return UnoControlModel::convertFastPropertyValue( rConvertedValue, rOldValue, nPropId, rValue );
    }


    Any UnoControlFormattedFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        Any aReturn;
        switch (nPropId)
        {
            case BASEPROPERTY_DEFAULTCONTROL: aReturn <<= OUString( OUString::createFromAscii( szServiceName_UnoControlFormattedField ) ); break;

            case BASEPROPERTY_TREATASNUMBER: aReturn <<= true; break;

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

    Reference< XPropertySetInfo > UnoControlFormattedFieldModel::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    OUString UnoControlFormattedFieldModel::getImplementationName()
        throw (css::uno::RuntimeException, std::exception)
    {
        return OUString("stardiv.Toolkit.UnoControlFormattedFieldModel");
    }

    css::uno::Sequence<OUString>
    UnoControlFormattedFieldModel::getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception)
    {
        auto s(UnoControlModel::getSupportedServiceNames());
        s.realloc(s.getLength() + 2);
        s[s.getLength() - 2] = "com.sun.star.awt.UnoControlFormattedFieldModel";
        s[s.getLength() - 1] = "stardiv.vcl.controlmodel.FormattedField";
        return s;
    }

    // = UnoFormattedFieldControl


    UnoFormattedFieldControl::UnoFormattedFieldControl()
        :UnoSpinFieldControl()
    {
    }


    OUString UnoFormattedFieldControl::GetComponentServiceName()
    {
        return OUString("FormattedField");
    }


    void UnoFormattedFieldControl::textChanged(const TextEvent& e) throw(RuntimeException, std::exception)
    {
        Reference< XVclWindowPeer >  xPeer(getPeer(), UNO_QUERY);
        OSL_ENSURE(xPeer.is(), "UnoFormattedFieldControl::textChanged : what kind of peer do I have ?");

        Sequence< OUString > aNames( 2 );
        aNames[0] = GetPropertyName( BASEPROPERTY_EFFECTIVE_VALUE );
        aNames[1] = GetPropertyName( BASEPROPERTY_TEXT );

        Sequence< Any > aValues( 2 );
        aValues[0] = xPeer->getProperty( aNames[0] );
        aValues[1] = xPeer->getProperty( aNames[1] );

        ImplSetPropertyValues( aNames, aValues, false );

        if ( GetTextListeners().getLength() )
            GetTextListeners().textChanged( e );
    }

    OUString UnoFormattedFieldControl::getImplementationName()
        throw (css::uno::RuntimeException, std::exception)
    {
        return OUString("stardiv.Toolkit.UnoFormattedFieldControl");
    }

    css::uno::Sequence<OUString>
    UnoFormattedFieldControl::getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception)
    {
        auto s(UnoEditControl::getSupportedServiceNames());
        s.realloc(s.getLength() + 2);
        s[s.getLength() - 2] = "com.sun.star.awt.UnoControlFormattedField";
        s[s.getLength() - 1] = "stardiv.vcl.control.FormattedField";
        return s;
    }
}   // namespace toolkit


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_UnoControlFormattedFieldModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoControlFormattedFieldModel(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_UnoFormattedFieldControl_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoFormattedFieldControl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

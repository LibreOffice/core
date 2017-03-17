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

#include "richtextmodel.hxx"
#include "richtextengine.hxx"
#include "richtextunowrapper.hxx"

#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>

#include <cppuhelper/typeprovider.hxx>
#include <comphelper/guarding.hxx>
#include <svl/itempool.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <editeng/editstat.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>


namespace frm
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::style;

    namespace WritingMode2 = ::com::sun::star::text::WritingMode2;

    ORichTextModel::ORichTextModel( const Reference< XComponentContext >& _rxFactory )
        :OControlModel       ( _rxFactory, OUString() )
        ,FontControlModel    ( true                          )
        ,m_pEngine           ( RichTextEngine::Create()      )
        ,m_bSettingEngineText( false                         )
        ,m_aModifyListeners  ( m_aMutex                      )
    {
        m_nClassId = FormComponentType::TEXTFIELD;

        getPropertyDefaultByHandle( PROPERTY_ID_DEFAULTCONTROL          ) >>= m_sDefaultControl;
        getPropertyDefaultByHandle( PROPERTY_ID_BORDER                  ) >>= m_nBorder;
        getPropertyDefaultByHandle( PROPERTY_ID_ENABLED                 ) >>= m_bEnabled;
        getPropertyDefaultByHandle( PROPERTY_ID_ENABLEVISIBLE           ) >>= m_bEnableVisible;
        getPropertyDefaultByHandle( PROPERTY_ID_HARDLINEBREAKS          ) >>= m_bHardLineBreaks;
        getPropertyDefaultByHandle( PROPERTY_ID_HSCROLL                 ) >>= m_bHScroll;
        getPropertyDefaultByHandle( PROPERTY_ID_VSCROLL                 ) >>= m_bVScroll;
        getPropertyDefaultByHandle( PROPERTY_ID_READONLY                ) >>= m_bReadonly;
        getPropertyDefaultByHandle( PROPERTY_ID_PRINTABLE               ) >>= m_bPrintable;
        m_aAlign = getPropertyDefaultByHandle( PROPERTY_ID_ALIGN );
        getPropertyDefaultByHandle( PROPERTY_ID_ECHO_CHAR               ) >>= m_nEchoChar;
        getPropertyDefaultByHandle( PROPERTY_ID_MAXTEXTLEN              ) >>= m_nMaxTextLength;
        getPropertyDefaultByHandle( PROPERTY_ID_MULTILINE               ) >>= m_bMultiLine;
        getPropertyDefaultByHandle( PROPERTY_ID_RICH_TEXT               ) >>= m_bReallyActAsRichText;
        getPropertyDefaultByHandle( PROPERTY_ID_HIDEINACTIVESELECTION   ) >>= m_bHideInactiveSelection;
        getPropertyDefaultByHandle( PROPERTY_ID_LINEEND_FORMAT          ) >>= m_nLineEndFormat;
        getPropertyDefaultByHandle( PROPERTY_ID_WRITING_MODE            ) >>= m_nTextWritingMode;
        getPropertyDefaultByHandle( PROPERTY_ID_CONTEXT_WRITING_MODE    ) >>= m_nContextWritingMode;

        implInit();
    }


    ORichTextModel::ORichTextModel( const ORichTextModel* _pOriginal, const Reference< XComponentContext >& _rxFactory )
        :OControlModel       ( _pOriginal, _rxFactory, false )
        ,FontControlModel    ( _pOriginal                        )
        ,m_bSettingEngineText( false                             )
        ,m_aModifyListeners  ( m_aMutex                          )
    {

        m_aTabStop               = _pOriginal->m_aTabStop;
        m_aBackgroundColor       = _pOriginal->m_aBackgroundColor;
        m_aBorderColor           = _pOriginal->m_aBorderColor;
        m_aVerticalAlignment     = _pOriginal->m_aVerticalAlignment;
        m_sDefaultControl        = _pOriginal->m_sDefaultControl;
        m_sHelpText              = _pOriginal->m_sHelpText;
        m_sHelpURL               = _pOriginal->m_sHelpURL;
        m_nBorder                = _pOriginal->m_nBorder;
        m_bEnabled               = _pOriginal->m_bEnabled;
        m_bEnableVisible         = _pOriginal->m_bEnableVisible;
        m_bHardLineBreaks        = _pOriginal->m_bHardLineBreaks;
        m_bHScroll               = _pOriginal->m_bHScroll;
        m_bVScroll               = _pOriginal->m_bVScroll;
        m_bReadonly              = _pOriginal->m_bReadonly;
        m_bPrintable             = _pOriginal->m_bPrintable;
        m_bReallyActAsRichText   = _pOriginal->m_bReallyActAsRichText;
        m_bHideInactiveSelection = _pOriginal->m_bHideInactiveSelection;
        m_nLineEndFormat         = _pOriginal->m_nLineEndFormat;
        m_nTextWritingMode       = _pOriginal->m_nTextWritingMode;
        m_nContextWritingMode    = _pOriginal->m_nContextWritingMode;

        m_aAlign               = _pOriginal->m_aAlign;
        m_nEchoChar            = _pOriginal->m_nEchoChar;
        m_nMaxTextLength       = _pOriginal->m_nMaxTextLength;
        m_bMultiLine           = _pOriginal->m_bMultiLine;

        m_pEngine.reset(_pOriginal->m_pEngine->Clone());
        m_sLastKnownEngineText = m_pEngine->GetText();

        implInit();
    }


    void ORichTextModel::implInit()
    {
        OSL_ENSURE( m_pEngine.get(), "ORichTextModel::implInit: where's the engine?" );
        if ( m_pEngine.get() )
        {
            m_pEngine->SetModifyHdl( LINK( this, ORichTextModel, OnEngineContentModified ) );

            EEControlBits nEngineControlWord = m_pEngine->GetControlWord();
            nEngineControlWord = nEngineControlWord & ~EEControlBits::AUTOPAGESIZE;
            m_pEngine->SetControlWord( nEngineControlWord );

            VCLXDevice* pUnoRefDevice = new VCLXDevice;
            {
                SolarMutexGuard g;
                pUnoRefDevice->SetOutputDevice( m_pEngine->GetRefDevice() );
            }
            m_xReferenceDevice = pUnoRefDevice;
        }

        implDoAggregation();
        implRegisterProperties();
    }


    void ORichTextModel::implDoAggregation()
    {
        osl_atomic_increment( &m_refCount );

        {
            m_xAggregate = new ORichTextUnoWrapper( *m_pEngine, this );
            setAggregation( m_xAggregate );
            doSetDelegator();
        }

        osl_atomic_decrement( &m_refCount );
    }


    void ORichTextModel::implRegisterProperties()
    {
        REGISTER_PROP_2( DEFAULTCONTROL,        m_sDefaultControl,          BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( HELPTEXT,              m_sHelpText,                BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( HELPURL,               m_sHelpURL,                 BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( ENABLED,               m_bEnabled,                 BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( ENABLEVISIBLE,               m_bEnableVisible,                 BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( BORDER,                m_nBorder,                  BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( HARDLINEBREAKS,        m_bHardLineBreaks,          BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( HSCROLL,               m_bHScroll,                 BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( VSCROLL,               m_bVScroll,                 BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( READONLY,              m_bReadonly,                BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( PRINTABLE,             m_bPrintable,               BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( REFERENCE_DEVICE,      m_xReferenceDevice,         BOUND, TRANSIENT    );
        REGISTER_PROP_2( RICH_TEXT,             m_bReallyActAsRichText,     BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( HIDEINACTIVESELECTION, m_bHideInactiveSelection,   BOUND, MAYBEDEFAULT );

        REGISTER_VOID_PROP_2( TABSTOP,          m_aTabStop,             sal_Bool,           BOUND, MAYBEDEFAULT );
        REGISTER_VOID_PROP_2( BACKGROUNDCOLOR,  m_aBackgroundColor,     sal_Int32,          BOUND, MAYBEDEFAULT );
        REGISTER_VOID_PROP_2( BORDERCOLOR,      m_aBorderColor,         sal_Int32,          BOUND, MAYBEDEFAULT );
        REGISTER_VOID_PROP_2( VERTICAL_ALIGN,   m_aVerticalAlignment,   VerticalAlignment,  BOUND, MAYBEDEFAULT );

        // properties which exist only for compatibility with the css.swt.UnoControlEditModel,
        // since we replace the default implementation for this service
        REGISTER_PROP_2( ECHO_CHAR,             m_nEchoChar,            BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( MAXTEXTLEN,            m_nMaxTextLength,       BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( MULTILINE,             m_bMultiLine,           BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( TEXT,                  m_sLastKnownEngineText, BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( LINEEND_FORMAT,        m_nLineEndFormat,       BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( WRITING_MODE,          m_nTextWritingMode,     BOUND, MAYBEDEFAULT );
        REGISTER_PROP_3( CONTEXT_WRITING_MODE,  m_nContextWritingMode,  BOUND, MAYBEDEFAULT, TRANSIENT );

        REGISTER_VOID_PROP_2( ALIGN,        m_aAlign,           sal_Int16, BOUND, MAYBEDEFAULT );
    }


    ORichTextModel::~ORichTextModel( )
    {
        if ( !OComponentHelper::rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }
        if ( m_pEngine.get() )
        {
            SolarMutexGuard g;
            SfxItemPool* pPool = m_pEngine->getPool();
            m_pEngine.reset();
            SfxItemPool::Free(pPool);
        }


    }


    Any SAL_CALL ORichTextModel::queryAggregation( const Type& _rType )
    {
        Any aReturn = ORichTextModel_BASE::queryInterface( _rType );

        if ( !aReturn.hasValue() )
            aReturn = OControlModel::queryAggregation( _rType );

        return aReturn;
    }


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ORichTextModel, OControlModel, ORichTextModel_BASE )

    OUString SAL_CALL ORichTextModel::getImplementationName()
    {
        return OUString( "com.sun.star.comp.forms.ORichTextModel" );
    }

    Sequence< OUString > SAL_CALL ORichTextModel::getSupportedServiceNames()
    {
        Sequence< OUString > aOwnNames( 8 );
        aOwnNames[ 0 ] = FRM_SUN_COMPONENT_RICHTEXTCONTROL;
        aOwnNames[ 1 ] = "com.sun.star.text.TextRange";
        aOwnNames[ 2 ] = "com.sun.star.style.CharacterProperties";
        aOwnNames[ 3 ] = "com.sun.star.style.ParagraphProperties";
        aOwnNames[ 4 ] = "com.sun.star.style.CharacterPropertiesAsian";
        aOwnNames[ 5 ] = "com.sun.star.style.CharacterPropertiesComplex";
        aOwnNames[ 6 ] = "com.sun.star.style.ParagraphPropertiesAsian";
        aOwnNames[ 7 ] = "com.sun.star.style.ParagraphPropertiesComplex";

        return ::comphelper::combineSequences(
            getAggregateServiceNames(),
            ::comphelper::concatSequences(
                OControlModel::getSupportedServiceNames_Static(),
                aOwnNames)
        );
    }

    IMPLEMENT_DEFAULT_CLONING( ORichTextModel )


    void SAL_CALL ORichTextModel::disposing()
    {
        m_aModifyListeners.disposeAndClear( EventObject( *this ) );
        OControlModel::disposing();
    }


    namespace
    {
        void lcl_removeProperty( Sequence< Property >& _rSeq, const OUString& _rPropertyName )
        {
            Property* pLoop = _rSeq.getArray();
            Property* pEnd = _rSeq.getArray() + _rSeq.getLength();
            while ( pLoop != pEnd )
            {
                if ( pLoop->Name == _rPropertyName )
                {
                    ::std::copy( pLoop + 1, pEnd, pLoop );
                    _rSeq.realloc( _rSeq.getLength() - 1 );
                    break;
                }
                ++pLoop;
            }
        }
    }

    void ORichTextModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( 1, OControlModel )
            DECL_PROP2( TABINDEX,       sal_Int16,  BOUND,    MAYBEDEFAULT );
        END_DESCRIBE_PROPERTIES();

        // properties which the OPropertyContainerHelper is responsible for
        Sequence< Property > aContainedProperties;
        describeProperties( aContainedProperties );

        // properties which the FontControlModel is responsible for
        Sequence< Property > aFontProperties;
        describeFontRelatedProperties( aFontProperties );

        _rProps = concatSequences( aContainedProperties, aFontProperties, _rProps );
    }


    void ORichTextModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
    {
        OControlModel::describeAggregateProperties( _rAggregateProps );

        // our aggregate (the SvxUnoText) declares a FontDescriptor property, as does
        // our FormControlFont base class. We remove it from the base class' sequence
        // here, and later on care for both instances being in sync
        lcl_removeProperty( _rAggregateProps, PROPERTY_FONT );

        // similar, the WritingMode property is declared in our aggregate, too, but we override
        // it, since the aggregate does no proper PropertyState handling.
        lcl_removeProperty( _rAggregateProps, PROPERTY_WRITING_MODE );
    }


    void SAL_CALL ORichTextModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        if ( isRegisteredProperty( _nHandle ) )
        {
            OPropertyContainerHelper::getFastPropertyValue( _rValue, _nHandle );
        }
        else if ( isFontRelatedProperty( _nHandle ) )
        {
            FontControlModel::getFastPropertyValue( _rValue, _nHandle );
        }
        else
        {
            OControlModel::getFastPropertyValue( _rValue, _nHandle );
        }
    }


    sal_Bool SAL_CALL ORichTextModel::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
    {
        bool bModified = false;

        if ( isRegisteredProperty( _nHandle ) )
        {
            bModified = OPropertyContainerHelper::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
        }
        else if ( isFontRelatedProperty( _nHandle ) )
        {
            bModified = FontControlModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
        }
        else
        {
            bModified = OControlModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
        }

        return bModified;
    }


    void SAL_CALL ORichTextModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
    {
        if ( isRegisteredProperty( _nHandle ) )
        {
            OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );

            switch ( _nHandle )
            {
            case PROPERTY_ID_REFERENCE_DEVICE:
            {
            #if OSL_DEBUG_LEVEL > 0
                MapMode aOldMapMode = m_pEngine->GetRefDevice()->GetMapMode();
            #endif

                OutputDevice* pRefDevice = VCLUnoHelper::GetOutputDevice( m_xReferenceDevice );
                OSL_ENSURE( pRefDevice, "ORichTextModel::setFastPropertyValue_NoBroadcast: empty reference device?" );
                m_pEngine->SetRefDevice( pRefDevice );

            #if OSL_DEBUG_LEVEL > 0
                MapMode aNewMapMode = m_pEngine->GetRefDevice()->GetMapMode();
                OSL_ENSURE( aNewMapMode.GetMapUnit() == aOldMapMode.GetMapUnit(),
                    "ORichTextModel::setFastPropertyValue_NoBroadcast: You should not tamper with the MapUnit of the ref device!" );
                // if this assertion here is triggered, then we would need to adjust all
                // items in all text portions in all paragraphs in the attributes of the EditEngine,
                // as long as they are MapUnit-dependent. This holds at least for the FontSize.
            #endif
            }
            break;

            case PROPERTY_ID_TEXT:
            {
                MutexRelease aReleaseMutex( m_aMutex );
                impl_smlock_setEngineText( m_sLastKnownEngineText );
            }
            break;
            }   // switch ( _nHandle )
        }
        else if ( isFontRelatedProperty( _nHandle ) )
        {
            FontControlModel::setFastPropertyValue_NoBroadcast_impl(
                    *this, &ORichTextModel::setDependentFastPropertyValue,
                    _nHandle, _rValue);
        }
        else
        {
            switch ( _nHandle )
            {
            case PROPERTY_ID_WRITING_MODE:
            {
                // forward to our aggregate, so the EditEngine knows about it
                if ( m_xAggregateSet.is() )
                    m_xAggregateSet->setPropertyValue( "WritingMode", _rValue );
            }
            break;

            default:
                OControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
                break;
            }
        }
    }


    Any ORichTextModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aDefault;

        switch ( _nHandle )
        {
        case PROPERTY_ID_WRITING_MODE:
        case PROPERTY_ID_CONTEXT_WRITING_MODE:
            aDefault <<= WritingMode2::CONTEXT;
            break;

        case PROPERTY_ID_LINEEND_FORMAT:
            aDefault <<= (sal_Int16)LineEndFormat::LINE_FEED;
            break;

        case PROPERTY_ID_ECHO_CHAR:
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_MAXTEXTLEN:
            aDefault <<= (sal_Int16)0;
            break;

        case PROPERTY_ID_TABSTOP:
        case PROPERTY_ID_BACKGROUNDCOLOR:
        case PROPERTY_ID_BORDERCOLOR:
        case PROPERTY_ID_VERTICAL_ALIGN:
            /* void */
            break;

        case PROPERTY_ID_ENABLED:
        case PROPERTY_ID_ENABLEVISIBLE:
        case PROPERTY_ID_PRINTABLE:
        case PROPERTY_ID_HIDEINACTIVESELECTION:
            aDefault <<= true;
            break;

        case PROPERTY_ID_HARDLINEBREAKS:
        case PROPERTY_ID_HSCROLL:
        case PROPERTY_ID_VSCROLL:
        case PROPERTY_ID_READONLY:
        case PROPERTY_ID_MULTILINE:
        case PROPERTY_ID_RICH_TEXT:
            aDefault <<= false;
            break;

        case PROPERTY_ID_DEFAULTCONTROL:
            aDefault <<= OUString(FRM_SUN_CONTROL_RICHTEXTCONTROL);
            break;

        case PROPERTY_ID_HELPTEXT:
        case PROPERTY_ID_HELPURL:
        case PROPERTY_ID_TEXT:
            aDefault <<= OUString();
            break;

        case PROPERTY_ID_BORDER:
            aDefault <<= (sal_Int16)1;
            break;

        default:
            if ( isFontRelatedProperty( _nHandle ) )
                aDefault = FontControlModel::getPropertyDefaultByHandle( _nHandle );
            else
                aDefault = OControlModel::getPropertyDefaultByHandle( _nHandle );
        }

        return aDefault;
    }


    void ORichTextModel::impl_smlock_setEngineText( const OUString& _rText )
    {
        if ( m_pEngine.get() )
        {
            SolarMutexGuard aSolarGuard;
            m_bSettingEngineText = true;
            m_pEngine->SetText( _rText );
            m_bSettingEngineText = false;
        }
    }


    OUString SAL_CALL ORichTextModel::getServiceName()
    {
        return OUString(FRM_SUN_COMPONENT_RICHTEXTCONTROL);
    }


    RichTextEngine* ORichTextModel::getEditEngine( const Reference< XControlModel >& _rxModel )
    {
        RichTextEngine* pEngine = nullptr;

        Reference< XUnoTunnel > xTunnel( _rxModel, UNO_QUERY );
        OSL_ENSURE( xTunnel.is(), "ORichTextModel::getEditEngine: invalid model!" );
        if ( xTunnel.is() )
        {
            try
            {
                pEngine = reinterpret_cast< RichTextEngine* >( xTunnel->getSomething( getEditEngineTunnelId() ) );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "ORichTextModel::getEditEngine: caught an exception!" );
            }
        }
        return pEngine;
    }


    Sequence< sal_Int8 > ORichTextModel::getEditEngineTunnelId()
    {
        static ::cppu::OImplementationId * pId = nullptr;
        if (! pId)
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if (! pId)
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }


    IMPL_LINK_NOARG( ORichTextModel, OnEngineContentModified, LinkParamNone*, void )
    {
        if ( !m_bSettingEngineText )
        {
            m_aModifyListeners.notifyEach( &XModifyListener::modified, EventObject( *this ) );

            potentialTextChange();
                // is this a good idea? It may become expensive in case of larger texts,
                // and this method here is called for every single changed character ...
                // On the other hand, the API *requires* us to notify changes in the "Text"
                // property immediately ...
        }
    }


    sal_Int64 SAL_CALL ORichTextModel::getSomething( const Sequence< sal_Int8 >& _rId )
    {
        Sequence< sal_Int8 > aEditEngineAccessId( getEditEngineTunnelId() );
        if  (   ( _rId.getLength() == aEditEngineAccessId.getLength() )
            &&  ( 0 == memcmp( aEditEngineAccessId.getConstArray(),  _rId.getConstArray(), _rId.getLength() ) )
            )
            return reinterpret_cast< sal_Int64 >( m_pEngine.get() );

        Reference< XUnoTunnel > xAggTunnel;
        if ( query_aggregation( m_xAggregate, xAggTunnel ) )
            return xAggTunnel->getSomething( _rId );

        return 0;
    }


    void SAL_CALL ORichTextModel::addModifyListener( const Reference< XModifyListener >& _rxListener )
    {
        m_aModifyListeners.addInterface( _rxListener );
    }


    void SAL_CALL ORichTextModel::removeModifyListener( const Reference< XModifyListener >& _rxListener )
    {
        m_aModifyListeners.removeInterface( _rxListener );
    }


    void ORichTextModel::potentialTextChange( )
    {
        OUString sCurrentEngineText;
        if ( m_pEngine.get() )
            sCurrentEngineText = m_pEngine->GetText();

        if ( sCurrentEngineText != m_sLastKnownEngineText )
        {
            sal_Int32 nHandle = PROPERTY_ID_TEXT;
            Any aOldValue; aOldValue <<= m_sLastKnownEngineText;
            Any aNewValue; aNewValue <<= sCurrentEngineText;
            fire( &nHandle, &aNewValue, &aOldValue, 1, false );

            m_sLastKnownEngineText = sCurrentEngineText;
        }
    }


} // namespace frm


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_forms_ORichTextModel_get_implementation(css::uno::XComponentContext* context,
                                                          css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ORichTextModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

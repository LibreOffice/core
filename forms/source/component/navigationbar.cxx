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

#include "navigationbar.hxx"
#include <property.hxx>
#include <services.hxx>

#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>

using namespace comphelper;

namespace frm
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::container;
    using namespace ::comphelper;

    namespace WritingMode2 = ::com::sun::star::text::WritingMode2;

#define REGISTER_VOID_PROP( prop, memberAny, type ) \
    registerMayBeVoidProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT | PropertyAttribute::MAYBEVOID, \
        &memberAny, cppu::UnoType<type>::get() );

    ONavigationBarModel::ONavigationBarModel( const Reference< XComponentContext >& _rxFactory )
        :OControlModel( _rxFactory, OUString() )
        ,FontControlModel( true )
    {

        m_nClassId = FormComponentType::NAVIGATIONBAR;
        implInitPropertyContainer();

        getPropertyDefaultByHandle( PROPERTY_ID_DEFAULTCONTROL          ) >>= m_sDefaultControl;
        getPropertyDefaultByHandle( PROPERTY_ID_ICONSIZE                ) >>= m_nIconSize;
        getPropertyDefaultByHandle( PROPERTY_ID_BORDER                  ) >>= m_nBorder;
        getPropertyDefaultByHandle( PROPERTY_ID_DELAY                   ) >>= m_nDelay;
        getPropertyDefaultByHandle( PROPERTY_ID_ENABLED                 ) >>= m_bEnabled;
        getPropertyDefaultByHandle( PROPERTY_ID_ENABLEVISIBLE           ) >>= m_bEnableVisible;
        getPropertyDefaultByHandle( PROPERTY_ID_SHOW_POSITION           ) >>= m_bShowPosition;
        getPropertyDefaultByHandle( PROPERTY_ID_SHOW_NAVIGATION         ) >>= m_bShowNavigation;
        getPropertyDefaultByHandle( PROPERTY_ID_SHOW_RECORDACTIONS      ) >>= m_bShowActions;
        getPropertyDefaultByHandle( PROPERTY_ID_SHOW_FILTERSORT         ) >>= m_bShowFilterSort;
        getPropertyDefaultByHandle( PROPERTY_ID_WRITING_MODE            ) >>= m_nWritingMode;
        getPropertyDefaultByHandle( PROPERTY_ID_CONTEXT_WRITING_MODE    ) >>= m_nContextWritingMode;
    }


    ONavigationBarModel::ONavigationBarModel( const ONavigationBarModel* _pOriginal, const Reference< XComponentContext >& _rxFactory )
        :OControlModel( _pOriginal, _rxFactory )
        ,FontControlModel( _pOriginal )
    {

        implInitPropertyContainer();

        m_aTabStop              = _pOriginal->m_aTabStop;
        m_aBackgroundColor      = _pOriginal->m_aBackgroundColor;
        m_sDefaultControl       = _pOriginal->m_sDefaultControl;
        m_sHelpText             = _pOriginal->m_sHelpText;
        m_sHelpURL              = _pOriginal->m_sHelpURL;
        m_bEnabled              = _pOriginal->m_bEnabled;
        m_bEnableVisible        = _pOriginal->m_bEnableVisible;
        m_nIconSize             = _pOriginal->m_nIconSize;
        m_nBorder               = _pOriginal->m_nBorder;
        m_nDelay                = _pOriginal->m_nDelay;
        m_bShowPosition         = _pOriginal->m_bShowPosition;
        m_bShowNavigation       = _pOriginal->m_bShowNavigation;
        m_bShowActions          = _pOriginal->m_bShowActions;
        m_bShowFilterSort       = _pOriginal->m_bShowFilterSort;
        m_nWritingMode          = _pOriginal->m_nWritingMode;
        m_nContextWritingMode   = _pOriginal->m_nContextWritingMode;
    }


    void ONavigationBarModel::implInitPropertyContainer()
    {
        REGISTER_PROP_2( DEFAULTCONTROL,      m_sDefaultControl,        BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( HELPTEXT,            m_sHelpText,              BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( HELPURL,             m_sHelpURL,               BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( ENABLED,             m_bEnabled,               BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( ENABLEVISIBLE,       m_bEnableVisible,         BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( ICONSIZE,            m_nIconSize,              BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( BORDER,              m_nBorder,                BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( DELAY,               m_nDelay,                 BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( SHOW_POSITION,       m_bShowPosition,          BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( SHOW_NAVIGATION,     m_bShowNavigation,        BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( SHOW_RECORDACTIONS,  m_bShowActions,           BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( SHOW_FILTERSORT,     m_bShowFilterSort,        BOUND, MAYBEDEFAULT );
        REGISTER_PROP_2( WRITING_MODE,        m_nWritingMode,           BOUND, MAYBEDEFAULT );
        REGISTER_PROP_3( CONTEXT_WRITING_MODE,m_nContextWritingMode,    BOUND, MAYBEDEFAULT, TRANSIENT );

        REGISTER_VOID_PROP( TABSTOP,         m_aTabStop,         sal_Bool );
        REGISTER_VOID_PROP( BACKGROUNDCOLOR, m_aBackgroundColor, sal_Int32 );
    }


    ONavigationBarModel::~ONavigationBarModel()
    {
        if ( !OComponentHelper::rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }

    }


    Any SAL_CALL ONavigationBarModel::queryAggregation( const Type& _rType )
    {
        Any aReturn = ONavigationBarModel_BASE::queryInterface( _rType );

        if ( !aReturn.hasValue() )
            aReturn = OControlModel::queryAggregation( _rType );

        return aReturn;
    }


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ONavigationBarModel, OControlModel, ONavigationBarModel_BASE )


    IMPLEMENT_DEFAULT_CLONING( ONavigationBarModel )


    OUString SAL_CALL ONavigationBarModel::getImplementationName()
    {
        return "com.sun.star.comp.form.ONavigationBarModel";
    }


    Sequence< OUString > SAL_CALL ONavigationBarModel::getSupportedServiceNames()
    {
        Sequence< OUString > aSupported = OControlModel::getSupportedServiceNames_Static();
        aSupported.realloc( aSupported.getLength() + 2 );

        OUString* pArray = aSupported.getArray();
        pArray[ aSupported.getLength() - 2 ] = "com.sun.star.awt.UnoControlModel";
        pArray[ aSupported.getLength() - 1 ] = FRM_SUN_COMPONENT_NAVTOOLBAR;
        return aSupported;
    }

    OUString SAL_CALL ONavigationBarModel::getServiceName()
    {
        return FRM_SUN_COMPONENT_NAVTOOLBAR;
    }

    #define PERSIST_TABSTOP         0x0001
    #define PERSIST_BACKGROUND      0x0002
    #define PERSIST_TEXTCOLOR       0x0004
    #define PERSIST_TEXTLINECOLOR   0x0008

    #define PERSIST_ENABLED         0x0001
    #define PERSIST_LARGEICONS      0x0002
        // leaf a leap here - this will allow for two more icon size values to be stored compatibly
    #define PERSIST_SHOW_POSITION   0x0008
    #define PERSIST_SHOW_NAVIGATION 0x0010
    #define PERSIST_SHOW_ACTIONS    0x0020
    #define PERSIST_SHOW_FILTERSORT 0x0040


    void SAL_CALL ONavigationBarModel::write( const Reference< XObjectOutputStream >& _rxOutStream )
    {
        // open a section for compatibility - if we later on write additional members,
        // then older versions can skip them
        OStreamSection aEnsureBlockCompat( _rxOutStream );

        // base class
        OControlModel::write( _rxOutStream );

        {
            OStreamSection aEnsureCompat( _rxOutStream );
            // determine which properties are not void and need to be written
            sal_Int32 nNonVoids = 0;
            if ( m_aTabStop.hasValue() )
                nNonVoids |= PERSIST_TABSTOP;
            if ( m_aBackgroundColor.hasValue() )
                nNonVoids |= PERSIST_BACKGROUND;
            if ( hasTextColor() )
                nNonVoids |= PERSIST_TEXTCOLOR;
            if ( hasTextLineColor() )
                nNonVoids |= PERSIST_TEXTLINECOLOR;

            _rxOutStream->writeLong( nNonVoids );

            // the maybeboid anys
            if ( nNonVoids & PERSIST_TABSTOP )
            {
                bool bTabStop( false );
                m_aTabStop >>= bTabStop;
                _rxOutStream->writeBoolean( bTabStop );
            }
            if ( nNonVoids & PERSIST_BACKGROUND )
            {
                sal_Int32 nBackgroundColor = 0;
                m_aBackgroundColor >>= nBackgroundColor;
                _rxOutStream->writeLong( nBackgroundColor );
            }
            if ( nNonVoids & PERSIST_TEXTCOLOR )
            {
               _rxOutStream->writeLong( sal_Int32(getTextColor()) );
            }
            if ( nNonVoids & PERSIST_TEXTLINECOLOR )
            {
                _rxOutStream->writeLong( sal_Int32(getTextLineColor()) );
            }
        }

        {
            OStreamSection aEnsureCompat( _rxOutStream );
            ::comphelper::operator<<( _rxOutStream, getFont() );
        }

        // our boolean flags
        sal_Int32 nFlags = 0;
        if ( m_bEnabled        ) nFlags |= PERSIST_ENABLED;
        if ( m_nIconSize       ) nFlags |= PERSIST_LARGEICONS;   // at the moment, this is quasi boolean
        if ( m_bShowPosition   ) nFlags |= PERSIST_SHOW_POSITION;
        if ( m_bShowNavigation ) nFlags |= PERSIST_SHOW_NAVIGATION;
        if ( m_bShowActions    ) nFlags |= PERSIST_SHOW_ACTIONS;
        if ( m_bShowFilterSort ) nFlags |= PERSIST_SHOW_FILTERSORT;
        _rxOutStream->writeLong( nFlags );

        // our strings
        _rxOutStream->writeUTF( m_sHelpText       );
        _rxOutStream->writeUTF( m_sHelpURL        );
        _rxOutStream->writeUTF( m_sDefaultControl );

        // misc
        _rxOutStream->writeShort( m_nBorder );
        _rxOutStream->writeLong ( m_nDelay  );
    }


    void SAL_CALL ONavigationBarModel::read( const Reference< XObjectInputStream >& _rxInStream )
    {
        OStreamSection aEnsureBlockCompat( _rxInStream );

        // base class
        OControlModel::read( _rxInStream );

        {
            OStreamSection aEnsureCompat( _rxInStream );
            // determine which properties were non-void
            sal_Int32 nNonVoids = _rxInStream->readLong( );

            // the maybeboid anys
            if ( nNonVoids & PERSIST_TABSTOP )
                m_aTabStop <<= _rxInStream->readBoolean();
            else
                m_aTabStop.clear();

            if ( nNonVoids & PERSIST_BACKGROUND )
                m_aBackgroundColor <<= _rxInStream->readLong();
            else
                m_aBackgroundColor.clear();

            if ( nNonVoids & PERSIST_TEXTCOLOR )
                setTextColor( ::Color(ColorTransparency, _rxInStream->readLong()) );
            else
                clearTextColor();

            if ( nNonVoids & PERSIST_TEXTLINECOLOR )
                setTextLineColor( ::Color(ColorTransparency, _rxInStream->readLong()) );
            else
                clearTextLineColor();
        }

        {
            OStreamSection aEnsureCompat( _rxInStream );
            FontDescriptor aFont;
            ::comphelper::operator>>( _rxInStream, aFont );
            setFont( aFont );
        }

        // our boolean flags
        sal_Int32 nFlags = _rxInStream->readLong( );
        m_bEnabled        = ( nFlags & PERSIST_ENABLED         ) != 0;
        m_nIconSize       = ( nFlags & PERSIST_LARGEICONS      ) ? 1 : 0;
        m_bShowPosition   = ( nFlags & PERSIST_SHOW_POSITION   ) != 0;
        m_bShowNavigation = ( nFlags & PERSIST_SHOW_NAVIGATION ) != 0;
        m_bShowActions    = ( nFlags & PERSIST_SHOW_ACTIONS    ) != 0;
        m_bShowFilterSort = ( nFlags & PERSIST_SHOW_FILTERSORT ) != 0;

        // our strings
        m_sHelpText       = _rxInStream->readUTF( );
        m_sHelpURL        = _rxInStream->readUTF( );
        m_sDefaultControl = _rxInStream->readUTF( );

        // misc
        m_nBorder = _rxInStream->readShort();
        m_nDelay  = _rxInStream->readLong();
    }


    void SAL_CALL ONavigationBarModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
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


    sal_Bool SAL_CALL ONavigationBarModel::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue,
        sal_Int32 _nHandle, const Any& _rValue )
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


    void SAL_CALL ONavigationBarModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
    {
        if ( isRegisteredProperty( _nHandle ) )
        {
            OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );
        }
        else if ( isFontRelatedProperty( _nHandle ) )
        {
            FontControlModel::setFastPropertyValue_NoBroadcast_impl(
                    *this, &ONavigationBarModel::setDependentFastPropertyValue,
                    _nHandle, _rValue);
        }
        else
        {
            OControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }


    Any ONavigationBarModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aDefault;

        switch ( _nHandle )
        {
        case PROPERTY_ID_TABSTOP:
        case PROPERTY_ID_BACKGROUNDCOLOR:
            /* void */
            break;
        case PROPERTY_ID_WRITING_MODE:
        case PROPERTY_ID_CONTEXT_WRITING_MODE:
            aDefault <<= WritingMode2::CONTEXT;
            break;

        case PROPERTY_ID_ENABLED:
        case PROPERTY_ID_ENABLEVISIBLE:
        case PROPERTY_ID_SHOW_POSITION:
        case PROPERTY_ID_SHOW_NAVIGATION:
        case PROPERTY_ID_SHOW_RECORDACTIONS:
        case PROPERTY_ID_SHOW_FILTERSORT:
            aDefault <<= true;
            break;

        case PROPERTY_ID_ICONSIZE:
            aDefault <<= sal_Int16(0);
            break;

        case PROPERTY_ID_DEFAULTCONTROL:
            aDefault <<= OUString( "com.sun.star.form.control.NavigationToolBar" );
            break;

        case PROPERTY_ID_HELPTEXT:
        case PROPERTY_ID_HELPURL:
            aDefault <<= OUString();
            break;

        case PROPERTY_ID_BORDER:
            aDefault <<= sal_Int16(0);
            break;

        case PROPERTY_ID_DELAY:
            aDefault <<= sal_Int32(20);
            break;

        default:
            if ( isFontRelatedProperty( _nHandle ) )
                aDefault = FontControlModel::getPropertyDefaultByHandle( _nHandle );
            else
                aDefault = OControlModel::getPropertyDefaultByHandle( _nHandle );
        }
        return aDefault;
    }


    void ONavigationBarModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( 1, OControlModel )
            DECL_PROP2( TABINDEX,           sal_Int16,          BOUND, MAYBEDEFAULT );
        END_DESCRIBE_PROPERTIES();

        // properties which the OPropertyContainerHelper is responsible for
        Sequence< Property > aContainedProperties;
        describeProperties( aContainedProperties );

        // properties which the FontControlModel is responsible for
        Sequence< Property > aFontProperties;
        describeFontRelatedProperties( aFontProperties );

        _rProps = concatSequences(
            aContainedProperties,
            aFontProperties,
            _rProps
        );
    }

}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_form_ONavigationBarModel_get_implementation(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::ONavigationBarModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

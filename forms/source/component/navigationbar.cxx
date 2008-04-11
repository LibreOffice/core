/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: navigationbar.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "navigationbar.hxx"
#include "frm_module.hxx"
#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ONavigationBarModel()
{
    static ::frm::OMultiInstanceAutoRegistration< ::frm::ONavigationBarModel > aAutoRegistration;
}

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::container;
    using namespace ::comphelper;

#define REGISTER_PROP( prop, member ) \
    registerProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT, \
        &member, ::getCppuType( &member ) );

#define REGISTER_VOID_PROP( prop, memberAny, type ) \
    registerMayBeVoidProperty( PROPERTY_##prop, PROPERTY_ID_##prop, PropertyAttribute::BOUND | PropertyAttribute::MAYBEDEFAULT | PropertyAttribute::MAYBEVOID, \
        &memberAny, ::getCppuType( static_cast< type* >( NULL ) ) );

    //==================================================================
    // ONavigationBarModel
    //==================================================================
    DBG_NAME( ONavigationBarModel )
    //------------------------------------------------------------------
    ONavigationBarModel::ONavigationBarModel( const Reference< XMultiServiceFactory >& _rxFactory )
        :OControlModel( _rxFactory, ::rtl::OUString() )
        ,FontControlModel( true )
    {
        DBG_CTOR( ONavigationBarModel, NULL );

        m_nClassId = FormComponentType::NAVIGATIONBAR;
        implInitPropertyContainer();

        getPropertyDefaultByHandle( PROPERTY_ID_DEFAULTCONTROL     ) >>= m_sDefaultControl;
        getPropertyDefaultByHandle( PROPERTY_ID_ICONSIZE           ) >>= m_nIconSize;
        getPropertyDefaultByHandle( PROPERTY_ID_BORDER             ) >>= m_nBorder;
        getPropertyDefaultByHandle( PROPERTY_ID_DELAY              ) >>= m_nDelay;
        getPropertyDefaultByHandle( PROPERTY_ID_ENABLED            ) >>= m_bEnabled;
        getPropertyDefaultByHandle( PROPERTY_ID_SHOW_POSITION      ) >>= m_bShowPosition;
        getPropertyDefaultByHandle( PROPERTY_ID_SHOW_NAVIGATION    ) >>= m_bShowNavigation;
        getPropertyDefaultByHandle( PROPERTY_ID_SHOW_RECORDACTIONS ) >>= m_bShowActions;
        getPropertyDefaultByHandle( PROPERTY_ID_SHOW_FILTERSORT    ) >>= m_bShowFilterSort;
    }

    //------------------------------------------------------------------
    ONavigationBarModel::ONavigationBarModel( const ONavigationBarModel* _pOriginal, const Reference< XMultiServiceFactory >& _rxFactory )
        :OControlModel( _pOriginal, _rxFactory )
        ,FontControlModel( _pOriginal )
    {
        DBG_CTOR( ONavigationBarModel, NULL );

        implInitPropertyContainer();

        m_aTabStop          = _pOriginal->m_aTabStop;
        m_aBackgroundColor  = _pOriginal->m_aBackgroundColor;
        m_sDefaultControl   = _pOriginal->m_sDefaultControl;
        m_sHelpText         = _pOriginal->m_sHelpText;
        m_sHelpURL          = _pOriginal->m_sHelpURL;
        m_bEnabled          = _pOriginal->m_bEnabled;
        m_nIconSize         = _pOriginal->m_nIconSize;
        m_nBorder           = _pOriginal->m_nBorder;
        m_nDelay            = _pOriginal->m_nDelay;
        m_bShowPosition     = _pOriginal->m_bShowPosition;
        m_bShowNavigation   = _pOriginal->m_bShowNavigation;
        m_bShowActions      = _pOriginal->m_bShowActions;
        m_bShowFilterSort   = _pOriginal->m_bShowFilterSort;
    }

    //------------------------------------------------------------------
    void ONavigationBarModel::implInitPropertyContainer()
    {
        REGISTER_PROP( DEFAULTCONTROL,     m_sDefaultControl  );
        REGISTER_PROP( HELPTEXT,           m_sHelpText        );
        REGISTER_PROP( HELPURL,            m_sHelpURL         );
        REGISTER_PROP( ENABLED,            m_bEnabled         );
        REGISTER_PROP( ICONSIZE,           m_nIconSize        );
        REGISTER_PROP( BORDER,             m_nBorder          );
        REGISTER_PROP( DELAY,              m_nDelay           );
        REGISTER_PROP( SHOW_POSITION,      m_bShowPosition    );
        REGISTER_PROP( SHOW_NAVIGATION,    m_bShowNavigation  );
        REGISTER_PROP( SHOW_RECORDACTIONS, m_bShowActions     );
        REGISTER_PROP( SHOW_FILTERSORT,    m_bShowFilterSort  );

        REGISTER_VOID_PROP( TABSTOP,         m_aTabStop,         sal_Bool );
        REGISTER_VOID_PROP( BACKGROUNDCOLOR, m_aBackgroundColor, sal_Int32 );
    }

    //------------------------------------------------------------------
    ONavigationBarModel::~ONavigationBarModel()
    {
        if ( !OComponentHelper::rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }

        DBG_DTOR( ONavigationBarModel, NULL );
    }

    //------------------------------------------------------------------
    Any SAL_CALL ONavigationBarModel::queryAggregation( const Type& _rType ) throw ( RuntimeException )
    {
        Any aReturn = ONavigationBarModel_BASE::queryInterface( _rType );

        if ( !aReturn.hasValue() )
            aReturn = OControlModel::queryAggregation( _rType );

        return aReturn;
    }

    //------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( ONavigationBarModel, OControlModel, ONavigationBarModel_BASE )

    //------------------------------------------------------------------------------
    IMPLEMENT_DEFAULT_CLONING( ONavigationBarModel )

    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ONavigationBarModel::getImplementationName()  throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ONavigationBarModel::getSupportedServiceNames()  throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ONavigationBarModel::getImplementationName_Static()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.form.ONavigationBarModel" ) );
    }

    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ONavigationBarModel::getSupportedServiceNames_Static()
    {
        Sequence< ::rtl::OUString > aSupported = OControlModel::getSupportedServiceNames_Static();
        aSupported.realloc( aSupported.getLength() + 2 );

        ::rtl::OUString* pArray = aSupported.getArray();
        pArray[ aSupported.getLength() - 2 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlModel" ) );
        pArray[ aSupported.getLength() - 1 ] = FRM_SUN_COMPONENT_NAVTOOLBAR;
        return aSupported;
    }

    //------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ONavigationBarModel::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return *( new ONavigationBarModel( _rxFactory ) );
    }

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarModel::disposing()
    {
        OControlModel::disposing( );
    }

    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ONavigationBarModel::getServiceName() throw ( RuntimeException )
    {
        return FRM_SUN_COMPONENT_NAVTOOLBAR;
    }

    //------------------------------------------------------------------
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

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarModel::write( const Reference< XObjectOutputStream >& _rxOutStream ) throw ( IOException, RuntimeException )
    {
        // open a section for compatibility - if we later on write additional members,
        // then older versions can skip them
        Reference< XDataOutputStream > xDataOut( _rxOutStream, UNO_QUERY );
        OStreamSection aEnsureBlockCompat( xDataOut );

        // base class
        OControlModel::write( _rxOutStream );

        {
            OStreamSection aEnsureCompat( xDataOut );
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
                sal_Bool bTabStop( sal_False );
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
               _rxOutStream->writeLong( getTextColor() );
            }
            if ( nNonVoids & PERSIST_TEXTLINECOLOR )
            {
                _rxOutStream->writeLong( getTextLineColor() );
            }
        }

        {
            OStreamSection aEnsureCompat( xDataOut );
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

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarModel::read( const Reference< XObjectInputStream >& _rxInStream ) throw ( IOException, RuntimeException )
    {
        Reference< XDataInputStream > xDataIn( _rxInStream, UNO_QUERY );
        OStreamSection aEnsureBlockCompat( xDataIn );

        // base class
        OControlModel::read( _rxInStream );

        {
            OStreamSection aEnsureCompat( xDataIn );
            // determine which properties were non-void
            sal_Int32 nNonVoids = _rxInStream->readLong( );

            // the maybeboid anys
            if ( nNonVoids & PERSIST_TABSTOP )
                m_aTabStop = makeAny( _rxInStream->readBoolean() );
            else
                m_aTabStop.clear();

            if ( nNonVoids & PERSIST_BACKGROUND )
                m_aBackgroundColor = makeAny( _rxInStream->readLong() );
            else
                m_aBackgroundColor.clear();

            if ( nNonVoids & PERSIST_TEXTCOLOR )
                setTextColor( _rxInStream->readLong() );
            else
                clearTextColor();

            if ( nNonVoids & PERSIST_TEXTLINECOLOR )
                setTextLineColor( _rxInStream->readLong() );
            else
                clearTextLineColor();
        }

        {
            OStreamSection aEnsureCompat( xDataIn );
            FontDescriptor aFont;
            ::comphelper::operator>>( _rxInStream, aFont );
            setFont( aFont );
        }

        // our boolean flags
        sal_Int32 nFlags = _rxInStream->readLong( );
        m_bEnabled        = ( nFlags & PERSIST_ENABLED         ) ? sal_True : sal_False;
        m_nIconSize       = ( nFlags & PERSIST_LARGEICONS      ) ?        1 :         0;
        m_bShowPosition   = ( nFlags & PERSIST_SHOW_POSITION   ) ? sal_True : sal_False;
        m_bShowNavigation = ( nFlags & PERSIST_SHOW_NAVIGATION ) ? sal_True : sal_False;
        m_bShowActions    = ( nFlags & PERSIST_SHOW_ACTIONS    ) ? sal_True : sal_False;
        m_bShowFilterSort = ( nFlags & PERSIST_SHOW_FILTERSORT ) ? sal_True : sal_False;

        // our strings
        m_sHelpText       = _rxInStream->readUTF( );
        m_sHelpURL        = _rxInStream->readUTF( );
        m_sDefaultControl = _rxInStream->readUTF( );

        // misc
        m_nBorder = _rxInStream->readShort();
        m_nDelay  = _rxInStream->readLong();
    }

    //------------------------------------------------------------------
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

    //------------------------------------------------------------------
    sal_Bool SAL_CALL ONavigationBarModel::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue,
        sal_Int32 _nHandle, const Any& _rValue ) throw( IllegalArgumentException )
    {
        sal_Bool bModified = sal_False;

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

    //------------------------------------------------------------------
    void SAL_CALL ONavigationBarModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw ( Exception )
    {
        if ( isRegisteredProperty( _nHandle ) )
        {
            OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );
        }
        else if ( isFontRelatedProperty( _nHandle ) )
        {
            FontDescriptor aOldFont( getFont() );

            FontControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );

            if ( isFontAggregateProperty( _nHandle ) )
                firePropertyChange( PROPERTY_ID_FONT, makeAny( getFont() ), makeAny( aOldFont ) );
        }
        else
        {
            OControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }

    //------------------------------------------------------------------
    Any ONavigationBarModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aDefault;

        switch ( _nHandle )
        {
        case PROPERTY_ID_TABSTOP:
        case PROPERTY_ID_BACKGROUNDCOLOR:
            /* void */
            break;

        case PROPERTY_ID_ENABLED:
        case PROPERTY_ID_SHOW_POSITION:
        case PROPERTY_ID_SHOW_NAVIGATION:
        case PROPERTY_ID_SHOW_RECORDACTIONS:
        case PROPERTY_ID_SHOW_FILTERSORT:
            aDefault <<= (sal_Bool)sal_True;
            break;

        case PROPERTY_ID_ICONSIZE:
            aDefault <<= (sal_Int16)0;
            break;

        case PROPERTY_ID_DEFAULTCONTROL:
            aDefault <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.control.NavigationToolBar" ) );
            break;

        case PROPERTY_ID_HELPTEXT:
        case PROPERTY_ID_HELPURL:
            aDefault <<= ::rtl::OUString();
            break;

        case PROPERTY_ID_BORDER:
            aDefault <<= (sal_Int16)0;
            break;

        case PROPERTY_ID_DELAY:
            aDefault <<= (sal_Int32)20;
            break;

        default:
            if ( isFontRelatedProperty( _nHandle ) )
                aDefault = FontControlModel::getPropertyDefaultByHandle( _nHandle );
            else
                aDefault = OControlModel::getPropertyDefaultByHandle( _nHandle );
        }
        return aDefault;
    }

    //------------------------------------------------------------------
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

//.........................................................................
}   // namespace frm
//.........................................................................

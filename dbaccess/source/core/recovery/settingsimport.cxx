/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#include "precompiled_dbaccess.hxx"

#include "settingsimport.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::xml::sax::XAttributeList;
    /** === end UNO using === **/

    //====================================================================
    //= SettingsImport
    //====================================================================
    //--------------------------------------------------------------------
    SettingsImport::SettingsImport()
        :m_refCount( 0 )
    {
    }

    //--------------------------------------------------------------------
    SettingsImport::~SettingsImport()
    {
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL SettingsImport::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL SettingsImport::release()
    {
        oslInterlockedCount newCount = osl_decrementInterlockedCount( &m_refCount );
        if ( newCount == 0 )
            delete this;
        return newCount;
    }

    //--------------------------------------------------------------------
    void SettingsImport::startElement( const Reference< XAttributeList >& i_rAttributes )
    {
        // find the name of the setting
        if ( i_rAttributes.is() )
        {
            m_sItemName = i_rAttributes->getValueByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "config:name" ) ) );
            m_sItemType = i_rAttributes->getValueByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "config:type" ) ) );
        }
    }

    //--------------------------------------------------------------------
    void SettingsImport::endElement()
    {
    }

    //--------------------------------------------------------------------
    void SettingsImport::characters( const ::rtl::OUString& i_rCharacters )
    {
        m_aCharacters.append( i_rCharacters );
    }

    //--------------------------------------------------------------------
    void SettingsImport::split( const ::rtl::OUString& i_rElementName, ::rtl::OUString& o_rNamespace, ::rtl::OUString& o_rLocalName )
    {
        o_rNamespace = ::rtl::OUString();
        o_rLocalName = i_rElementName;
        const sal_Int32 nSeparatorPos = i_rElementName.indexOf( ':' );
        if ( nSeparatorPos > -1 )
        {
            o_rNamespace = i_rElementName.copy( 0, nSeparatorPos );
            o_rLocalName = i_rElementName.copy( nSeparatorPos + 1 );
        }

        OSL_ENSURE( o_rNamespace.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "config" ) ), "SettingsImport::split: unexpected namespace!" );
            // our recovery file is kind of hand-made, so there shouldn't be anything else than "config".
            // If there is, then just ignore it ...
    }

    //====================================================================
    //= IgnoringSettingsImport
    //====================================================================
    //--------------------------------------------------------------------
    ::rtl::Reference< SettingsImport > IgnoringSettingsImport::nextState( const ::rtl::OUString& i_rElementName )
    {
        (void)i_rElementName;
        return this;
    }

    //====================================================================
    //= OfficeSettingsImport
    //====================================================================
    //--------------------------------------------------------------------
    OfficeSettingsImport::OfficeSettingsImport( ::comphelper::NamedValueCollection& o_rSettings )
        :m_rSettings( o_rSettings )
    {
    }

    //--------------------------------------------------------------------
    OfficeSettingsImport::~OfficeSettingsImport()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::Reference< SettingsImport > OfficeSettingsImport::nextState( const ::rtl::OUString& i_rElementName )
    {
        // separate the namespace part from the element name
        ::rtl::OUString sNamespace;
        ::rtl::OUString sLocalName;
        split( i_rElementName, sNamespace, sLocalName );

        if ( sLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "config-item-set" ) ) )
            return new ConfigItemSetImport( m_rSettings );

#if OSL_DEBUG_LEVEL > 0
        ::rtl::OString sMessage( "unknown (or unsupported at this place) element name '" );
        sMessage += ::rtl::OUStringToOString( i_rElementName, RTL_TEXTENCODING_UTF8 );
        sMessage += "', ignoring";
        OSL_FAIL( sMessage.getStr() );
#endif
        return new IgnoringSettingsImport;
    }

    //====================================================================
    //= ConfigItemImport
    //====================================================================
    //--------------------------------------------------------------------
    ConfigItemImport::ConfigItemImport( ::comphelper::NamedValueCollection& o_rSettings )
        :m_rSettings( o_rSettings )
    {
    }

    //--------------------------------------------------------------------
    ConfigItemImport::~ConfigItemImport()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::Reference< SettingsImport > ConfigItemImport::nextState( const ::rtl::OUString& i_rElementName )
    {
        OSL_FAIL( "ConfigItemImport::nextState: unexpected: this class is responsible for child-less items only!" );
        (void)i_rElementName;
        return new IgnoringSettingsImport;
    }

    //--------------------------------------------------------------------
    void ConfigItemImport::endElement()
    {
        SettingsImport::endElement();

        const ::rtl::OUString sItemName( getItemName() );
        ENSURE_OR_RETURN_VOID( sItemName.getLength(), "no item name -> no item value" );
        Any aValue;
        getItemValue( aValue );
        m_rSettings.put( sItemName, aValue );
    }

    //--------------------------------------------------------------------
    void ConfigItemImport::getItemValue( ::com::sun::star::uno::Any& o_rValue ) const
    {
        o_rValue.clear();

        // the characters building up th evalue
        ::rtl::OUStringBuffer aCharacters( getAccumulatedCharacters() );
        const ::rtl::OUString sValue = aCharacters.makeStringAndClear();

        const ::rtl::OUString& rItemType( getItemType() );
        ENSURE_OR_RETURN_VOID( rItemType.getLength(), "no item type -> no item value" );

        if ( ::xmloff::token::IsXMLToken( rItemType, ::xmloff::token::XML_INT ) )
        {
            sal_Int32 nValue(0);
            if ( SvXMLUnitConverter::convertNumber( nValue, sValue ) )
                o_rValue <<= nValue;
            else
            {
                OSL_FAIL( "ConfigItemImport::getItemValue: could not convert an int value!" );
            }
        }
        else if ( ::xmloff::token::IsXMLToken( rItemType, ::xmloff::token::XML_BOOLEAN ) )
        {
            bool nValue( sal_False );
            if ( SvXMLUnitConverter::convertBool( nValue, sValue ) )
                o_rValue <<= nValue;
            else
            {
                OSL_FAIL( "ConfigItemImport::getItemValue: could not convert a boolean value!" );
            }
        }
        else if ( ::xmloff::token::IsXMLToken( rItemType, ::xmloff::token::XML_STRING ) )
        {
            o_rValue <<= sValue;
        }
#if OSL_DEBUG_LEVEL > 0
        else
        {
            ::rtl::OString sMessage( "ConfigItemImport::getItemValue: unsupported item type '" );
            sMessage += ::rtl::OUStringToOString( rItemType, RTL_TEXTENCODING_UTF8 );
            sMessage += "', ignoring";
            OSL_FAIL( sMessage.getStr() );
        }
#endif
    }

    //====================================================================
    //= ConfigItemSetImport
    //====================================================================
    //--------------------------------------------------------------------
    ConfigItemSetImport::ConfigItemSetImport( ::comphelper::NamedValueCollection& o_rSettings )
        :ConfigItemImport( o_rSettings )
    {
    }

    //--------------------------------------------------------------------
    ConfigItemSetImport::~ConfigItemSetImport()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::Reference< SettingsImport > ConfigItemSetImport::nextState( const ::rtl::OUString& i_rElementName )
    {
        // separate the namespace part from the element name
        ::rtl::OUString sNamespace;
        ::rtl::OUString sLocalName;
        split( i_rElementName, sNamespace, sLocalName );

        if ( sLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "config-item-set" ) ) )
            return new ConfigItemSetImport( m_aChildSettings );
        if ( sLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "config-item" ) ) )
            return new ConfigItemImport( m_aChildSettings );

#if OSL_DEBUG_LEVEL > 0
        ::rtl::OString sMessage( "unknown element name '" );
        sMessage += ::rtl::OUStringToOString( i_rElementName, RTL_TEXTENCODING_UTF8 );
        sMessage += "', ignoring";
        OSL_FAIL( sMessage.getStr() );
#endif
        return new IgnoringSettingsImport;
    }

    //--------------------------------------------------------------------
    void ConfigItemSetImport::getItemValue( Any& o_rValue ) const
    {
        o_rValue <<= m_aChildSettings.getPropertyValues();
    }

//........................................................................
} // namespace dbaccess
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

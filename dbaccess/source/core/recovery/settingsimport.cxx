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

#include "settingsimport.hxx"

#include <tools/diagnose_ex.h>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>

namespace dbaccess
{

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

    // SettingsImport
    SettingsImport::SettingsImport()
        :m_refCount( 0 )
    {
    }

    SettingsImport::~SettingsImport()
    {
    }

    oslInterlockedCount SAL_CALL SettingsImport::acquire()
    {
        return osl_atomic_increment( &m_refCount );
    }

    oslInterlockedCount SAL_CALL SettingsImport::release()
    {
        oslInterlockedCount newCount = osl_atomic_decrement( &m_refCount );
        if ( newCount == 0 )
            delete this;
        return newCount;
    }

    void SettingsImport::startElement( const Reference< XAttributeList >& i_rAttributes )
    {
        // find the name of the setting
        if ( i_rAttributes.is() )
        {
            m_sItemName = i_rAttributes->getValueByName( "config:name" );
            m_sItemType = i_rAttributes->getValueByName( "config:type" );
        }
    }

    void SettingsImport::endElement()
    {
    }

    void SettingsImport::characters( const OUString& i_rCharacters )
    {
        m_aCharacters.append( i_rCharacters );
    }

    void SettingsImport::split( const OUString& i_rElementName, OUString& o_rNamespace, OUString& o_rLocalName )
    {
        o_rNamespace = OUString();
        o_rLocalName = i_rElementName;
        const sal_Int32 nSeparatorPos = i_rElementName.indexOf( ':' );
        if ( nSeparatorPos > -1 )
        {
            o_rNamespace = i_rElementName.copy( 0, nSeparatorPos );
            o_rLocalName = i_rElementName.copy( nSeparatorPos + 1 );
        }

        OSL_ENSURE( o_rNamespace == "config", "SettingsImport::split: unexpected namespace!" );
            // our recovery file is kind of hand-made, so there shouldn't be anything else than "config".
            // If there is, then just ignore it ...
    }

    // IgnoringSettingsImport
    ::rtl::Reference< SettingsImport > IgnoringSettingsImport::nextState( const OUString& i_rElementName )
    {
        (void)i_rElementName;
        return this;
    }

    // OfficeSettingsImport
    OfficeSettingsImport::OfficeSettingsImport( ::comphelper::NamedValueCollection& o_rSettings )
        :m_rSettings( o_rSettings )
    {
    }

    OfficeSettingsImport::~OfficeSettingsImport()
    {
    }

    ::rtl::Reference< SettingsImport > OfficeSettingsImport::nextState( const OUString& i_rElementName )
    {
        // separate the namespace part from the element name
        OUString sNamespace;
        OUString sLocalName;
        split( i_rElementName, sNamespace, sLocalName );

        if ( sLocalName == "config-item-set" )
            return new ConfigItemSetImport( m_rSettings );

#if OSL_DEBUG_LEVEL > 0
        OString sMessage( "unknown (or unsupported at this place) element name '" );
        sMessage += OUStringToOString( i_rElementName, RTL_TEXTENCODING_UTF8 );
        sMessage += "', ignoring";
        OSL_FAIL( sMessage.getStr() );
#endif
        return new IgnoringSettingsImport;
    }

    // ConfigItemImport
    ConfigItemImport::ConfigItemImport( ::comphelper::NamedValueCollection& o_rSettings )
        :m_rSettings( o_rSettings )
    {
    }

    ConfigItemImport::~ConfigItemImport()
    {
    }

    ::rtl::Reference< SettingsImport > ConfigItemImport::nextState( const OUString& i_rElementName )
    {
        OSL_FAIL( "ConfigItemImport::nextState: unexpected: this class is responsible for child-less items only!" );
        (void)i_rElementName;
        return new IgnoringSettingsImport;
    }

    void ConfigItemImport::endElement()
    {
        SettingsImport::endElement();

        const OUString sItemName( getItemName() );
        ENSURE_OR_RETURN_VOID( !sItemName.isEmpty(), "no item name -> no item value" );
        Any aValue;
        getItemValue( aValue );
        m_rSettings.put( sItemName, aValue );
    }

    void ConfigItemImport::getItemValue( ::com::sun::star::uno::Any& o_rValue ) const
    {
        o_rValue.clear();

        // the characters building up th evalue
        OUStringBuffer aCharacters( getAccumulatedCharacters() );
        const OUString sValue = aCharacters.makeStringAndClear();

        const OUString& rItemType( getItemType() );
        ENSURE_OR_RETURN_VOID( !rItemType.isEmpty(), "no item type -> no item value" );

        if ( ::xmloff::token::IsXMLToken( rItemType, ::xmloff::token::XML_INT ) )
        {
            sal_Int32 nValue(0);
            if (::sax::Converter::convertNumber( nValue, sValue ))
            {
                o_rValue <<= nValue;
            }
            else
            {
                OSL_FAIL( "ConfigItemImport::getItemValue: could not convert an int value!" );
            }
        }
        else if ( ::xmloff::token::IsXMLToken( rItemType, ::xmloff::token::XML_BOOLEAN ) )
        {
            bool bValue(false);
            if (::sax::Converter::convertBool( bValue, sValue ))
            {
                o_rValue <<= bValue;
            }
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
            OString sMessage( "ConfigItemImport::getItemValue: unsupported item type '" );
            sMessage += OUStringToOString( rItemType, RTL_TEXTENCODING_UTF8 );
            sMessage += "', ignoring";
            OSL_FAIL( sMessage.getStr() );
        }
#endif
    }

    // ConfigItemSetImport
    ConfigItemSetImport::ConfigItemSetImport( ::comphelper::NamedValueCollection& o_rSettings )
        :ConfigItemImport( o_rSettings )
    {
    }

    ConfigItemSetImport::~ConfigItemSetImport()
    {
    }

    ::rtl::Reference< SettingsImport > ConfigItemSetImport::nextState( const OUString& i_rElementName )
    {
        // separate the namespace part from the element name
        OUString sNamespace;
        OUString sLocalName;
        split( i_rElementName, sNamespace, sLocalName );

        if ( sLocalName == "config-item-set" )
            return new ConfigItemSetImport( m_aChildSettings );
        if ( sLocalName == "config-item" )
            return new ConfigItemImport( m_aChildSettings );

#if OSL_DEBUG_LEVEL > 0
        OString sMessage( "unknown element name '" );
        sMessage += OUStringToOString( i_rElementName, RTL_TEXTENCODING_UTF8 );
        sMessage += "', ignoring";
        OSL_FAIL( sMessage.getStr() );
#endif
        return new IgnoringSettingsImport;
    }

    void ConfigItemSetImport::getItemValue( Any& o_rValue ) const
    {
        o_rValue <<= m_aChildSettings.getPropertyValues();
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef SETTINGSIMPORT_HXX
#define SETTINGSIMPORT_HXX

#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

namespace dbaccess
{

    // SettingsImport
    /** a simplified version of xmloff/DocumentSettingsContext

        It would be nice if the DocumentSettingsContext would not be that tightly interwoven with the SvXMLImport
        class, so we could re-use it here ...
    */
    class SettingsImport : public ::rtl::IReference
    {
    public:
        SettingsImport();

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

        // own overriables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        ) = 0;
        virtual void startElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& i_rAttributes
        );
        virtual void endElement();
        virtual void characters( const OUString& i_rCharacters );

    protected:
        virtual ~SettingsImport();

    protected:
        static void split( const OUString& i_rElementName, OUString& o_rNamespace, OUString& o_rLocalName );

    protected:
        const OUString&          getItemName() const                 { return m_sItemName; }
        const OUString&          getItemType() const                 { return m_sItemType; }
        const OUStringBuffer&    getAccumulatedCharacters() const    { return m_aCharacters; }

    private:
        oslInterlockedCount     m_refCount;
        // value of the config:name attribute, if any
        OUString         m_sItemName;
        // value of the config:type attribute, if any
        OUString         m_sItemType;
        // accumulated characters, if any
        OUStringBuffer   m_aCharacters;
    };

    // IgnoringSettingsImport
    class IgnoringSettingsImport : public SettingsImport
    {
    public:
        IgnoringSettingsImport()
        {
        }

        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        );

    private:
        ~IgnoringSettingsImport()
        {
        }
    };

    // OfficeSettingsImport
    class OfficeSettingsImport : public SettingsImport
    {
    public:
        OfficeSettingsImport( ::comphelper::NamedValueCollection& o_rSettings );

        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        );

    protected:
        ~OfficeSettingsImport();

    private:
        // the settings collection to which |this| will contribute a single setting
        ::comphelper::NamedValueCollection& m_rSettings;
    };

    // ConfigItemSetImport
    class ConfigItemImport : public SettingsImport
    {
    public:
        ConfigItemImport( ::comphelper::NamedValueCollection& o_rSettings );

    protected:
        ~ConfigItemImport();

    public:
        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        );
        virtual void endElement();

    protected:
        // own overridables
        /// retrieves the value represented by the element
        virtual void getItemValue( ::com::sun::star::uno::Any& o_rValue ) const;

    private:
        // the settings collection to which |this| will contribute a single setting
        ::comphelper::NamedValueCollection& m_rSettings;
    };

    // ConfigItemSetImport
    class ConfigItemSetImport : public ConfigItemImport
    {
    public:
        ConfigItemSetImport( ::comphelper::NamedValueCollection& o_rSettings );

    protected:
        ~ConfigItemSetImport();

    public:
        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        );

    protected:
        // ConfigItemImport overridables
        virtual void getItemValue( ::com::sun::star::uno::Any& o_rValue ) const;

    private:
        /// the settings represented by our child elements
        ::comphelper::NamedValueCollection  m_aChildSettings;
    };

} // namespace dbaccess

#endif // SETTINGSIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

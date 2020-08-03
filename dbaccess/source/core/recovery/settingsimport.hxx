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

#pragma once

#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace dbaccess
{

    // SettingsImport
    /** a simplified version of xmloff/DocumentSettingsContext

        It would be nice if the DocumentSettingsContext would not be that tightly interwoven with the SvXMLImport
        class, so we could re-use it here ...
    */
    class SettingsImport : public salhelper::SimpleReferenceObject
    {
    public:
        SettingsImport();

        // own overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        ) = 0;
        void startElement(
            const css::uno::Reference< css::xml::sax::XAttributeList >& i_rAttributes
        );
        virtual void endElement();
        void characters( const OUString& i_rCharacters );

    protected:
        virtual ~SettingsImport() override;

    protected:
        static void split( const OUString& i_rElementName, OUString& o_rNamespace, OUString& o_rLocalName );

    protected:
        const OUString&          getItemName() const                 { return m_sItemName; }
        const OUString&          getItemType() const                 { return m_sItemType; }
        const OUStringBuffer&    getAccumulatedCharacters() const    { return m_aCharacters; }

    private:
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
        ) override;

    private:
        virtual ~IgnoringSettingsImport() override
        {
        }
    };

    // OfficeSettingsImport
    class OfficeSettingsImport : public SettingsImport
    {
    public:
        explicit OfficeSettingsImport( ::comphelper::NamedValueCollection& o_rSettings );

        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        ) override;

    protected:
        virtual ~OfficeSettingsImport() override;

    private:
        // the settings collection to which |this| will contribute a single setting
        ::comphelper::NamedValueCollection& m_rSettings;
    };

    // ConfigItemSetImport
    class ConfigItemImport : public SettingsImport
    {
    public:
        explicit ConfigItemImport( ::comphelper::NamedValueCollection& o_rSettings );

    protected:
        virtual ~ConfigItemImport() override;

    public:
        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        ) override;
        virtual void endElement() override;

    protected:
        // own overridables
        /// retrieves the value represented by the element
        virtual void getItemValue( css::uno::Any& o_rValue ) const;

    private:
        // the settings collection to which |this| will contribute a single setting
        ::comphelper::NamedValueCollection& m_rSettings;
    };

    // ConfigItemSetImport
    class ConfigItemSetImport : public ConfigItemImport
    {
    public:
        explicit ConfigItemSetImport( ::comphelper::NamedValueCollection& o_rSettings );

    protected:
        virtual ~ConfigItemSetImport() override;

    public:
        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const OUString& i_rElementName
        ) override;

    protected:
        // ConfigItemImport overridables
        virtual void getItemValue( css::uno::Any& o_rValue ) const override;

    private:
        /// the settings represented by our child elements
        ::comphelper::NamedValueCollection  m_aChildSettings;
    };

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

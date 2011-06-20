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

#ifndef SETTINGSIMPORT_HXX
#define SETTINGSIMPORT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/xml/sax/XAttributeList.hpp>
/** === end UNO includes === **/

#include <comphelper/namedvaluecollection.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= SettingsImport
    //====================================================================
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
            const ::rtl::OUString& i_rElementName
        ) = 0;
        virtual void startElement(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& i_rAttributes
        );
        virtual void endElement();
        virtual void characters( const ::rtl::OUString& i_rCharacters );

    protected:
        virtual ~SettingsImport();

    protected:
        static void split( const ::rtl::OUString& i_rElementName, ::rtl::OUString& o_rNamespace, ::rtl::OUString& o_rLocalName );

    protected:
        const ::rtl::OUString&          getItemName() const                 { return m_sItemName; }
        const ::rtl::OUString&          getItemType() const                 { return m_sItemType; }
        const ::rtl::OUStringBuffer&    getAccumulatedCharacters() const    { return m_aCharacters; }

    private:
        oslInterlockedCount     m_refCount;
        // value of the config:name attribute, if any
        ::rtl::OUString         m_sItemName;
        // value of the config:type attribute, if any
        ::rtl::OUString         m_sItemType;
        // accumulated characters, if any
        ::rtl::OUStringBuffer   m_aCharacters;
    };

    //====================================================================
    //= IgnoringSettingsImport
    //====================================================================
    class IgnoringSettingsImport : public SettingsImport
    {
    public:
        IgnoringSettingsImport()
        {
        }

        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const ::rtl::OUString& i_rElementName
        );

    private:
        ~IgnoringSettingsImport()
        {
        }
    };

    //====================================================================
    //= OfficeSettingsImport
    //====================================================================
    class OfficeSettingsImport : public SettingsImport
    {
    public:
        OfficeSettingsImport( ::comphelper::NamedValueCollection& o_rSettings );

        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const ::rtl::OUString& i_rElementName
        );

    protected:
        ~OfficeSettingsImport();

    private:
        // the settings collection to which |this| will contribute a single setting
        ::comphelper::NamedValueCollection& m_rSettings;
    };

    //====================================================================
    //= ConfigItemSetImport
    //====================================================================
    class ConfigItemImport : public SettingsImport
    {
    public:
        ConfigItemImport( ::comphelper::NamedValueCollection& o_rSettings );

    protected:
        ~ConfigItemImport();

    public:
        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const ::rtl::OUString& i_rElementName
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

    //====================================================================
    //= ConfigItemSetImport
    //====================================================================
    class ConfigItemSetImport : public ConfigItemImport
    {
    public:
        ConfigItemSetImport( ::comphelper::NamedValueCollection& o_rSettings );

    protected:
        ~ConfigItemSetImport();

    public:
        // SettingsImport overridables
        virtual ::rtl::Reference< SettingsImport >  nextState(
            const ::rtl::OUString& i_rElementName
        );

    protected:
        // ConfigItemImport overridables
        virtual void getItemValue( ::com::sun::star::uno::Any& o_rValue ) const;

    private:
        /// the settings represented by our child elements
        ::comphelper::NamedValueCollection  m_aChildSettings;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // SETTINGSIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <textconnectionsettings.hxx>
#include <unoadmin.hxx>
#include <stringconstants.hxx>
#include <propertystorage.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/XTextConnectionSettings.hpp>

#include <comphelper/proparrhlp.hxx>
#include <connectivity/CommonTools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/implbase.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::cpo::uno::Any;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::cpo::uno::Sequence;
    using ::com::sun::star::beans::Property;

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    // OTextConnectionSettingsDialog

    namespace {

    class OTextConnectionSettingsDialog;

    }

    typedef ::cppu::ImplInheritanceHelper<   ODatabaseAdministrationDialog
                                         ,   css::sdb::XTextConnectionSettings
                                         >   OTextConnectionSettingsDialog_BASE;
    typedef ::comphelper::OPropertyArrayUsageHelper< OTextConnectionSettingsDialog >    OTextConnectionSettingsDialog_PBASE;

    namespace {

    class OTextConnectionSettingsDialog
            :public OTextConnectionSettingsDialog_BASE
            ,public OTextConnectionSettingsDialog_PBASE
    {
        PropertyValues  m_aPropertyValues;

    public:
        explicit OTextConnectionSettingsDialog( const Reference<XComponentContext>& _rContext );

        virtual cpo::uno::Sequence<sal_Int8> getImplementationId() override;

        DECLARE_SERVICE_INFO();
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;


        virtual void setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) override;
        virtual bool convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue) override;
        virtual void getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const override;

        // Overrides to resolve inheritance ambiguity
        virtual void setPropertyValue(const OUString& p1, const cpo::uno::Any& p2) override
            { ODatabaseAdministrationDialog::setPropertyValue(p1, p2); }
        virtual cpo::uno::Any getPropertyValue(const OUString& p1) override
            { return ODatabaseAdministrationDialog::getPropertyValue(p1); }
        virtual void addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
            { ODatabaseAdministrationDialog::addPropertyChangeListener(p1, p2); }
        virtual void removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
            { ODatabaseAdministrationDialog::removePropertyChangeListener(p1, p2); }
        virtual void addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
            { ODatabaseAdministrationDialog::addVetoableChangeListener(p1, p2); }
        virtual void removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
            { ODatabaseAdministrationDialog::removeVetoableChangeListener(p1, p2); }
        virtual void setTitle(const OUString& p1) override
            { ODatabaseAdministrationDialog::setTitle(p1); }
        virtual sal_Int16 execute() override
            { return ODatabaseAdministrationDialog::execute(); }

    protected:
        // OGenericUnoDialog overridables
        virtual std::unique_ptr<weld::DialogController> createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
        using OTextConnectionSettingsDialog_BASE::getFastPropertyValue;
    };

    }

    // OTextConnectionSettingsDialog
    OTextConnectionSettingsDialog::OTextConnectionSettingsDialog( const Reference<XComponentContext>& _rContext )
        :OTextConnectionSettingsDialog_BASE( _rContext )
    {
        TextConnectionSettingsDialog::bindItemStorages( *m_pDatasourceItems, m_aPropertyValues );
    }

    cpo::uno::Sequence<sal_Int8>
    OTextConnectionSettingsDialog::getImplementationId()
    {
        return cpo::uno::Sequence<sal_Int8>();
    }

    OUString OTextConnectionSettingsDialog::getImplementationName()
    {
        return u"com.sun.star.comp.dbaccess.OTextConnectionSettingsDialog"_ustr;
    }
    bool OTextConnectionSettingsDialog::supportsService(const OUString& _rServiceName)
    {
        const cpo::uno::Sequence< OUString > aSupported(getSupportedServiceNames());
        for (const OUString& s : aSupported)
            if (s == _rServiceName)
                return true;

        return false;
    }
    cpo::uno::Sequence< OUString > OTextConnectionSettingsDialog::getSupportedServiceNames()
    {
        return { u"com.sun.star.sdb.TextConnectionSettings"_ustr };
    }

    Reference< XPropertySetInfo >  OTextConnectionSettingsDialog::getPropertySetInfo()
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    ::cppu::IPropertyArrayHelper& OTextConnectionSettingsDialog::getInfoHelper()
    {
        return *getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* OTextConnectionSettingsDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );

        // in addition to the properties registered by the base class, we have
        // more properties which are not even handled by the PropertyContainer implementation,
        // but whose values are stored in our item set
        sal_Int32 nProp = aProps.getLength();
        aProps.realloc( nProp + 6 );
        auto pProps = aProps.getArray();

        pProps[ nProp++ ] = Property(
            u"HeaderLine"_ustr,
            PROPERTY_ID_HEADER_LINE,
            ::cppu::UnoType< bool >::get(),
            PropertyAttribute::TRANSIENT
        );

        pProps[ nProp++ ] = Property(
            u"FieldDelimiter"_ustr,
            PROPERTY_ID_FIELD_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        pProps[ nProp++ ] = Property(
            u"StringDelimiter"_ustr,
            PROPERTY_ID_STRING_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        pProps[ nProp++ ] = Property(
            u"DecimalDelimiter"_ustr,
            PROPERTY_ID_DECIMAL_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        pProps[ nProp++ ] = Property(
            u"ThousandDelimiter"_ustr,
            PROPERTY_ID_THOUSAND_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        pProps[ nProp++ ] = Property(
            u"CharSet"_ustr,
            PROPERTY_ID_ENCODING,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    std::unique_ptr<weld::DialogController> OTextConnectionSettingsDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        return std::make_unique<TextConnectionSettingsDialog>(Application::GetFrameWeld(rParent), *m_pDatasourceItems);
    }

    void OTextConnectionSettingsDialog::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
    {
        PropertyValues::const_iterator pos = m_aPropertyValues.find( _nHandle );
        if ( pos != m_aPropertyValues.end() )
        {
            pos->second->setPropertyValue( _rValue );
        }
        else
        {
            OTextConnectionSettingsDialog::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }

    bool OTextConnectionSettingsDialog::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
    {
        bool bModified = false;

        PropertyValues::const_iterator pos = m_aPropertyValues.find( _nHandle );
        if ( pos != m_aPropertyValues.end() )
        {
            // we're lazy here ...
            _rConvertedValue = _rValue;
            pos->second->getPropertyValue( _rOldValue );
            bModified = true;
        }
        else
        {
            bModified = OTextConnectionSettingsDialog::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
        }

        return bModified;
    }

    void OTextConnectionSettingsDialog::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        PropertyValues::const_iterator pos = m_aPropertyValues.find( _nHandle );
        if ( pos != m_aPropertyValues.end() )
        {
            pos->second->getPropertyValue( _rValue );
        }
        else
        {
            OTextConnectionSettingsDialog::getFastPropertyValue( _rValue, _nHandle );
        }
    }

} // namespace dbaui

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_dbaccess_OTextConnectionSettingsDialog_get_implementation(
    css::uno::XComponentContext* context, cpo::uno::Sequence<cpo::uno::Any> const& )
{
    return cppu::acquire(static_cast<dbaui::ODatabaseAdministrationDialog*>(new ::dbaui::OTextConnectionSettingsDialog(context)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

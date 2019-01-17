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

#include <textconnectionsettings.hxx>
#include <uiservices.hxx>
#include <dbu_reghelper.hxx>
#include <apitools.hxx>
#include <unoadmin.hxx>
#include <stringconstants.hxx>
#include <propertystorage.hxx>

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/XTextConnectionSettings.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/proparrhlp.hxx>
#include <svtools/genericunodialog.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/implbase.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::lang::IllegalArgumentException;

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    // OTextConnectionSettingsDialog

    class OTextConnectionSettingsDialog;
    typedef ::cppu::ImplInheritanceHelper<   ODatabaseAdministrationDialog
                                         ,   css::sdb::XTextConnectionSettings
                                         >   OTextConnectionSettingsDialog_BASE;
    typedef ::comphelper::OPropertyArrayUsageHelper< OTextConnectionSettingsDialog >    OTextConnectionSettingsDialog_PBASE;

    class OTextConnectionSettingsDialog
            :public OTextConnectionSettingsDialog_BASE
            ,public OTextConnectionSettingsDialog_PBASE
            ,public ::cppu::WeakImplHelper< css::sdb::XTextConnectionSettings >
    {
        PropertyValues  m_aPropertyValues;

    protected:
        explicit OTextConnectionSettingsDialog( const Reference<XComponentContext>& _rContext );
        virtual ~OTextConnectionSettingsDialog() override;

    public:
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

        DECLARE_SERVICE_INFO();
        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_Static(  );
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static(  );
        static css::uno::Reference< css::uno::XInterface >
        Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);
        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) override;
        virtual sal_Bool SAL_CALL convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue) override;
        virtual void SAL_CALL getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const override;

        // Overrides to resolve inheritance ambiguity
        virtual void SAL_CALL setPropertyValue(const OUString& p1, const css::uno::Any& p2) override
            { ODatabaseAdministrationDialog::setPropertyValue(p1, p2); }
        virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& p1) override
            { return ODatabaseAdministrationDialog::getPropertyValue(p1); }
        virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
            { ODatabaseAdministrationDialog::addPropertyChangeListener(p1, p2); }
        virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
            { ODatabaseAdministrationDialog::removePropertyChangeListener(p1, p2); }
        virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
            { ODatabaseAdministrationDialog::addVetoableChangeListener(p1, p2); }
        virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
            { ODatabaseAdministrationDialog::removeVetoableChangeListener(p1, p2); }
        virtual void SAL_CALL setTitle(const OUString& p1) override
            { ODatabaseAdministrationDialog::setTitle(p1); }
        virtual sal_Int16 SAL_CALL execute() override
            { return ODatabaseAdministrationDialog::execute(); }

    protected:
        // OGenericUnoDialog overridables
        virtual svt::OGenericUnoDialog::Dialog createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
        using OTextConnectionSettingsDialog_BASE::getFastPropertyValue;
    };

    // OTextConnectionSettingsDialog
    OTextConnectionSettingsDialog::OTextConnectionSettingsDialog( const Reference<XComponentContext>& _rContext )
        :OTextConnectionSettingsDialog_BASE( _rContext )
    {
        TextConnectionSettingsDialog::bindItemStorages( *m_pDatasourceItems, m_aPropertyValues );
    }

    OTextConnectionSettingsDialog::~OTextConnectionSettingsDialog()
    {
    }

    css::uno::Sequence<sal_Int8>
    OTextConnectionSettingsDialog::getImplementationId()
    {
        return css::uno::Sequence<sal_Int8>();
    }

    IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(OTextConnectionSettingsDialog, "com.sun.star.comp.dbaccess.OTextConnectionSettingsDialog")
    IMPLEMENT_SERVICE_INFO_SUPPORTS(OTextConnectionSettingsDialog)
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(OTextConnectionSettingsDialog, "com.sun.star.sdb.TextConnectionSettings")

    css::uno::Reference< css::uno::XInterface >
        OTextConnectionSettingsDialog::Create(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new OTextConnectionSettingsDialog( comphelper::getComponentContext(_rxORB)));
    }

    Reference< XPropertySetInfo >  SAL_CALL OTextConnectionSettingsDialog::getPropertySetInfo()
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

        aProps[ nProp++ ] = Property(
            "HeaderLine",
            PROPERTY_ID_HEADER_LINE,
            ::cppu::UnoType< sal_Bool >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            "FieldDelimiter",
            PROPERTY_ID_FIELD_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            "StringDelimiter",
            PROPERTY_ID_STRING_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            "DecimalDelimiter",
            PROPERTY_ID_DECIMAL_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            "ThousandDelimiter",
            PROPERTY_ID_THOUSAND_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            "CharSet",
            PROPERTY_ID_ENCODING,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    svt::OGenericUnoDialog::Dialog OTextConnectionSettingsDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        return svt::OGenericUnoDialog::Dialog(std::make_unique<TextConnectionSettingsDialog>(Application::GetFrameWeld(rParent), *m_pDatasourceItems));
    }

    void SAL_CALL OTextConnectionSettingsDialog::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
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

    sal_Bool SAL_CALL OTextConnectionSettingsDialog::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
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

    void SAL_CALL OTextConnectionSettingsDialog::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
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

extern "C" void createRegistryInfo_OTextConnectionSettingsDialog()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OTextConnectionSettingsDialog > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

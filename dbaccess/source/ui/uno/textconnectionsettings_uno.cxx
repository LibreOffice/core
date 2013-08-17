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

#include "textconnectionsettings.hxx"
#include "dbu_reghelper.hxx"
#include "moduledbu.hxx"
#include "apitools.hxx"
#include "unoadmin.hxx"
#include "dbustrings.hrc"
#include "propertystorage.hxx"

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/XTextConnectionSettings.hpp>

#include <comphelper/processfactory.hxx>
#include <svtools/genericunodialog.hxx>
#include <cppuhelper/implbase1.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::lang::IllegalArgumentException;

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    // OTextConnectionSettingsDialog

    class OTextConnectionSettingsDialog;
    typedef ::cppu::ImplInheritanceHelper1  <   ODatabaseAdministrationDialog
                                            ,   ::com::sun::star::sdb::XTextConnectionSettings
                                            >   OTextConnectionSettingsDialog_BASE;
    typedef ::comphelper::OPropertyArrayUsageHelper< OTextConnectionSettingsDialog >    OTextConnectionSettingsDialog_PBASE;

    class OTextConnectionSettingsDialog
            :public OTextConnectionSettingsDialog_BASE
            ,public OTextConnectionSettingsDialog_PBASE
            ,public ::cppu::WeakImplHelper1< com::sun::star::sdb::XTextConnectionSettings >
    {
        OModuleClient   m_aModuleClient;
        PropertyValues  m_aPropertyValues;

    protected:
        OTextConnectionSettingsDialog( const Reference<XComponentContext>& _rContext );
        virtual ~OTextConnectionSettingsDialog();

    public:
        DECLARE_IMPLEMENTATION_ID( );
        DECLARE_SERVICE_INFO_STATIC( );
        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw(Exception);
        virtual sal_Bool SAL_CALL convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue) throw(IllegalArgumentException);
        virtual void SAL_CALL getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const;

        // Overrides to resolve inheritance ambiguity
        virtual void SAL_CALL setPropertyValue(const OUString& p1, const css::uno::Any& p2) throw (css::uno::RuntimeException)
            { ODatabaseAdministrationDialog::setPropertyValue(p1, p2); }
        virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& p1) throw (css::uno::RuntimeException)
            { return ODatabaseAdministrationDialog::getPropertyValue(p1); }
        virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw (css::uno::RuntimeException)
            { ODatabaseAdministrationDialog::addPropertyChangeListener(p1, p2); }
        virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw (css::uno::RuntimeException)
            { ODatabaseAdministrationDialog::removePropertyChangeListener(p1, p2); }
        virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw (css::uno::RuntimeException)
            { ODatabaseAdministrationDialog::addVetoableChangeListener(p1, p2); }
        virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw (css::uno::RuntimeException)
            { ODatabaseAdministrationDialog::removeVetoableChangeListener(p1, p2); }
        virtual void SAL_CALL setTitle(const OUString& p1) throw (css::uno::RuntimeException)
            { ODatabaseAdministrationDialog::setTitle(p1); }
        virtual sal_Int16 SAL_CALL execute() throw (css::uno::RuntimeException)
            { return ODatabaseAdministrationDialog::execute(); }

    protected:
        // OGenericUnoDialog overridables
        virtual Dialog* createDialog( Window* _pParent );
        virtual void implInitialize( const com::sun::star::uno::Any& _rValue );
    protected:
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

    IMPLEMENT_IMPLEMENTATION_ID( OTextConnectionSettingsDialog )

    IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(OTextConnectionSettingsDialog, "com.sun.star.comp.dbaccess.OTextConnectionSettingsDialog")
    IMPLEMENT_SERVICE_INFO_SUPPORTS(OTextConnectionSettingsDialog)
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(OTextConnectionSettingsDialog, "com.sun.star.sdb.TextConnectionSettings")

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL OTextConnectionSettingsDialog::Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new OTextConnectionSettingsDialog( comphelper::getComponentContext(_rxORB)));
    }

    Reference< XPropertySetInfo >  SAL_CALL OTextConnectionSettingsDialog::getPropertySetInfo() throw(RuntimeException)
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
            OUString( "HeaderLine" ),
            PROPERTY_ID_HEADER_LINE,
            ::cppu::UnoType< sal_Bool >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            OUString( "FieldDelimiter" ),
            PROPERTY_ID_FIELD_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            OUString( "StringDelimiter" ),
            PROPERTY_ID_STRING_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            OUString( "DecimalDelimiter" ),
            PROPERTY_ID_DECIMAL_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            OUString( "ThousandDelimiter" ),
            PROPERTY_ID_THOUSAND_DELIMITER,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            OUString( "CharSet" ),
            PROPERTY_ID_ENCODING,
            ::cppu::UnoType< OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    Dialog* OTextConnectionSettingsDialog::createDialog(Window* _pParent)
    {
        return new TextConnectionSettingsDialog( _pParent, *m_pDatasourceItems );
    }

    void OTextConnectionSettingsDialog::implInitialize(const Any& _rValue)
    {
        OTextConnectionSettingsDialog_BASE::implInitialize( _rValue );
    }

    void SAL_CALL OTextConnectionSettingsDialog::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw(Exception)
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

    sal_Bool SAL_CALL OTextConnectionSettingsDialog::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue) throw(IllegalArgumentException)
    {
        sal_Bool bModified = sal_False;

        PropertyValues::const_iterator pos = m_aPropertyValues.find( _nHandle );
        if ( pos != m_aPropertyValues.end() )
        {
            // we're lazy here ...
            _rConvertedValue = _rValue;
            pos->second->getPropertyValue( _rOldValue );
            bModified = sal_True;
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

extern "C" void SAL_CALL createRegistryInfo_OTextConnectionSettingsDialog()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OTextConnectionSettingsDialog > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

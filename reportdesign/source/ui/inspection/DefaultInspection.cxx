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
#include <DefaultInspection.hxx>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <strings.hrc>
#include <core_resource.hxx>
#include <helpids.h>
#include <cppuhelper/supportsservice.hxx>
#include <tools/debug.hxx>
#include <metadata.hxx>
#include <tools/urlobj.hxx>


namespace rptui
{
    OUString HelpIdUrl::getHelpURL( const OString& sHelpId )
    {
        OUString aTmp( OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8) );
        DBG_ASSERT( INetURLObject( aTmp ).GetProtocol() == INetProtocol::NotValid, "Wrong HelpId!" );
        return INET_HID_SCHEME + aTmp;
    }

    using namespace com::sun::star::uno;
    using namespace com::sun::star;
    using com::sun::star::inspection::PropertyCategoryDescriptor;


    //= DefaultComponentInspectorModel


    DefaultComponentInspectorModel::DefaultComponentInspectorModel( const Reference< XComponentContext >& _rxContext)
        :m_xContext( _rxContext )
        ,m_bConstructed( false )
        ,m_bHasHelpSection( false )
        ,m_bIsReadOnly(false)
        ,m_nMinHelpTextLines( 3 )
        ,m_nMaxHelpTextLines( 8 )
    {
    }

    DefaultComponentInspectorModel::~DefaultComponentInspectorModel()
    {
    }

    OUString SAL_CALL DefaultComponentInspectorModel::getImplementationName(  )
    {
        return "com.sun.star.comp.report.DefaultComponentInspectorModel";
    }

    sal_Bool SAL_CALL DefaultComponentInspectorModel::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService(this, ServiceName);
    }

    Sequence< OUString > SAL_CALL DefaultComponentInspectorModel::getSupportedServiceNames(  )
    {
        return { "com.sun.star.report.inspection.DefaultComponentInspectorModel" };
    }

    Sequence< Any > SAL_CALL DefaultComponentInspectorModel::getHandlerFactories()
    {
        // service names for all our handlers
        return Sequence<Any> {
            Any(OUString( "com.sun.star.report.inspection.ReportComponentHandler")),
            Any(OUString( "com.sun.star.form.inspection.EditPropertyHandler")),
            Any(OUString( "com.sun.star.report.inspection.DataProviderHandler")),
            Any(OUString( "com.sun.star.report.inspection.GeometryHandler"))
        };
    }

    sal_Bool SAL_CALL DefaultComponentInspectorModel::getHasHelpSection()
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return m_bHasHelpSection;
    }


    ::sal_Int32 SAL_CALL DefaultComponentInspectorModel::getMinHelpTextLines()
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return m_nMinHelpTextLines;
    }

    sal_Bool SAL_CALL DefaultComponentInspectorModel::getIsReadOnly()
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return m_bIsReadOnly;
    }

    void SAL_CALL DefaultComponentInspectorModel::setIsReadOnly( sal_Bool _isreadonly )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_bIsReadOnly = _isreadonly;
    }


    ::sal_Int32 SAL_CALL DefaultComponentInspectorModel::getMaxHelpTextLines()
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return m_nMaxHelpTextLines;
    }

    void SAL_CALL DefaultComponentInspectorModel::initialize( const Sequence< Any >& _arguments )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( m_bConstructed )
            throw ucb::AlreadyInitializedException();

        if ( !_arguments.hasElements() )
        {   // constructor: "createDefault()"
            m_bConstructed = true;
            return;
        }

        if ( _arguments.getLength() == 2 )
        {   // constructor: "createWithHelpSection( long, long )"
            sal_Int32 nMinHelpTextLines( 0 ), nMaxHelpTextLines( 0 );
            if ( !( _arguments[0] >>= nMinHelpTextLines ) || !( _arguments[1] >>= nMaxHelpTextLines ) )
                throw lang::IllegalArgumentException( OUString(), *this, 0 );
            createWithHelpSection( nMinHelpTextLines, nMaxHelpTextLines );
            return;
        }

        throw lang::IllegalArgumentException( OUString(), *this, 0 );
    }


    void DefaultComponentInspectorModel::createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines )
    {
        if ( ( _nMinHelpTextLines <= 0 ) || ( _nMaxHelpTextLines <= 0 ) || ( _nMinHelpTextLines > _nMaxHelpTextLines ) )
            throw lang::IllegalArgumentException( OUString(), *this, 0 );

        m_bHasHelpSection = true;
        m_nMinHelpTextLines = _nMinHelpTextLines;
        m_nMaxHelpTextLines = _nMaxHelpTextLines;
        m_bConstructed = true;
    }

    Sequence< PropertyCategoryDescriptor > SAL_CALL DefaultComponentInspectorModel::describeCategories(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const struct
        {
            const char* programmaticName;
            const char* uiNameResId;
            OString    helpId;
        } aCategories[] = {
            { "General",    RID_STR_PROPPAGE_DEFAULT,   HID_RPT_PROPDLG_TAB_GENERAL },
            { "Data",       RID_STR_PROPPAGE_DATA,      HID_RPT_PROPDLG_TAB_DATA },
        };

        const size_t nCategories = SAL_N_ELEMENTS( aCategories );
        Sequence< PropertyCategoryDescriptor > aReturn( nCategories );
        PropertyCategoryDescriptor* pReturn = aReturn.getArray();
        for ( size_t i=0; i<nCategories; ++i, ++pReturn )
        {
            pReturn->ProgrammaticName = OUString::createFromAscii( aCategories[i].programmaticName );
            pReturn->UIName = RptResId( aCategories[i].uiNameResId );
            pReturn->HelpURL = HelpIdUrl::getHelpURL( aCategories[i].helpId );
        }

        return aReturn;
    }


    ::sal_Int32 SAL_CALL DefaultComponentInspectorModel::getPropertyOrderIndex( const OUString& _rPropertyName )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        const sal_Int32 nPropertyId( OPropertyInfoService::getPropertyId( _rPropertyName ) );
        if ( nPropertyId != -1 )
            return nPropertyId;

        if ( !m_xComponent.is() )
            try
            {
                m_xComponent.set(m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.form.inspection.DefaultFormComponentInspectorModel",m_xContext),UNO_QUERY_THROW);
            }
            catch(const Exception &)
            {
                return 0;
            }

        return m_xComponent->getPropertyOrderIndex(_rPropertyName);
    }


} // namespace rptui

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
reportdesign_DefaultComponentInspectorModel_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new rptui::DefaultComponentInspectorModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

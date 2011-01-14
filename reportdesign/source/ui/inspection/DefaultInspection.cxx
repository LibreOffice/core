/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"
#include "DefaultInspection.hxx"
#include <comphelper/sequence.hxx>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#ifndef _REPORT_DLGRESID_HRC
#include <RptResId.hrc>
#endif
#include "ModuleHelper.hxx"
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#include <cppuhelper/implbase1.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include "metadata.hxx"
#include <tools/urlobj.hxx>

//........................................................................
namespace rptui
{
//........................................................................
    //------------------------------------------------------------------------
    ::rtl::OUString HelpIdUrl::getHelpURL( const rtl::OString& sHelpId )
    {
        ::rtl::OUStringBuffer aBuffer;
        ::rtl::OUString aTmp( sHelpId, sHelpId.getLength(), RTL_TEXTENCODING_UTF8 );
        DBG_ASSERT( INetURLObject( aTmp ).GetProtocol() == INET_PROT_NOT_VALID, "Wrong HelpId!" );
        aBuffer.appendAscii( INET_HID_SCHEME );
        aBuffer.append( aTmp.getStr() );
        return aBuffer.makeStringAndClear();
    }

    /** === begin UNO using === **/
    using namespace com::sun::star::uno;
    using namespace com::sun::star;
    using com::sun::star::inspection::PropertyCategoryDescriptor;
    /** === end UNO using === **/

    //====================================================================
    //= DefaultComponentInspectorModel
    //====================================================================
    DBG_NAME(DefaultComponentInspectorModel)
    //--------------------------------------------------------------------
    DefaultComponentInspectorModel::DefaultComponentInspectorModel( const Reference< XComponentContext >& _rxContext)
        :m_xContext( _rxContext )
        ,m_bConstructed( false )
        ,m_bHasHelpSection( false )
        ,m_bIsReadOnly(sal_False)
        ,m_nMinHelpTextLines( 3 )
        ,m_nMaxHelpTextLines( 8 )
        ,m_pInfoService(new OPropertyInfoService())
    {
        DBG_CTOR(DefaultComponentInspectorModel,NULL);
    }

    //------------------------------------------------------------------------
    DefaultComponentInspectorModel::~DefaultComponentInspectorModel()
    {
        DBG_DTOR(DefaultComponentInspectorModel,NULL);
    }

    //------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DefaultComponentInspectorModel::getImplementationName(  ) throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //------------------------------------------------------------------------
    sal_Bool SAL_CALL DefaultComponentInspectorModel::supportsService( const ::rtl::OUString& ServiceName ) throw(RuntimeException)
    {
        return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_static());
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DefaultComponentInspectorModel::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //------------------------------------------------------------------------
    ::rtl::OUString DefaultComponentInspectorModel::getImplementationName_Static(  ) throw(RuntimeException)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.DefaultComponentInspectorModel"));
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > DefaultComponentInspectorModel::getSupportedServiceNames_static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.inspection.DefaultComponentInspectorModel"));
        return aSupported;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL DefaultComponentInspectorModel::create( const Reference< XComponentContext >& _rxContext )
    {
        return *(new DefaultComponentInspectorModel( _rxContext ));
    }

    //--------------------------------------------------------------------
    Sequence< Any > SAL_CALL DefaultComponentInspectorModel::getHandlerFactories() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );


        // service names for all our handlers
        const struct
        {
            const sal_Char* serviceName;
        } aFactories[] = {

            { "com.sun.star.report.inspection.ReportComponentHandler"},
            { "com.sun.star.form.inspection.EditPropertyHandler"},
            { "com.sun.star.report.inspection.DataProviderHandler"},
            { "com.sun.star.report.inspection.GeometryHandler"}

            // generic virtual edit properties

        };

        const size_t nFactories = sizeof( aFactories ) / sizeof( aFactories[ 0 ] );
        Sequence< Any > aReturn( nFactories );
        Any* pReturn = aReturn.getArray();
        for ( size_t i = 0; i < nFactories; ++i )
        {
            *pReturn++ <<= ::rtl::OUString::createFromAscii( aFactories[i].serviceName );
        }

        return aReturn;
    }
    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL DefaultComponentInspectorModel::getHasHelpSection() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return m_bHasHelpSection;
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultComponentInspectorModel::getMinHelpTextLines() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return m_nMinHelpTextLines;
    }
    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL DefaultComponentInspectorModel::getIsReadOnly() throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return m_bIsReadOnly;
    }
    //--------------------------------------------------------------------
    void SAL_CALL DefaultComponentInspectorModel::setIsReadOnly( ::sal_Bool _isreadonly ) throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_bIsReadOnly = _isreadonly;
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultComponentInspectorModel::getMaxHelpTextLines() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return m_nMaxHelpTextLines;
    }
    //--------------------------------------------------------------------
    void SAL_CALL DefaultComponentInspectorModel::initialize( const Sequence< Any >& _arguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( m_bConstructed )
            throw ucb::AlreadyInitializedException();

        if ( !_arguments.hasElements() )
        {   // constructor: "createDefault()"
            createDefault();
            return;
        }

        sal_Int32 nMinHelpTextLines( 0 ), nMaxHelpTextLines( 0 );
        if ( _arguments.getLength() == 2 )
        {   // constructor: "createWithHelpSection( long, long )"
            if ( !( _arguments[0] >>= nMinHelpTextLines ) || !( _arguments[1] >>= nMaxHelpTextLines ) )
                throw lang::IllegalArgumentException( ::rtl::OUString(), *this, 0 );
            createWithHelpSection( nMinHelpTextLines, nMaxHelpTextLines );
            return;
        }

        throw lang::IllegalArgumentException( ::rtl::OUString(), *this, 0 );
    }

    //--------------------------------------------------------------------
    void DefaultComponentInspectorModel::createDefault()
    {
        m_bConstructed = true;
    }
    //--------------------------------------------------------------------
    void DefaultComponentInspectorModel::createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines )
    {
        if ( ( _nMinHelpTextLines <= 0 ) || ( _nMaxHelpTextLines <= 0 ) || ( _nMinHelpTextLines > _nMaxHelpTextLines ) )
            throw lang::IllegalArgumentException( ::rtl::OUString(), *this, 0 );

        m_bHasHelpSection = true;
        m_nMinHelpTextLines = _nMinHelpTextLines;
        m_nMaxHelpTextLines = _nMaxHelpTextLines;
        m_bConstructed = true;
    }
    //--------------------------------------------------------------------
    Sequence< PropertyCategoryDescriptor > SAL_CALL DefaultComponentInspectorModel::describeCategories(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const struct
        {
            const sal_Char* programmaticName;
            sal_uInt16          uiNameResId;
            rtl::OString    helpId;
        } aCategories[] = {
            { "General",    RID_STR_PROPPAGE_DEFAULT,   HID_RPT_PROPDLG_TAB_GENERAL },
            { "Data",       RID_STR_PROPPAGE_DATA,      HID_RPT_PROPDLG_TAB_DATA },
        };

        const size_t nCategories = sizeof( aCategories ) / sizeof( aCategories[0] );
        Sequence< PropertyCategoryDescriptor > aReturn( nCategories );
        PropertyCategoryDescriptor* pReturn = aReturn.getArray();
        for ( size_t i=0; i<nCategories; ++i, ++pReturn )
        {
            pReturn->ProgrammaticName = ::rtl::OUString::createFromAscii( aCategories[i].programmaticName );
            pReturn->UIName = String( ModuleRes( aCategories[i].uiNameResId ) );
            pReturn->HelpURL = HelpIdUrl::getHelpURL( aCategories[i].helpId );
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL DefaultComponentInspectorModel::getPropertyOrderIndex( const ::rtl::OUString& _rPropertyName ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        const sal_Int32 nPropertyId( m_pInfoService->getPropertyId( _rPropertyName ) );
        if ( nPropertyId != -1 )
            return nPropertyId;

        if ( !m_xComponent.is() )
            try
            {
                m_xComponent.set(m_xContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.inspection.DefaultFormComponentInspectorModel")),m_xContext),UNO_QUERY_THROW);
            }
            catch(Exception)
            {
                return 0;
            }

        return m_xComponent->getPropertyOrderIndex(_rPropertyName);
    }

//........................................................................
} // namespace rptui
//........................................................................


/*************************************************************************
 *
 *  $RCSfile: commanddefinition.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_
#define _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_

#ifndef _DBA_CORE_COMMANDBASE_HXX_
#include "commandbase.hxx"
#endif
#ifndef _DBA_CORE_CONTAINERELEMENT_HXX_
#include "containerelement.hxx"
#endif
#ifndef _DBA_CORE_SIMPLEPROPERTYCONTAINER_HXX_
#include "simplepropertycontainer.hxx"
#endif
#ifndef _DBA_CORE_CONFIGURATIONFLUSHABLE_HXX_
#include "configurationflushable.hxx"
#endif
#ifndef _DBA_REGISTRATION_HELPER_HXX_
#include "registrationhelper.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _UNOTOOLS_PROPERTY_ARRAY_HELPER_HXX_
#include <unotools/proparrhlp.hxx>
#endif

//==========================================================================
//= OCommandDefinition - a database "document" which describes a query
//==========================================================================

typedef ::cppu::WeakImplHelper2<
                    ::com::sun::star::lang::XUnoTunnel,
                    ::com::sun::star::lang::XServiceInfo
                    > OCommandDefinition_Base;

class OCommandDefinition    :public OCommandDefinition_Base
                            ,public OCommandBase
                            ,public OContainerElement
                            ,public OMutexAndBroadcastHelper
                            ,public OSimplePropertyContainer
                            ,public OConfigurationFlushable
                            ,public ::utl::OPropertyArrayUsageHelper< OCommandDefinition >
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xContainer;

private:
    OCommandDefinition();

protected:
    ~OCommandDefinition();

public:
    OCommandDefinition(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer,
            const ::rtl::OUString& _rElementName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rxConfigurationRoot
        );

// ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw(::com::sun::star::uno::RuntimeException) { OCommandDefinition_Base::acquire(); }
    virtual void SAL_CALL release(  ) throw(::com::sun::star::uno::RuntimeException) { OCommandDefinition_Base::release(); }

// ::com::sun::star::lang::XServiceInfo
    DECLARE_SERVICE_INFO();

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

// ::com::sun::star::lang::XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

// OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// OContainerElement
    virtual void        inserted(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer,
        const ::rtl::OUString& _rElementName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rxConfigRoot);

    virtual void        removed();

    virtual sal_Bool    isContainerElement() const { return m_xConfigurationNode.is(); }

protected:
// OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

// OConfigurationFlushable
    void flush_NoBroadcast(  ) throw(::com::sun::star::uno::RuntimeException);

// initialize the members from the current configuration node
    void initializeFromConfiguration();

private:
    // helper
    void registerProperties();
};

#endif // _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_



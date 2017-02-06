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

#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/component.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <rtl/instance.hxx>
#include <comphelper/processfactory.hxx>

#include <svl/solar.hrc>

#include "xmlfiltersettingsdialog.hxx"

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::frame;


class XMLFilterDialogComponentBase
{
protected:
    ::osl::Mutex maMutex;
};


class XMLFilterDialogComponent :    public XMLFilterDialogComponentBase,
                                    public OComponentHelper,
                                    public css::ui::dialogs::XExecutableDialog,
                                    public XServiceInfo,
                                    public XInitialization,
                                    public XTerminateListener
{
public:
    explicit XMLFilterDialogComponent( const Reference< XComponentContext >& rxContext );

protected:
    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) override;
    virtual Any SAL_CALL queryAggregation( Type const & rType ) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XTypeProvider
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual Sequence< Type > SAL_CALL getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;
    virtual sal_Int16 SAL_CALL execute(  ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const EventObject& Event ) override;
    virtual void SAL_CALL notifyTermination( const EventObject& Event ) override;
    virtual void SAL_CALL disposing( const EventObject& Source ) override;

    /** Called in dispose method after the listeners were notified.
    */
    virtual void SAL_CALL disposing() override;

private:
    css::uno::Reference<css::awt::XWindow>   mxParent;  /// parent window
    css::uno::Reference< XComponentContext > mxContext;

    VclPtr<XMLFilterSettingsDialog>          mpDialog;
};

XMLFilterDialogComponent::XMLFilterDialogComponent( const css::uno::Reference< XComponentContext >& rxContext ) :
    OComponentHelper( maMutex ),
    mxContext( rxContext ),
    mpDialog( nullptr )
{
    Reference< XDesktop2 > xDesktop = Desktop::create( rxContext );
    Reference< XTerminateListener > xListener( this );
    xDesktop->addTerminateListener( xListener );
}

// XInterface
Any SAL_CALL XMLFilterDialogComponent::queryInterface( const Type& aType )
{
    return OComponentHelper::queryInterface( aType );
}


Any SAL_CALL XMLFilterDialogComponent::queryAggregation( Type const & rType )
{
    if (rType == cppu::UnoType<css::ui::dialogs::XExecutableDialog>::get())
    {
        void * p = static_cast< css::ui::dialogs::XExecutableDialog * >( this );
        return Any( &p, rType );
    }
    else if (rType == cppu::UnoType<XServiceInfo>::get())
    {
        void * p = static_cast< XServiceInfo * >( this );
        return Any( &p, rType );
    }
    else if (rType == cppu::UnoType<XInitialization>::get())
    {
        void * p = static_cast< XInitialization * >( this );
        return Any( &p, rType );
    }
    else if (rType == cppu::UnoType<XTerminateListener>::get())
    {
        void * p = static_cast< XTerminateListener * >( this );
        return Any( &p, rType );
    }
    return OComponentHelper::queryAggregation( rType );
}


void SAL_CALL XMLFilterDialogComponent::acquire() throw ()
{
    OComponentHelper::acquire();
}


void SAL_CALL XMLFilterDialogComponent::release() throw ()
{
    OComponentHelper::release();
}

/// @throws RuntimeException
OUString XMLFilterDialogComponent_getImplementationName()
{
    return OUString( "com.sun.star.comp.ui.XSLTFilterDialog" );
}

/// @throws RuntimeException
Sequence< OUString > SAL_CALL XMLFilterDialogComponent_getSupportedServiceNames()
{
    Sequence< OUString > aSupported { "com.sun.star.ui.dialogs.XSLTFilterDialog" };
    return aSupported;
}

/// @throws Exception
Reference< XInterface > SAL_CALL XMLFilterDialogComponent_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
{
    return static_cast<OWeakObject*>(new XMLFilterDialogComponent( comphelper::getComponentContext(rSMgr) ));
}

OUString SAL_CALL XMLFilterDialogComponent::getImplementationName()
{
    return XMLFilterDialogComponent_getImplementationName();
}

namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

Sequence< sal_Int8 > SAL_CALL XMLFilterDialogComponent::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


namespace
{
    class DialogComponentTypes
    {
    private:
        OTypeCollection m_aTypes;
    public:
        DialogComponentTypes() :
            m_aTypes(
                cppu::UnoType<XComponent>::get(),
                cppu::UnoType<XTypeProvider>::get(),
                cppu::UnoType<XAggregation>::get(),
                cppu::UnoType<XWeak>::get(),
                cppu::UnoType<XServiceInfo>::get(),
                cppu::UnoType<XInitialization>::get(),
                cppu::UnoType<XTerminateListener>::get(),
                cppu::UnoType<css::ui::dialogs::XExecutableDialog>::get())
        {
        }
        OTypeCollection& getTypeCollection() { return m_aTypes; }
    };

    struct theDialogComponentTypes : rtl::Static<DialogComponentTypes, theDialogComponentTypes> {};
}

Sequence< Type > XMLFilterDialogComponent::getTypes()
{
    return theDialogComponentTypes::get().getTypeCollection().getTypes();
}

Sequence< OUString > SAL_CALL XMLFilterDialogComponent::getSupportedServiceNames()
{
    return XMLFilterDialogComponent_getSupportedServiceNames();
}

sal_Bool SAL_CALL XMLFilterDialogComponent::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService( this, ServiceName );
}

/** Called in dispose method after the listeners were notified.
*/
void SAL_CALL XMLFilterDialogComponent::disposing()
{
    ::SolarMutexGuard aGuard;

    mpDialog.disposeAndClear();
}


// XTerminateListener
void SAL_CALL XMLFilterDialogComponent::queryTermination( const EventObject& /* Event */ )
{
    ::SolarMutexGuard aGuard;

    if (!mpDialog)
        return;

    // we will never give a veto here
    if (!mpDialog->isClosable())
    {
        mpDialog->ToTop();
        throw TerminationVetoException(
            "The office cannot be closed while the XMLFilterDialog is running",
            static_cast<XTerminateListener*>(this));
    }
    else
        mpDialog->Close();
}


void SAL_CALL XMLFilterDialogComponent::notifyTermination( const EventObject& /* Event */ )
{
    // we are going down, so dispose us!
    dispose();
}

void SAL_CALL XMLFilterDialogComponent::disposing( const EventObject& /* Source */ )
{
}


void SAL_CALL XMLFilterDialogComponent::setTitle( const OUString& /* _rTitle */ )
{
}


sal_Int16 SAL_CALL XMLFilterDialogComponent::execute(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDialog )
    {
        Reference< XComponent > xComp( this );
        if (mxParent.is())
            mpDialog = VclPtr<XMLFilterSettingsDialog>::Create(VCLUnoHelper::GetWindow(mxParent), mxContext);
        else
            mpDialog = VclPtr<XMLFilterSettingsDialog>::Create(nullptr, mxContext, Dialog::InitFlag::NoParent);
        mpDialog->Execute();
    }
    else if( !mpDialog->IsVisible() )
    {
        mpDialog->Execute();
    }
    mpDialog->ToTop();

    return 0;
}


void SAL_CALL XMLFilterDialogComponent::initialize( const Sequence< Any >& aArguments )
{
    const Any* pArguments = aArguments.getConstArray();
    for(sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
    {
        PropertyValue aProperty;
        if(*pArguments >>= aProperty)
        {
            if( aProperty.Name == "ParentWindow" )
            {
                aProperty.Value >>= mxParent;
            }
        }
    }
}


extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL xsltdlg_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    void * pRet = nullptr;

    if( pServiceManager )
    {
        Reference< XSingleServiceFactory > xFactory;

        OUString implName = OUString::createFromAscii( pImplName );
        if ( implName.equals(XMLFilterDialogComponent_getImplementationName()) )
        {
            xFactory = createOneInstanceFactory(
                static_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                XMLFilterDialogComponent_createInstance, XMLFilterDialogComponent_getSupportedServiceNames() );

        }

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

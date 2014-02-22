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
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <rtl/instance.hxx>
#include <comphelper/processfactory.hxx>

#include <svl/solar.hrc>

#include "xmlfiltersettingsdialog.hxx"

using namespace ::rtl;
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
                                    public ::com::sun::star::ui::dialogs::XExecutableDialog,
                                    public XServiceInfo,
                                    public XInitialization,
                                    public XTerminateListener
{
public:
    XMLFilterDialogComponent( const Reference< XComponentContext >& rxContext );
    virtual ~XMLFilterDialogComponent();

protected:
    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) throw (RuntimeException);
    virtual Any SAL_CALL queryAggregation( Type const & rType ) throw (RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XTypeProvider
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);
    virtual Sequence< Type > SAL_CALL getTypes() throw (RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

    // XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) throw(RuntimeException);
    virtual sal_Int16 SAL_CALL execute(  ) throw(RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const EventObject& Event ) throw (TerminationVetoException, RuntimeException);
    virtual void SAL_CALL notifyTermination( const EventObject& Event ) throw (RuntimeException);
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    /** Called in dispose method after the listeners were notified.
    */
    virtual void SAL_CALL disposing();

private:
    com::sun::star::uno::Reference<com::sun::star::awt::XWindow> mxParent;  /// parent window
    com::sun::star::uno::Reference< XComponentContext > mxContext;

    XMLFilterSettingsDialog* mpDialog;
};



namespace
{
    static ResMgr* pXSLTResMgr = NULL;
}

ResMgr* getXSLTDialogResMgr()
{
    return pXSLTResMgr;
}

XMLFilterDialogComponent::XMLFilterDialogComponent( const com::sun::star::uno::Reference< XComponentContext >& rxContext ) :
    OComponentHelper( maMutex ),
    mxContext( rxContext ),
    mpDialog( NULL )
{
    Reference< XDesktop2 > xDesktop = Desktop::create( rxContext );
    Reference< XTerminateListener > xListener( this );
    xDesktop->addTerminateListener( xListener );
}



XMLFilterDialogComponent::~XMLFilterDialogComponent()
{
}



// XInterface
Any SAL_CALL XMLFilterDialogComponent::queryInterface( const Type& aType ) throw (RuntimeException)
{
    return OComponentHelper::queryInterface( aType );
}



Any SAL_CALL XMLFilterDialogComponent::queryAggregation( Type const & rType ) throw (RuntimeException)
{
    if (rType == ::getCppuType( (Reference< ::com::sun::star::ui::dialogs::XExecutableDialog > const *)0 ))
    {
        void * p = static_cast< ::com::sun::star::ui::dialogs::XExecutableDialog * >( this );
        return Any( &p, rType );
    }
    else if (rType == ::getCppuType( (Reference< XServiceInfo > const *)0 ))
    {
        void * p = static_cast< XServiceInfo * >( this );
        return Any( &p, rType );
    }
    else if (rType == ::getCppuType( (Reference< XInitialization > const *)0 ))
    {
        void * p = static_cast< XInitialization * >( this );
        return Any( &p, rType );
    }
    else if (rType == ::getCppuType( (Reference< XTerminateListener > const *)0 ))
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



OUString XMLFilterDialogComponent_getImplementationName() throw ( RuntimeException )
{
    return OUString( "com.sun.star.comp.ui.XSLTFilterDialog" );
}

Sequence< OUString > SAL_CALL XMLFilterDialogComponent_getSupportedServiceNames()  throw ( RuntimeException )
{
    Sequence< OUString > aSupported(1);
    aSupported[0] = "com.sun.star.ui.dialogs.XSLTFilterDialog";
    return aSupported;
}

Reference< XInterface > SAL_CALL XMLFilterDialogComponent_createInstance( const Reference< XMultiServiceFactory > & rSMgr) throw ( Exception )
{
    return (OWeakObject*)new XMLFilterDialogComponent( comphelper::getComponentContext(rSMgr) );
}

OUString SAL_CALL XMLFilterDialogComponent::getImplementationName() throw(com::sun::star::uno::RuntimeException)
{
    return XMLFilterDialogComponent_getImplementationName();
}

namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

Sequence< sal_Int8 > SAL_CALL XMLFilterDialogComponent::getImplementationId( void ) throw( RuntimeException )
{
    ::cppu::OImplementationId &rID = lcl_ImplId::get();
    return rID.getImplementationId();
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
                ::getCppuType( (const Reference< XComponent > *)0 ),
                ::getCppuType( (const Reference< XTypeProvider > *)0 ),
                ::getCppuType( (const Reference< XAggregation > *)0 ),
                ::getCppuType( (const Reference< XWeak > *)0 ),
                ::getCppuType( (const Reference< XServiceInfo > *)0 ),
                ::getCppuType( (const Reference< XInitialization > *)0 ),
                ::getCppuType( (const Reference< XTerminateListener > *)0 ),
                ::getCppuType( (const Reference< ::com::sun::star::ui::dialogs::XExecutableDialog > *)0 ))
        {
        }
        OTypeCollection& getTypeCollection() { return m_aTypes; }
    };

    struct theDialogComponentTypes : rtl::Static<DialogComponentTypes, theDialogComponentTypes> {};
}

Sequence< Type > XMLFilterDialogComponent::getTypes() throw (RuntimeException)
{
    return theDialogComponentTypes::get().getTypeCollection().getTypes();
}

Sequence< OUString > SAL_CALL XMLFilterDialogComponent::getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException)
{
    return XMLFilterDialogComponent_getSupportedServiceNames();
}

sal_Bool SAL_CALL XMLFilterDialogComponent::supportsService(const OUString& ServiceName) throw(RuntimeException)
{
    return cppu::supportsService( this, ServiceName );
}

/** Called in dispose method after the listeners were notified.
*/
void SAL_CALL XMLFilterDialogComponent::disposing()
{
    ::SolarMutexGuard aGuard;

    if( mpDialog )
    {
        delete mpDialog;
        mpDialog = NULL;
    }

    if (pXSLTResMgr)
    {
        delete pXSLTResMgr;
        pXSLTResMgr = NULL;
    }
}



// XTerminateListener
void SAL_CALL XMLFilterDialogComponent::queryTermination( const EventObject& /* Event */ ) throw (TerminationVetoException, RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if (!mpDialog)
        return;

    // we will never give a veto here
    if (!mpDialog->isClosable())
    {
        mpDialog->ToTop();
        throw TerminationVetoException(
            OUString("The office cannot be closed while the XMLFilterDialog is running"),
            Reference<XInterface>(static_cast<XTerminateListener*>(this), UNO_QUERY));
    }
    else
        mpDialog->Close();
}



void SAL_CALL XMLFilterDialogComponent::notifyTermination( const EventObject& /* Event */ ) throw (RuntimeException)
{
    // we are going down, so dispose us!
    dispose();
}

void SAL_CALL XMLFilterDialogComponent::disposing( const EventObject& /* Source */ ) throw (RuntimeException)
{
}


void SAL_CALL XMLFilterDialogComponent::setTitle( const OUString& /* _rTitle */ ) throw(RuntimeException)
{
}


sal_Int16 SAL_CALL XMLFilterDialogComponent::execute(  ) throw(RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pXSLTResMgr )
    {
        pXSLTResMgr = ResMgr::CreateResMgr( "xsltdlg", Application::GetSettings().GetUILanguageTag() );
    }

    if( NULL == mpDialog )
    {
        Window* pParent = DIALOG_NO_PARENT;
        if (mxParent.is())
            pParent = VCLUnoHelper::GetWindow(mxParent);

        Reference< XComponent > xComp( this );
        mpDialog = new XMLFilterSettingsDialog(pParent, mxContext);
        mpDialog->Execute();
    }
    else if( !mpDialog->IsVisible() )
    {
        mpDialog->Execute();
    }
    mpDialog->ToTop();

    return 0;
}


void SAL_CALL XMLFilterDialogComponent::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
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
    void * pRet = 0;

    if( pServiceManager )
    {
        Reference< XSingleServiceFactory > xFactory;

        OUString implName = OUString::createFromAscii( pImplName );
        if ( implName.equals(XMLFilterDialogComponent_getImplementationName()) )
        {
            xFactory = createOneInstanceFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
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

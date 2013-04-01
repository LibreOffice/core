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


#include "taskpane/ToolPanelViewShell.hxx"
#include "framework/FrameworkHelper.hxx"
#include "PaneChildWindows.hxx"
#include "ViewShellBase.hxx"
#include "app.hrc"

#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <sfx2/frame.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <sfx2/viewfrm.hxx>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <toolkit/helper/vclunohelper.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::ui::XUIElementFactory;
    using ::com::sun::star::ui::XUIElement;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::lang::XInitialization;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::awt::XWindow;

    //==================================================================================================================
    //= ToolPanelFactory
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper3 <   XUIElementFactory
                                    ,   XServiceInfo
                                    ,   XInitialization
                                    >   ToolPanelFactory_Base;
    class ToolPanelFactory : public ToolPanelFactory_Base
    {
    public:
        ToolPanelFactory( const Reference< XComponentContext >& i_rContext );

        // XUIElementFactory
        virtual Reference< XUIElement > SAL_CALL createUIElement( const ::rtl::OUString& ResourceURL, const Sequence< PropertyValue >& Args ) throw (NoSuchElementException, IllegalArgumentException, RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException);

    protected:
        virtual ~ToolPanelFactory();

    private:
        const Reference< XComponentContext >    m_xContext;
    };

    //------------------------------------------------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ToolPanelFactory_createInstance( const Reference< XComponentContext >& i_rContext )
    {
        return static_cast< cppu::OWeakObject * >(
            new ToolPanelFactory( i_rContext ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString ToolPanelFactory_getImplementationName() throw(RuntimeException)
    {
        return ::rtl::OUString( "com.sun.star.comp.drawing.ToolPanelFactory" ) ;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ToolPanelFactory_getSupportedServiceNames (void)
        throw (RuntimeException)
    {
        const ::rtl::OUString sServiceName( "com.sun.star.drawing.DefaultToolPanelFactory" );
        return Sequence< ::rtl::OUString >( &sServiceName, 1 );
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelFactory::ToolPanelFactory( const Reference< XComponentContext >& i_rContext )
        :m_xContext( i_rContext )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelFactory::~ToolPanelFactory()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XUIElement > SAL_CALL ToolPanelFactory::createUIElement( const ::rtl::OUString& i_rResourceURL, const Sequence< PropertyValue >& i_rArgs ) throw (NoSuchElementException, IllegalArgumentException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;

        const PanelId ePanelId( toolpanel::GetStandardPanelId( i_rResourceURL ) );
        if ( ePanelId == PID_UNKNOWN )
            throw NoSuchElementException( i_rResourceURL, *this );

        const ::comphelper::NamedValueCollection aArgs( i_rArgs );
        const Reference< XFrame > xDocFrame( aArgs.getOrDefault( "Frame", Reference< XFrame >() ) );
        const Reference< XWindow > xParentWindow( aArgs.getOrDefault( "ParentWindow", Reference< XWindow >() ) );
        if ( !xDocFrame.is() || !xParentWindow.is() )
            throw IllegalArgumentException(
                "For creating a standard tool panel, a Frame and a Parent window are needed." ,
                *this,
                2
            );

        // look up the Sfx(View)Frame for the given XFrame
        SfxViewFrame* pViewFrame = NULL;
        for (   SfxFrame* pFrame = SfxFrame::GetFirst();
                pFrame != NULL;
                pFrame = SfxFrame::GetNext( *pFrame )
            )
        {
            if ( pFrame->GetFrameInterface() == xDocFrame )
            {
                pViewFrame = pFrame->GetCurrentViewFrame();
                break;
            }
        }

        if ( !pViewFrame || !pViewFrame->HasChildWindow( SID_TASKPANE ) )
            throw IllegalArgumentException(
                "Illegal frame." ,
                *this,
                2
            );

        // retrieve the task pane
        ToolPanelChildWindow* pToolPanelWindow( dynamic_cast< ToolPanelChildWindow* >( pViewFrame->GetChildWindow( SID_TASKPANE ) ) );
        if ( !pToolPanelWindow )
            throw IllegalArgumentException(
                "No Impress document, or no Impress Task Pane." ,
                *this,
                2
            );

        // retrieve the ViewShellBase, and the view shell of the task pane
        ViewShellBase* pViewShellBase = dynamic_cast< ViewShellBase* >( pViewFrame->GetViewShell() );
        ::boost::shared_ptr< framework::FrameworkHelper > pFrameworkHelper;
        if ( pViewShellBase )
            pFrameworkHelper = framework::FrameworkHelper::Instance( *pViewShellBase );
        ::boost::shared_ptr< ViewShell > pViewShell;
        if ( pFrameworkHelper.get() )
            pViewShell = pFrameworkHelper->GetViewShell( framework::FrameworkHelper::msRightPaneURL );
        ToolPanelViewShell* pToolPanelShell = dynamic_cast< ToolPanelViewShell* >( pViewShell.get() );

        if ( !pToolPanelShell )
            throw IllegalArgumentException(
                "Wrong document type." ,
                *this,
                2
            );

        ::Window* pParentWindow = VCLUnoHelper::GetWindow( xParentWindow );
        if ( !pParentWindow || !pToolPanelShell->IsPanelAnchorWindow( *pParentWindow ) )
            throw IllegalArgumentException(
                "Unsupported parent window." ,
                *this,
                2
            );

        return pToolPanelShell->CreatePanelUIElement( xDocFrame, i_rResourceURL );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ToolPanelFactory::getImplementationName(  ) throw (RuntimeException)
    {
        return ToolPanelFactory_getImplementationName();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL ToolPanelFactory::supportsService( const ::rtl::OUString& i_rServiceName ) throw (RuntimeException)
    {
        const Sequence< ::rtl::OUString > aSupported( getSupportedServiceNames() );
        for (   const ::rtl::OUString* pSupported = aSupported.getConstArray();
                pSupported != aSupported.getConstArray() + aSupported.getLength();
                ++pSupported
            )
            if ( *pSupported == i_rServiceName )
                return sal_True;

        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ToolPanelFactory::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        return ToolPanelFactory_getSupportedServiceNames();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ToolPanelFactory::initialize( const Sequence< Any >& i_rArguments ) throw (Exception, RuntimeException)
    {
        ::comphelper::NamedValueCollection aArgs( i_rArguments );
        (void)aArgs;
        // TODO
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

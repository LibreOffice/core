/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqlcommanddesign.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-15 14:40:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_SQLCOMMANDDESIGN_HXX
#include "sqlcommanddesign.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_UNOURL_HXX
#include "unourl.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
/** === end UNO includes === **/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    namespace FrameSearchFlag = ::com::sun::star::frame::FrameSearchFlag;
    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::lang::XMultiComponentFactory;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::awt::XTopWindow;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::frame::XComponentLoader;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::frame::XFramesSupplier;
    using ::com::sun::star::frame::XFrames;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::uno::TypeClass_STRING;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::frame::XDispatchProvider;
    using ::com::sun::star::frame::XDispatch;
    /** === end UNO using === **/

    //====================================================================
    //= SQLCommandDesigner
    //====================================================================
    //--------------------------------------------------------------------
    SQLCommandDesigner::SQLCommandDesigner( const Reference< XComponentContext >& _rxContext,
            const Reference< XPropertySet >& _rxRowSet, const ::dbtools::SharedConnection& _rConnection,
            const Link& _rCloseLink )
        :m_xContext( _rxContext )
        ,m_xRowSet( _rxRowSet )
        ,m_xConnection( _rConnection )
        ,m_aCloseLink( _rCloseLink )
    {
        if ( m_xContext.is() )
            m_xORB = m_xContext->getServiceManager();
        if ( !m_xORB.is() || !m_xRowSet.is() || !m_xConnection.is() )
            throw NullPointerException();

        impl_doOpenDesignerFrame_nothrow();
    }

    //--------------------------------------------------------------------
    SQLCommandDesigner::~SQLCommandDesigner()
    {
    }

    //--------------------------------------------------------------------
    void SAL_CALL SQLCommandDesigner::propertyChange( const PropertyChangeEvent& Event ) throw (RuntimeException)
    {
        OSL_ENSURE( m_xDesigner.is() && ( Event.Source == m_xDesigner ), "SQLCommandDesigner::propertyChange: where did this come from?" );

        if ( m_xDesigner.is() && ( Event.Source == m_xDesigner ) )
        {
            if ( PROPERTY_ACTIVECOMMAND == Event.PropertyName )
            {
                OSL_ENSURE( Event.NewValue.getValueTypeClass() == TypeClass_STRING,
                    "SQLCommandDesigner::propertyChange: invalid new value for the ActiveCommand!" );
                m_xRowSet->setPropertyValue( PROPERTY_COMMAND, Event.NewValue );
            }
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL SQLCommandDesigner::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        OSL_ENSURE( m_xDesigner.is() && ( Source.Source == m_xDesigner ), "SQLCommandDesigner::propertyChange: where did this come from?" );

        if ( m_xDesigner.is() && ( Source.Source == m_xDesigner ) )
        {
            impl_designerClosed_nothrow();
            m_xDesigner.clear();
        }
    }

    //--------------------------------------------------------------------
    void SQLCommandDesigner::dispose()
    {
        if ( impl_isDisposed() )
            return;

        if ( isActive() )
            impl_closeDesigner_throw();

        m_xConnection.clear();
        m_xContext.clear();
        m_xRowSet.clear();
        m_xORB.clear();
    }

    //--------------------------------------------------------------------
    void SQLCommandDesigner::impl_checkDisposed_throw() const
    {
        if ( impl_isDisposed() )
            throw DisposedException();
    }

    //--------------------------------------------------------------------
    void SQLCommandDesigner::raise() const
    {
        impl_checkDisposed_throw();
        impl_raise_nothrow();
    }

    //------------------------------------------------------------------------
    bool SQLCommandDesigner::suspend() const
    {
        impl_checkDisposed_throw();
        return impl_trySuspendDesigner_nothrow();
    }

    //--------------------------------------------------------------------
    void SQLCommandDesigner::impl_raise_nothrow() const
    {
        OSL_PRECOND( isActive(), "SQLCommandDesigner::impl_raise_nothrow: not active!" );
        if ( !isActive() )
            return;

        try
        {
            // activate the frame for this component
            Reference< XFrame >     xFrame( m_xDesigner->getFrame(), UNO_QUERY_THROW );
            Reference< XWindow >    xWindow( xFrame->getContainerWindow(), UNO_QUERY_THROW );
            Reference< XTopWindow > xTopWindow( xWindow, UNO_QUERY_THROW );

            xTopWindow->toFront();
            xWindow->setFocus();
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "SQLCommandDesigner::impl_raise_nothrow: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void SQLCommandDesigner::impl_doOpenDesignerFrame_nothrow()
    {
        OSL_PRECOND( !isActive(),
            "SQLCommandDesigner::impl_doOpenDesignerFrame_nothrow: already active!" );
        OSL_PRECOND( m_xConnection.is() && m_xRowSet.is(), "SQLCommandDesigner::impl_doOpenDesignerFrame_nothrow: this will crash!" );

        try
        {
            // for various reasons, we don't want the new frame to appear in the desktop's frame list
            // thus, we create a blank frame at the desktop, remove it from the desktop's frame list
            // immediately, and then load the component into this blank (and now parent-less) frame
            Reference< XComponentLoader > xLoader( impl_createEmptyParentlessTask_nothrow(), UNO_QUERY_THROW );
            Sequence< PropertyValue > aArgs( 3 );
            aArgs[0].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IndependentSQLCommand" ) );
            aArgs[0].Value = m_xRowSet->getPropertyValue( PROPERTY_COMMAND );
            aArgs[1].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ActiveConnection" ) );
            aArgs[1].Value <<= m_xConnection.getTyped();
            aArgs[2].Name  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "QueryDesignView" ) );
            aArgs[2].Value <<= (sal_Bool)sal_True;

            Reference< XComponent > xQueryDesign = xLoader->loadComponentFromURL(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".component:DB/QueryDesign" ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_self" ) ),
                FrameSearchFlag::TASKS | FrameSearchFlag::CREATE,
                aArgs
            );

            // remember this newly loaded component - we need to care for it e.g. when we're suspended
            m_xDesigner = m_xDesigner.query( xQueryDesign );
            OSL_ENSURE( m_xDesigner.is() || !xQueryDesign.is(), "SQLCommandDesigner::impl_doOpenDesignerFrame_nothrow: the component is expected to be a controller!" );
            if ( m_xDesigner.is() )
            {
                Reference< XPropertySet > xQueryDesignProps( m_xDesigner, UNO_QUERY );
                OSL_ENSURE( xQueryDesignProps.is(), "SQLCommandDesigner::impl_doOpenDesignerFrame_nothrow: the controller should have properties!" );
                if ( xQueryDesignProps.is() )
                    xQueryDesignProps->addPropertyChangeListener( PROPERTY_ACTIVECOMMAND, this );
            }

            // get the frame which we just opened and set it's title
            Reference< XPropertySet > xFrameProps;
            Reference< XController > xController( xQueryDesign, UNO_QUERY );
            if ( xController.is() )
                xFrameProps = xFrameProps.query( xController->getFrame() );

            if ( xFrameProps.is() && xFrameProps->getPropertySetInfo().is() && xFrameProps->getPropertySetInfo()->hasPropertyByName( PROPERTY_TITLE ) )
            {
                ::svt::OLocalResourceAccess aEnumStrings( PcrRes( RID_RSC_ENUM_COMMAND_TYPE ), RSC_RESOURCE );
                ::rtl::OUString sDisplayName = String( ResId( 3 ) );
                xFrameProps->setPropertyValue( PROPERTY_TITLE, makeAny( sDisplayName ) );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "SQLCommandDesigner::impl_doOpenDesignerFrame_nothrow: caught an exception!" );
            m_xDesigner.clear();
        }
    }

    //------------------------------------------------------------------------
    Reference< XFrame > SQLCommandDesigner::impl_createEmptyParentlessTask_nothrow( ) const
    {
        OSL_PRECOND( m_xORB.is(), "SQLCommandDesigner::impl_createEmptyParentlessTask_nothrow: this will crash!" );

        Reference< XFrame > xFrame;
        try
        {
            Reference< XInterface      > xDesktop          ( m_xORB->createInstanceWithContext( SERVICE_DESKTOP, m_xContext ) );
            Reference< XFrame          > xDesktopFrame     ( xDesktop,      UNO_QUERY_THROW );
            Reference< XFramesSupplier > xSuppDesktopFrames( xDesktopFrame, UNO_QUERY_THROW );

            Reference< XFrames > xDesktopFramesCollection( xSuppDesktopFrames->getFrames(), UNO_QUERY_THROW );
            xFrame = xDesktopFrame->findFrame( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ), FrameSearchFlag::CREATE );
            OSL_ENSURE( xFrame.is(), "SQLCommandDesigner::impl_createEmptyParentlessTask_nothrow: could not create an empty frame!" );
            xDesktopFramesCollection->remove( xFrame );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "SQLCommandDesigner::impl_createEmptyParentlessTask_nothrow: caught an exception!" );
        }
        return xFrame;
    }

    //------------------------------------------------------------------------
    void SQLCommandDesigner::impl_designerClosed_nothrow()
    {
        if ( m_aCloseLink.IsSet() )
            m_aCloseLink.Call( this );
    }

    //------------------------------------------------------------------------
    void SQLCommandDesigner::impl_closeDesigner_throw()
    {
        impl_checkDisposed_throw();
        OSL_PRECOND( isActive(), "SQLCommandDesigner::impl_closeDesigner_throw: invalid calle!" );

        // close it
        try
        {
            // do not listen anymore ....
            Reference< XPropertySet > xProps( m_xDesigner, UNO_QUERY );
            if ( xProps.is() )
                xProps->removePropertyChangeListener( PROPERTY_ACTIVECOMMAND, this );

            // we need to close the frame via the "user interface", by dispatching a close command,
            // instead of calling XCloseable::close directly. The latter method would also close
            // the frame, but not care for things like shutting down the office when the last
            // frame is gone ...
            UnoURL aCloseURL( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CloseDoc" ) ),
                Reference< XMultiServiceFactory >( m_xORB, UNO_QUERY ) );

            Reference< XDispatchProvider > xProvider( m_xDesigner->getFrame(), UNO_QUERY_THROW );
            Reference< XDispatch > xDispatch( xProvider->queryDispatch( aCloseURL, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_top" ) ), FrameSearchFlag::SELF ) );
            OSL_ENSURE( xDispatch.is(), "SQLCommandDesigner::impl_closeDesigner_throw: no dispatcher for the CloseDoc command!" );
            if ( xDispatch.is() )
            {
                xDispatch->dispatch( aCloseURL, Sequence< PropertyValue >( ) );
            }
            else
            {
                // fallback: use the XCloseable::close (with all possible disadvantages)
                Reference< XCloseable > xClose( m_xDesigner->getFrame(), UNO_QUERY );
                if ( xClose.is() )
                    xClose->close( sal_True );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "SQLCommandDesigner::impl_closeDesigner_throw: caught an exception!" );
        }

        m_xDesigner = NULL;
    }

    //------------------------------------------------------------------------
    bool SQLCommandDesigner::impl_trySuspendDesigner_nothrow() const
    {
        OSL_PRECOND( isActive(), "SQLCommandDesigner::impl_trySuspendDesigner_nothrow: no active designer, this will crash!" );
        sal_Bool bAllow = sal_True;
        try
        {
            bAllow = m_xDesigner->suspend( sal_True );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "SQLCommandDesigner::impl_trySuspendDesigner_nothrow: caught an exception!" );
        }
        return bAllow;
    }
//........................................................................
} // namespace pcr
//........................................................................


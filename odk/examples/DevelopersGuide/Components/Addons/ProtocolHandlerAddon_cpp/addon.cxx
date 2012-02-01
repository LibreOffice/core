/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <addon.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XMessageBox.hpp>

using rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::awt;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::PropertyValue;
using com::sun::star::util::URL;

// This is the service name an Add-On has to implement
#define SERVICE_NAME "com.sun.star.frame.ProtocolHandler"


/**
  * Show a message box with the UNO based toolkit
  */
static void ShowMessageBox( const Reference< XToolkit >& rToolkit, const Reference< XFrame >& rFrame, const OUString& aTitle, const OUString& aMsgText )
{
    if ( rFrame.is() && rToolkit.is() )
    {
        // describe window properties.
        WindowDescriptor                aDescriptor;
        aDescriptor.Type              = WindowClass_MODALTOP;
        aDescriptor.WindowServiceName = OUString( RTL_CONSTASCII_USTRINGPARAM( "infobox" ));
        aDescriptor.ParentIndex       = -1;
        aDescriptor.Parent            = Reference< XWindowPeer >( rFrame->getContainerWindow(), UNO_QUERY );
        aDescriptor.Bounds            = Rectangle(0,0,300,200);
        aDescriptor.WindowAttributes  = WindowAttribute::BORDER |
WindowAttribute::MOVEABLE |
WindowAttribute::CLOSEABLE;

        Reference< XWindowPeer > xPeer = rToolkit->createWindow( aDescriptor );
        if ( xPeer.is() )
        {
            Reference< XMessageBox > xMsgBox( xPeer, UNO_QUERY );
            if ( xMsgBox.is() )
            {
                xMsgBox->setCaptionText( aTitle );
                xMsgBox->setMessageText( aMsgText );
                xMsgBox->execute();
            }
        }
    }
}

/**
  * Called by the Office framework.
  * One-time initialization. We have to store the context information
  * given, like the frame we are bound to, into our members.
  */
void SAL_CALL Addon::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException)
{
    Reference < XFrame > xFrame;
    if ( aArguments.getLength() )
    {
        aArguments[0] >>= xFrame;
        mxFrame = xFrame;
    }

    // Create the toolkit to have access to it later
    mxToolkit = Reference< XToolkit >( mxMSF->createInstance(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            "com.sun.star.awt.Toolkit" ))), UNO_QUERY );
}

/**
  * Called by the Office framework.
  * We are ask to query the given URL and return a dispatch object if the URL
  * contains an Add-On command.
  */
Reference< XDispatch > SAL_CALL Addon::queryDispatch( const URL& aURL, const ::rtl::OUString& sTargetFrameName, sal_Int32 nSearchFlags )
                throw( RuntimeException )
{
    Reference < XDispatch > xRet;
    if ( aURL.Protocol.compareToAscii("org.openoffice.Office.addon.example:") == 0 )
    {
        if ( aURL.Path.compareToAscii( "Function1" ) == 0 )
            xRet = this;
        else if ( aURL.Path.compareToAscii( "Function2" ) == 0 )
            xRet = this;
        else if ( aURL.Path.compareToAscii( "Help" ) == 0 )
            xRet = this;
    }

    return xRet;
}

/**
  * Called by the Office framework.
  * We are ask to execute the given Add-On command URL.
  */
void SAL_CALL Addon::dispatch( const URL& aURL, const Sequence < PropertyValue >& lArgs ) throw (RuntimeException)
{
    if ( aURL.Protocol.compareToAscii("org.openoffice.Office.addon.example:") == 0 )
    {
        if ( aURL.Path.compareToAscii( "Function1" ) == 0 )
        {
            ShowMessageBox( mxToolkit, mxFrame,
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "SDK Add-On example" )),
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "Function 1 activated" )) );
        }
        else if ( aURL.Path.compareToAscii( "Function2" ) == 0 )
        {
            ShowMessageBox( mxToolkit, mxFrame,
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "SDK Add-On example" )),
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "Function 2 activated" )) );
        }
        else if ( aURL.Path.compareToAscii( "Help" ) == 0 )
        {
            // Show info box
            ShowMessageBox( mxToolkit, mxFrame,
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "About SDK Add-On example" )),
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "This is the SDK Add-On example" )) );
    }
    }
}

/**
  * Called by the Office framework.
  * We are ask to query the given sequence of URLs and return dispatch objects if the URLs
  * contain Add-On commands.
  */
Sequence < Reference< XDispatch > > SAL_CALL Addon::queryDispatches( const Sequence < DispatchDescriptor >& seqDescripts )
            throw( RuntimeException )
{
    sal_Int32 nCount = seqDescripts.getLength();
    Sequence < Reference < XDispatch > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL, seqDescripts[i].FrameName, seqDescripts[i].SearchFlags );

    return lDispatcher;
}

/**
  * Called by the Office framework.
  * We are ask to query the given sequence of URLs and return dispatch objects if the URLs
  * contain Add-On commands.
  */
void SAL_CALL Addon::addStatusListener( const Reference< XStatusListener >& xControl, const URL& aURL ) throw (RuntimeException)
{
}

/**
  * Called by the Office framework.
  * We are ask to query the given sequence of URLs and return dispatch objects if the URLs
  * contain Add-On commands.
  */
void SAL_CALL Addon::removeStatusListener( const Reference< XStatusListener >& xControl, const URL& aURL ) throw (RuntimeException)
{
}

//##################################################################################################
//#### Helper functions for the implementation of UNO component interfaces #########################
//##################################################################################################

::rtl::OUString Addon_getImplementationName()
throw (RuntimeException)
{
    return ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL Addon_supportsService( const ::rtl::OUString& ServiceName )
throw (RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
}

Sequence< ::rtl::OUString > SAL_CALL Addon_getSupportedServiceNames()
throw (RuntimeException)
{
    Sequence < ::rtl::OUString > aRet(1);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}

Reference< XInterface > SAL_CALL Addon_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new Addon( rSMgr );
}

//##################################################################################################
//#### Implementation of the recommended/mandatory interfaces of a UNO component ###################
//##################################################################################################

// XServiceInfo
::rtl::OUString SAL_CALL Addon::getImplementationName(  )
    throw (RuntimeException)
{
    return Addon_getImplementationName();
}

sal_Bool SAL_CALL Addon::supportsService( const ::rtl::OUString& rServiceName )
    throw (RuntimeException)
{
    return Addon_supportsService( rServiceName );
}

Sequence< ::rtl::OUString > SAL_CALL Addon::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return Addon_getSupportedServiceNames();
}

/*************************************************************************
 *
 *  $RCSfile: TestMinimalComponent.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-06-30 15:56:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XSet;
import com.sun.star.lang.XServiceInfo;

public class TestMinimalComponent {
  public static void main(String args[]) {
    try {
      /* Bootstraps a component context with the jurt base components
         registered. Component context to be granted to a component for running.
         Arbitrary values can be retrieved from the context. */
      XComponentContext xcomponentcontext =
      com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

      /* Gets the service manager instance to be used (or null). This method has
         been added for convenience, because the service manager is a often used
         object. */
      XMultiComponentFactory xmulticomponentfactory =
      xcomponentcontext.getServiceManager();

      // Querying for the interface XSet on XMultiComponentFactory
      XSet xsetMultiServiceFactory = ( XSet ) UnoRuntime.queryInterface(
      XSet.class, xmulticomponentfactory );

      // Getting the XSingleServiceFactory for the minimal component
      XSingleServiceFactory xsingleservicefactoryMinimalComponent =
      MinimalComponent.__getServiceFactory(
      "MinimalComponent$MinimalComponentImplementation",
      ( XMultiServiceFactory ) UnoRuntime.queryInterface(
      XMultiServiceFactory.class, xmulticomponentfactory ),
      null );

      // Inserting the XSingleServiceFactory into the container
      xsetMultiServiceFactory.insert( xsingleservicefactoryMinimalComponent );

      // Creating an instance of the minimal component
      Object objectMinimalComponent =
      xmulticomponentfactory.createInstanceWithContext(
      "org.OpenOffice.MinimalComponent", xcomponentcontext );

      XServiceInfo xserviceinfoMinimalComponent = ( XServiceInfo )
      UnoRuntime.queryInterface( XServiceInfo.class,
      objectMinimalComponent );

      System.out.println( "\nXServiceInfo is used to get the implementation name:\n    -> " +
                          xserviceinfoMinimalComponent.getImplementationName() +
                          "\nOk");

      // Removing the XSingleServiceFactory of the minimal Component from the container
      xsetMultiServiceFactory.remove( xsingleservicefactoryMinimalComponent );

      xcomponentcontext = null;

      System.exit(0);
    }
    catch( Exception exception ) {
      System.err.println( exception );
    }
  }
}

/*************************************************************************
 *
 *  $RCSfile: MinimalComponent.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-06-30 15:56:46 $
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

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.Type;

/** This class capsulates the class, that implements the minimal component, a
 * factory for creating the service (<CODE>__getServiceFactory</CODE>) and a
 * method, that writes the information into the given registry key
 * (<CODE>__writeRegistryServiceInfo</CODE>).
 * @version $Date: 2003-06-30 15:56:46 $
 * @author Bertram Nolte
 */
public class MinimalComponent {
  /** This class implements the component. At least the interfaces XServiceInfo,
   * XTypeProvider, and XInitialization should be provided by the service.
   */
  public static class MinimalComponentImplementation extends WeakBase implements XInitialization,
  XServiceInfo {
    /** The service name, that must be used to get an instance of this service.
     */
    static private final String __serviceName =
    "org.OpenOffice.MinimalComponent";

    /** The service manager, that gives access to all registered services.
     */
    private XMultiServiceFactory xmultiservicefactory;

    /** The constructor of the inner class has a XMultiServiceFactory parameter.
     * @param xmultiservicefactoryInitialization A special service factory
     * could be introduced while initializing.
     */
    public MinimalComponentImplementation(
    XMultiServiceFactory xmultiservicefactoryInitialization ) {
      xmultiservicefactory = xmultiservicefactoryInitialization;
    }

    /** This method is a member of the interface for initializing an object
     * directly after its creation.
     * @param object This array of arbitrary objects will be passed to the
     * component after its creation.
     * @throws Exception Every exception will not be handled, but will be
     * passed to the caller.
     */
    public void initialize( Object[] object )
    throws com.sun.star.uno.Exception {
      xmultiservicefactory = ( XMultiServiceFactory ) UnoRuntime.queryInterface(
      XMultiServiceFactory.class, object[ 0 ] );
    }

    /** This method returns an array of all supported service names.
     * @return Array of supported service names.
     */
    public String[] getSupportedServiceNames() {
      String []stringSupportedServiceNames = new String[ 1 ];

      stringSupportedServiceNames[ 0 ] = __serviceName;

      return( stringSupportedServiceNames );
    }

    /** This method returns true, if the given service will be
     * supported by the component.
     * @param stringService Service name.
     * @return True, if the given service name will be supported.
     */
    public boolean supportsService( String stringService ) {
      boolean booleanSupportsService = false;

      if ( stringService.equals( __serviceName ) ) {
        booleanSupportsService = true;
      }

      return( booleanSupportsService );
    }

    /** Return the class name of the component.
     * @return Class name of the component.
     */
    public String getImplementationName() {
      return( MinimalComponentImplementation.class.getName() );
    }

  }


  /** Gives a factory for creating the service.
   * This method is called by the <code>JavaLoader</code>
   * <p>
   * @return Returns a <code>XSingleServiceFactory</code> for creating the
   * component.
   * @see com.sun.star.comp.loader.JavaLoader#
   * @param stringImplementationName The implementation name of the component.
   * @param xmultiservicefactory The service manager, who gives access to every
   * known service.
   * @param xregistrykey Makes structural information (except regarding tree
   * structures) of a single
   * registry key accessible.
   */
  public static XSingleServiceFactory __getServiceFactory(
  String stringImplementationName,
  XMultiServiceFactory xmultiservicefactory,
  XRegistryKey xregistrykey ) {
    XSingleServiceFactory xsingleservicefactory = null;

    if ( stringImplementationName.equals(
    MinimalComponentImplementation.class.getName() ) )
      xsingleservicefactory = FactoryHelper.getServiceFactory(
      MinimalComponentImplementation.class,
      MinimalComponentImplementation.__serviceName,
      xmultiservicefactory,
      xregistrykey );

    return xsingleservicefactory;
  }

  /** Writes the service information into the given registry key.
   * This method is called by the <code>JavaLoader</code>.
   * @return returns true if the operation succeeded
   * @see com.sun.star.comp.loader.JavaLoader#
   * @param xregistrykey Makes structural information (except regarding tree
   * structures) of a single
   * registry key accessible.
   */
  public static boolean __writeRegistryServiceInfo(
  XRegistryKey xregistrykey ) {
    return FactoryHelper.writeRegistryServiceInfo(
    MinimalComponentImplementation.class.getName(),
    MinimalComponentImplementation.__serviceName,
    xregistrykey );
  }
}

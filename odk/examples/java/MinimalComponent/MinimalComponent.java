/*************************************************************************
 *
 *  $RCSfile: MinimalComponent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jsc $ $Date: 2003-02-28 17:39:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser cp General Public License Version 2.1
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
 * @version $Date: 2003-02-28 17:39:28 $
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

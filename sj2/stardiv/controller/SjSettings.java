/*************************************************************************
 *
 *  $RCSfile: SjSettings.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:54:03 $
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

package stardiv.controller;

import java.util.Properties;
import java.util.Hashtable;
import java.util.Enumeration;

import com.sun.star.lib.sandbox.SandboxSecurity;

/**
 * Über diese Klasse werden alle globalen Einstellungen, die für das Sj Projekt
 * wichtig sind, dokumentiert und modifiziert.
 *
 * @version    $Version: 1.0 $
 * @author     Markus Meyer
 *
 */
public final class SjSettings {
    /**
     * The following properties are used to setup the environment for
     * the stardiv packages.<BR>
     * "appletviewer.security.mode"="unrestricted" | "host" | "none": Set the
     * security level of the default SecurityManager. The default is "host".<BR>
     * "stardiv.security.defaultSecurityManager"="true" | "false": Create and set
     * the stardiv.security.AppletSecurity, if the property is "true". This occures
     * only in the first call.<BR>
     * "stardiv.security.noExit"="true" | "false": no exit is allowed. Use this property
     * if you are running more than one java application in the virtual machine. This occures
     * only in the first call.<BR>
     * "stardiv.security.disableSecurity"="true" | "false": disable security checking. Only usefull
     * if a SecurityManager is installed. The default is "false".<BR>
     * if you are running more than one java application in the virtual machine. This occures
     * only in the first call.<BR>
     * "stardiv.controller.installConsole"="true" | "false": pipe the stdout and stderr
     * through a console. Show the console with stardiv.controller.Console.showConsole( true ).
     * This occures only in the first call.<BR>
     * "stardiv.js.debugOnError"="true" | "false": Start the javascript ide, if an error
     * occures. The default is "false".<BR>
     * "stardiv.js.debugImmediate"="true" | "false": Start the javascript ide, if a script
     * starts. The default is "false".<BR>
     * "stardiv.debug.trace"="messageBox" | "window" | "file" | "none": The trace pipe is
     * set to one of the four mediums. The Default is "none".<BR>
     * "stardiv.debug.error"="messageBox" | "window" | "file" | "none": The error pipe is
     * set to one of the four mediums. The Default is "none".<BR>
     * "stardiv.debug.warning"="messageBox" | "window" | "file" | "none": The warning pipe is
     * set to one of the four mediums. The Default is "none".<BR>
     * If the properties http.proxyHost, http.proxyPort, http.maxConnections,
     * http.keepAlive or http.nonProxyHosts are changed, the method
     * sun.net.www.http.HttpClient.resetProperties() is called.<BR>
     * If the properties ftpProxySet, ftpProxyHost or ftpProxyPort are changed,
     * the static variables useFtpProxy, ftpProxyHost and ftpProxyPort in the class
     * sun.net.ftp.FtpClient are set.<BR>
     * <B>If you are writing your own SecurityManager and ClassLoader, please implement the
     * interfaces stardiv.security.ClassLoaderExtension and
     * stardiv.security.SecurityManagerExtension. Be shure to set the
     * stardiv.security.ClassLoaderFactory, to enable dynamic class loading, otherwise
     * the stardiv.security.AppletClassLoader is used. Set the factory with
     * SjSettings.setClassLoaderFactory().</B>
     */
    static public synchronized void changeProperties( Properties pChangeProps )
    {
        SecurityManager pSM = System.getSecurityManager();
        if( pSM != null )
            pSM.checkPropertiesAccess();
        Properties props = new Properties( System.getProperties() );
        boolean bInited = Boolean.getBoolean( "stardiv.controller.SjSettings.inited" );


        if( !bInited )
        {
            // check the awt.toolkit property: if none is set use com.sun.star.comp.jawt.peer.Toolkit
            //if ( props.getProperty("awt.toolkit") == null )
            //  props.put("awt.toolkit", "com.sun.star.comp.jawt.peer.Toolkit");

            // Define a number of standard properties
            props.put("acl.read", "+");
            props.put("acl.read.default", "");
            props.put("acl.write", "+");
            props.put("acl.write.default", "");

            // Standard browser properties
            props.put("browser", "stardiv.applet.AppletViewerFrame");
            props.put("browser.version", "4.02");
            props.put("browser.vendor", "Sun Microsystems, Inc.");
            props.put("http.agent", "JDK/1.1");

            // Define which packages can be accessed by applets
            props.put("package.restrict.access.sun", "true");
            props.put("package.restrict.access.netscape", "true");
            props.put("package.restrict.access.stardiv", "true");

            // Define which packages can be extended by applets
            props.put("package.restrict.definition.java", "true");
            props.put("package.restrict.definition.sun", "true");
            props.put("package.restrict.definition.netscape", "true");
            props.put("package.restrict.definition.stardiv", "true");

            // Define which properties can be read by applets.
            // A property named by "key" can be read only when its twin
            // property "key.applet" is true.  The following ten properties
            // are open by default.  Any other property can be explicitly
            // opened up by the browser user setting key.applet=true in
            // ~/.hotjava/properties.   Or vice versa, any of the following can
            // be overridden by the user's properties.
            props.put("java.version.applet", "true");
            props.put("java.vendor.applet", "true");
            props.put("java.vendor.url.applet", "true");
            props.put("java.class.version.applet", "true");
            props.put("os.name.applet", "true");
            props.put("os.version.applet", "true");
            props.put("os.arch.applet", "true");
            props.put("file.separator.applet", "true");
            props.put("path.separator.applet", "true");
            props.put("line.separator.applet", "true");

            // das appletresourceprotokol
            props.put("java.protocol.handler.pkgs", "stardiv.net.protocol");
        }

        boolean bHttpClientChanged = false;
        boolean bFtpClientChanged = false;
        boolean bSecurityChanged = false;
        // detect changes
        if( pChangeProps != null )
        {
            bHttpClientChanged =
                 !equalsImpl( props.get( "http.proxyHost" ), pChangeProps.get( "http.proxyHost" ) )
              || !equalsImpl( props.get( "http.proxyPort" ), pChangeProps.get( "http.proxyPort" ) )
              || !equalsImpl( props.get( "http.maxConnections" ), pChangeProps.get( "http.maxConnections" ) )
              || !equalsImpl( props.get( "http.keepAlive" ), pChangeProps.get( "http.keepAlive" ) )
              || !equalsImpl( props.get( "http.nonProxyHosts" ), pChangeProps.get( "http.nonProxyHosts" ) );
            bFtpClientChanged =
                 !equalsImpl( props.get( "ftpProxySet" ), pChangeProps.get( "ftpProxySet" ) )
              || !equalsImpl( props.get( "ftpProxyHost" ), pChangeProps.get( "ftpProxyHost" ) )
              || !equalsImpl( props.get( "ftpProxyPort" ), pChangeProps.get( "ftpProxyPort" ) );
            bSecurityChanged =
                 !equalsImpl( props.get( "appletviewer.security.mode" ), pChangeProps.get( "appletviewer.security.mode" ) )
              || !equalsImpl( props.get( "stardiv.security.disableSecurity" ), pChangeProps.get( "stardiv.security.disableSecurity" ) );
        }

        // put new and changed properties to the property table
        if( pChangeProps != null )
        {
            Enumeration aEnum = pChangeProps.propertyNames();
            while( aEnum.hasMoreElements() )
            {
                String aKey = (String)aEnum.nextElement();
                props.put( aKey, pChangeProps.getProperty( aKey ) );
            }
        }

        // Install a property list.
        if( !bInited )
            props.put( "stardiv.controller.SjSettings.inited", "true" );
        System.setProperties(props);
        if( !bInited )
        {
            // Security Manager setzten
            if( Boolean.getBoolean( "stardiv.security.defaultSecurityManager" ) )
            {
                boolean bNoExit = Boolean.getBoolean( "stardiv.security.noExit" );
                //Create and install the security manager
                System.setSecurityManager(new SandboxSecurity(bNoExit));
            }
            if( Boolean.getBoolean("stardiv.controller.installConsole") )
                Console.installConsole();

        }
        // Not Documented, setting a try catch, for IncompatibleClassChangeException.
        try
        {
            if( bHttpClientChanged )
                sun.net.www.http.HttpClient.resetProperties();
            if( bFtpClientChanged )
            {
                sun.net.ftp.FtpClient.useFtpProxy = Boolean.getBoolean("ftpProxySet");
                sun.net.ftp.FtpClient.ftpProxyHost = System.getProperty("ftpProxyHost");
                sun.net.ftp.FtpClient.ftpProxyPort = Integer.getInteger("ftpProxyPort", 80).intValue();
            }
        }
        catch( Throwable e )
        {
        }
        if( bSecurityChanged )
        {
            pSM = System.getSecurityManager();
            if( pSM instanceof SandboxSecurity )
            {
                ((SandboxSecurity)pSM).reset();
            }
        }
    }

    private static boolean equalsImpl( Object p1, Object p2 )
    {
        return p1 == p2 || (p1 != null && p2 != null && p1.equals( p2 ) );
    }
}




/*************************************************************************
 *
 *  $RCSfile: SandboxSecurity.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:28 $
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

package com.sun.star.lib.sandbox;

import java.io.File;
import java.io.IOException;
import java.io.FileDescriptor;
import java.net.URL;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.Hashtable;
import java.security.*;

import sun.security.provider.*;

/**
 * This class defines an applet security policy
 *
 * @version     1.68, 01/27/97
 */
public class SandboxSecurity extends SecurityManager //implements SecurityManagerExtension
{

    private static boolean debug = false;

    public final static int NETWORK_NONE = 1;
    public final static int NETWORK_HOST = 2;
    public final static int NETWORK_UNRESTRICTED = 3;

    private final static int PRIVELEGED_PORT = 1024;

    boolean bNoExit;
    boolean initACL;
    String  readACL[];
    String  writeACL[];
    int     networkMode;
    boolean bCheckSecurity;

    // where we look for identities
    IdentityScope scope;
    // local cache for network-loaded classes
    Hashtable loadedClasses;

    public int getNetworkMode(){ return networkMode; }

    /**
     * Construct and initialize.
     */
    public SandboxSecurity() {
        reset();
    }

    /**
     * Construct and initialize.
     */
    public SandboxSecurity( boolean bNoExit_ ) {
        reset();
        bNoExit = bNoExit_;
    }

    /**
     * Reset from Properties
     */
    public void reset() {
        String str = System.getProperty("appletviewer.security.mode");
//          System.err.println("#### SandboxSecurity.reset:" + str);
        if( str == null )
            networkMode = NETWORK_HOST;
        else if( str.equals("unrestricted") )
            networkMode = NETWORK_UNRESTRICTED;
        else if( str.equals("none") )
            networkMode = NETWORK_NONE;
        else
            networkMode = NETWORK_HOST;

//          bCheckSecurity = !Boolean.getBoolean( "stardiv.security.disableSecurity" );
        // see if security is disabled
        String prop = System.getProperty("Security", "1" );
        bCheckSecurity = true;
        if(prop.equals("0"))
            bCheckSecurity = false;

        // see if the system scope is one we know.
        IdentityScope scope = IdentityScope.getSystemScope();

        if (scope instanceof IdentityDatabase) {
            this.scope = (IdentityDatabase)scope;
            debug("installing " + scope + " as the scope for signers.");
        } else  {
            debug("no signer scope found.");
        }
        loadedClasses = new Hashtable();
    }

//      /**
//       * True if called directly from an applet.
//       */
//      boolean fromApplet() {
//      return classLoaderDepth() == 1;
//      }

//      /**
//       * This method takes a set of signers and returns true if
//       * this set of signers implies that a class is trusted.
//       * In this implementation, it returns true if any of the
//       * signers is a SystemIdentity which is trusted.
//       */
//      protected boolean assessTrust(Object[] signers) {
//      /* Remind: do we want to actually look into the scope here? */

//      for (int i = 0; i < signers.length; i++) {

//          if (signers[i] instanceof SystemIdentity) {
//          SystemIdentity sysid = (SystemIdentity)signers[i];
//          if (sysid.isTrusted()) {
//              return true;
//          }

//          } else if (signers[i] instanceof SystemSigner) {
//          SystemSigner sysid = (SystemSigner)signers[i];
//              if (sysid.isTrusted()) {
//                  return true;
//              }
//          }
//      }

//      return false;
//      }

//      /**
//       * True if called indirectly from an <it>untrusted</it> applet.
//       */
    boolean inApplet() {
        return inClassLoader();
    }

    /**
     * The only variable that currently affects whether an applet can
     * perform certain operations is the host it came from.
     */
    public Object getSecurityContext() {
        ClassLoader loader = currentClassLoader();

        if (loader == null) {
            return null;
        } else
        if (loader instanceof ClassContextImpl) {
            ClassContext appletLoader = (ClassContextImpl)loader;
            return appletLoader.getBase();
        } else {
            throw(new SandboxSecurityException("getsecuritycontext.unknown"));
//              return null;
        }
    }

    /**
     * Applets are not allowed to create class loaders, or even execute any
     * of ClassLoader's methods. The name of this method should be changed to
     * checkClassLoaderOperation or somesuch.
     */
    public synchronized void checkCreateClassLoader() {
        if( bCheckSecurity && !isSecureLoader() ) {
            if (classLoaderDepth() == 2)
                throw(new SandboxSecurityException("checkcreateclassloader"));
        }
    }

    /**
     * Returns true if this threadgroup is in the applet's own thread
     * group. This will return false if there is no current class
     * loader.
     */
    protected boolean inThreadGroup(ThreadGroup g) {
        ClassLoader loader = currentClassLoader();

        /* If this class wasn't loaded by an AppletClassLoader, we have
           not eay of telling, for now. */

        if (loader instanceof ClassContextImpl) {
            ClassContext appletLoader = (ClassContextImpl)loader;
            ThreadGroup appletGroup = appletLoader.getThreadGroup();
            return appletGroup.parentOf(g);
        }
        return false;
    }

    /**
     * Returns true of the threadgroup of thread is in the applet's
     * own threadgroup.
     */
    protected boolean inThreadGroup(Thread thread) {
        return inThreadGroup(thread.getThreadGroup());
    }

    /**
     * Applets are not allowed to manipulate threads outside
     * applet thread groups.
     */
    public synchronized void checkAccess(Thread t) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if (classLoaderDepth()==3 && (! inThreadGroup(t))) {
                throw(new SandboxSecurityException("checkaccess.thread"));
            }
        }
    }

    /**
     * Applets are not allowed to manipulate thread groups outside
     * applet thread groups.
     */
    public synchronized void checkAccess(ThreadGroup g) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if (classLoaderDepth() == 4 && (! inThreadGroup(g))) {
                throw(new SandboxSecurityException("checkaccess.threadgroup", g.toString()));
            }
        }
    }

    /**
     * Applets are not allowed to exit the VM.
     */
    public synchronized void checkExit(int status) {
        if( bNoExit ) {
            throw(new SandboxSecurityException("checkexit", String.valueOf(status)));
        }
        if( bCheckSecurity && !isSecureLoader() ) {
            if( inApplet() ) {
                throw(new SandboxSecurityException("checkexit", String.valueOf(status)));
            }
        }
    }

    /**
     * Applets are not allowed to fork processes.
     */
    public synchronized void checkExec(String cmd) {
        if( bCheckSecurity && !isSecureLoader()) {
            if (inApplet()) {
                throw(new SandboxSecurityException("checkexec", cmd));
            }
        }
    }

    /**
     * Applets are not allowed to link dynamic libraries.
     */
    public synchronized void checkLink(String lib) {
        if( bCheckSecurity && !isSecureLoader() ) {
            switch (classLoaderDepth()) {
              case 2: // Runtime.load
              case 3: // System.loadLibrary
                    throw(new SandboxSecurityException("checklink", lib));
              default:
                break;
            }
        }
    }

    /**
     * Applets are not allowed to access the entire system properties
     * list, only properties explicitly labeled as accessible to applets.
     */
    public synchronized void checkPropertiesAccess() {
        if( bCheckSecurity && !isSecureLoader() ) {
            if (classLoaderDepth() == 2) {
                throw(new SandboxSecurityException("checkpropsaccess"));
            }
        }
    }

    /**
     * Applets can access the system property named by <i>key</i>
     * only if its twin <i>key.applet</i> property is set to true.
     * For example, the property <code>java.home</code> can be read by
     * applets only if <code>java.home.applet</code> is <code>true</code>.
     */
    public synchronized void checkPropertyAccess(String key) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if (classLoaderDepth() == 2) {
                String prop = System.getProperty(key + ".applet");
                boolean allow = new Boolean(prop).booleanValue();
                if ( !allow ) {
                    throw(new SandboxSecurityException("checkpropsaccess.key", prop));
                }
            }
        }
    }

    /**
     * Parse an ACL. Deals with "~" and "+"
     */
    void parseACL(Vector v, String path, String defaultPath) {
        String sep = System.getProperty("path.separator");
        StringTokenizer t = new StringTokenizer(path, sep);

        while (t.hasMoreTokens()) {
            String dir = t.nextToken();
            if (dir.startsWith("~")) {
            v.addElement(System.getProperty("user.home") +
                     dir.substring(1));
            } else if (dir.equals("+")) {
            if (defaultPath != null) {
                parseACL(v, defaultPath, null);
            }
            } else {
            v.addElement(dir);
            }
        }
    }

    /**
     * Parse an ACL.
     */
    String[] parseACL(String path, String defaultPath) {
        if (path == null) {
            return new String[0];
        }
        if (path.equals("*")) {
            return null;
        }
        Vector v = new Vector();
        parseACL(v, path, defaultPath);

        String acl[] = new String[v.size()];
        v.copyInto(acl);
        return acl;
    }

    /**
     * Initialize ACLs. Called only once.
     */
    void initializeACLs() {
        readACL = parseACL(System.getProperty("acl.read"),
                   System.getProperty("acl.read.default"));
        writeACL = parseACL(System.getProperty("acl.write"),
                    System.getProperty("acl.write.default"));
        initACL = true;
    }

    /**
     * Check if an applet can read a particular file.
     */
    public synchronized void checkRead(String file) {
        if( bCheckSecurity && !isSecureLoader() ) {
            ClassLoader loader = currentClassLoader();

            /* If no class loader, it's a system class. */
            if (loader == null)
                return;

            /* If not an AppletClassLoader, we don't know what to do */
            if (! (loader instanceof ClassContextImpl))
                throw(new SandboxSecurityException("checkread.unknown", file));
            ClassContext appletLoader = (ClassContextImpl)loader;
            URL base = appletLoader.getBase();
            checkRead(file, base);
        }
    }

    public synchronized void checkRead(String file, URL base) {
        if( bCheckSecurity && (base != null) && !isSecureLoader() ) {
            if (!initACL)
                initializeACLs();
            if (readACL == null)
                return;

            String realPath = null;
            try {
                realPath = (new File(file)).getCanonicalPath();
            } catch (IOException e) {
                throw(new SandboxSecurityException("checkread.exception1", e.getMessage(), file));
            }

            for (int i = readACL.length ; i-- > 0 ;) {
                if (realPath.startsWith(readACL[i]))
                    return;
            }

            // if the applet is loaded from a file URL, allow reading
            // in that directory
            if (base.getProtocol().equals("file")) {
                String dir = null;
                try {
                    dir = (new File(base.getFile()).getCanonicalPath());
                } catch (IOException e) { // shouldn't happen
                    throw(new SandboxSecurityException("checkread.exception2", e.toString()));
                }
                if (realPath.startsWith(dir))
                    return;
            }
              throw new SandboxSecurityException("checkread", file, realPath);
        }
    }

    /**
     * Checks to see if the current context or the indicated context are
     * both allowed to read the given file name.
     * @param file the system dependent file name
     * @param context the alternate execution context which must also
     * be checked
     * @exception  SecurityException If the file is not found.
     */
    public void checkRead(String file, Object context) {
        checkRead(file);
        if (context != null)
            checkRead(file, (URL) context);
    }

    /**
     * Check if an applet can write a particular file.
     */
    public synchronized void checkWrite(String file) {
        if( bCheckSecurity && inApplet() && !isSecureLoader() ) {
            if (!initACL)
                initializeACLs();
            if (writeACL == null)
                return;

            String realPath = null;
            try {
                realPath = (new File(file)).getCanonicalPath();
            } catch (IOException e) {
                throw(new SandboxSecurityException("checkwrite.exception", e.getMessage(), file));
            }

            for (int i = writeACL.length ; i-- > 0 ;) {
                if (realPath.startsWith(writeACL[i]))
                    return;
            }
            throw(new SandboxSecurityException("checkwrite", file, realPath));
        }
    }

    /**
     * Applets are not allowed to open file descriptors unless
     * it is done through a socket, in which case other access
     * restrictions still apply.
     */
    public synchronized void checkRead(FileDescriptor fd) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if( (inApplet() && !inClass("java.net.SocketInputStream") ) || (!fd.valid()) )
                throw(new SandboxSecurityException("checkread.fd"));
        }
    }

    /**
     * Applets are not allowed to open file descriptors unless
     * it is done through a socket, in which case other access
     * restrictions still apply.
     */
    public synchronized void checkWrite(FileDescriptor fd) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if( (inApplet() && !inClass("java.net.SocketOutputStream")) || (!fd.valid()) )
                throw(new SandboxSecurityException("checkwrite.fd"));
        }
    }

    /**
     * Applets can only listen on unpriveleged ports > 1024
     * A port of 0 denotes an ephemeral system-assigned port
     * Which will be outside this range.  Note that java sockets
     * take an int and ports are really a u_short, but range
     * checking is done in ServerSocket & DatagramSocket, so the port policy
     * cannot be subverted by ints that wrap around to an illegal u_short.
     */
    public synchronized void checkListen(int port) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if (inApplet() && port > 0 && port < PRIVELEGED_PORT)
                throw(new SandboxSecurityException("checklisten", String.valueOf(port)));
        }
    }

    /**
     * Applets can accept connectionions on unpriveleged ports, from
     * any hosts they can also connect to (typically host-of-origin
     * only, depending on the network security setting).
     */
    public synchronized void checkAccept(String host, int port) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if( inApplet() && port < PRIVELEGED_PORT )
                throw(new SandboxSecurityException("checkaccept", host, String.valueOf(port)));
            checkConnect(host, port);
        }
    }

    /**
     * Check if an applet can connect to the given host:port.
     */
    public synchronized void checkConnect(String host, int port) {
        if(bCheckSecurity && !isSecureLoader() ) {
            ClassLoader loader = currentClassLoader();
            if (loader == null)
                return; // Not called from an applet, so it is ok

            // REMIND: This is only appropriate for our protocol handlers.
            int depth = classDepth("sun.net.www.http.HttpClient");
            if (depth > 1)
                return; // Called through our http protocol handler

            if(getInCheck())
                return;

            if (loader instanceof ClassContextImpl) {
                ClassContext appletLoader = (ClassContextImpl)loader;
                checkConnect(appletLoader.getBase().getHost(), host);
            } else {
                throw(new SandboxSecurityException("checkconnect.unknown"));
            }
        }
    }

    /**
     * Checks to see if the applet and the indicated execution context
     * are both allowed to connect to the indicated host and port.
     */
    public void checkConnect(String host, int port, Object context) {
        checkConnect(host, port);
        if (context != null)
            checkConnect(((URL) context).getHost(), host);
    }

    public synchronized void checkConnect(String fromHost, String toHost, boolean trustP) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if (fromHost == null)
                return;

            switch (networkMode) {
              case NETWORK_NONE:
                    throw(new SandboxSecurityException("checkconnect.networknone", fromHost, toHost));

              case NETWORK_HOST:
                /*
                 * The policy here is as follows:
                 *
                 * - if the strings match, and we know the IP address for it
                 * we allow the connection. The calling code downstream will
                 * substitute the IP in their request to the proxy if needed.
                 * - if the strings don't match, and we can get the IP of
                 * both hosts then
                 *   - if the IPs match, we allow the connection
                 *   - if they don't we throw(an exception
                 * - if the string match works and we don't know the IP address
                 * then we consult the trustProxy property, and if that is true,
                 * we allow the connection.
                 * set inCheck so InetAddress knows it doesn't have to
                 * check security.
                 */
                try {
                    inCheck = true;
                    InetAddress toHostAddr, fromHostAddr;
                     if (!fromHost.equals(toHost)) {
                        try {
                            // the only time we allow non-matching strings
                            // is when IPs and the IPs match.
                            toHostAddr = InetAddress.getByName(toHost);
                            fromHostAddr = InetAddress.getByName(fromHost);

                            if( fromHostAddr.equals(toHostAddr) )
                                return;
                            else
                                throw(new SandboxSecurityException("checkconnect.networkhost1", toHost, fromHost));

                        } catch (UnknownHostException e) {
                            throw(new SecurityException("checkconnect.networkhost2" + toHost + fromHost));
//                              throw(new SandboxSecurityException("checkconnect.networkhost2", toHost, fromHost));
                        }
                    } else {
                        try {
                            toHostAddr = InetAddress.getByName(toHost);

                            // strings match: if we have IP, we're homefree,
                            // otherwise we check the properties.
                            return;
                        // getBoolean really defaults to false.
                        } catch (UnknownHostException e) {
                        if( trustP )
                            return;
                        else
                            throw(new SandboxSecurityException("checkconnect.networkhost3", toHost));
                        }
                    }
                } finally {
                    inCheck = false;
                }

              case NETWORK_UNRESTRICTED:
                return;
            }
            throw(new SandboxSecurityException("checkconnect", fromHost, toHost));
        }
    }



    /**
     * Check if an applet from a host can connect to another
     * host. This usually means that you need to determine whether
     * the hosts are inside or outside the firewall. For now applets
     * can only access the host they came from.
     */
    public synchronized void checkConnect(String fromHost, String toHost) {
        checkConnect(fromHost, toHost, Boolean.getBoolean("trustProxy"));
    }

    /**
     * Checks to see if top-level windows can be created by the caller.
     */
    public synchronized boolean checkTopLevelWindow(Object window) {
        if( bCheckSecurity && inClassLoader() && !isSecureLoader() ) {
            /* XXX: this used to return depth > 3. However, this lets */
            /* some applets create frames without warning strings. */
            return false;
        }
        return true;
    }

    /**
     * Check if an applet can access a package.
     */
    public synchronized void checkPackageAccess(String pkg) {
        if( bCheckSecurity && inClassLoader() && !isSecureLoader() ) {
            if( pkg.equals( "stardiv.applet" )
            // Das AWT von StarDivision
              || pkg.equals( "stardiv.look" )
              || pkg.equals( "netscape.javascript" ) )
                return;

            final String forbidden[] = new String[]{
                "com.sun.star.uno",
                "com.sun.star.lib.uno",
                "com.sun.star.comp.connections",
                "com.sun.star.comp.loader",
                "com.sun.star.comp.servicemanager"
            };


            for(int j = 0; j < forbidden.length; ++ j) {
                if(pkg.startsWith(forbidden[j]))
                    throw(new SandboxSecurityException("checkpackageaccess2", pkg));
            }

            int i = pkg.indexOf('.');
            while (i > 0) {
                String subpkg = pkg.substring(0,i);
                if( Boolean.getBoolean("package.restrict.access." + subpkg) )
                    throw(new SandboxSecurityException("checkpackageaccess", pkg));
                i = pkg.indexOf('.',i+1);
            }
        }
    }

    /**
     * Check if an applet can define classes in a package.
     */
    public synchronized void checkPackageDefinition(String pkg) {
        return;
/*
    if (!inClassLoader())
        return;
        int i = pkg.indexOf('.');

        while (i > 0) {
        String subpkg = pkg.substring(0,i);
        if (Boolean.getBoolean("package.restrict.definition." + subpkg)) {
                throw(new SandboxSecurityException("checkpackagedefinition", pkg);
        }
        i = pkg.indexOf('.',i+1);
        }
*/
    }


    /**
     * Check if an applet can set a networking-related object factory.
     */
    public synchronized void checkSetFactory() {
        if( bCheckSecurity && inApplet() && !isSecureLoader() )
            throw(new SandboxSecurityException("cannotsetfactory"));
    }

    /**
     * Check if client is allowed to reflective access to a member or
     * a set of members for the specified class.  Once initial access
     * is granted, the reflected members can be queried for
     * identifying information, but can only be <strong>used</strong>
     * (via get, set, invoke, or newInstance) with standard Java
     * language access control.
     *
     * <p>The policy is to deny <em>untrusted</em> clients access to
     * <em>declared</em> members of classes other than those loaded
     * via the same class loader.  All other accesses are granted.
     *
     * XXX: Should VerifyClassAccess here?  Should Class.forName do it?
     */
    public void checkMemberAccess(Class clazz, int which) {
        if( bCheckSecurity && !isSecureLoader() ) {
            if( which != java.lang.reflect.Member.PUBLIC ) {
                ClassLoader currentLoader = currentClassLoader();
                if( currentLoader != null && (classLoaderDepth() <= 3) )
                    /* Client is an untrusted class loaded by currentLoader */
                    if( currentLoader != clazz.getClassLoader() )
                        throw(new SandboxSecurityException("checkmemberaccess"));
            }
        }
    }

    /**
     * Checks to see if an applet can initiate a print job request.
     */
    public void checkPrintJobAccess() {
        if( bCheckSecurity && inApplet() && !isSecureLoader() )
            throw(new SandboxSecurityException("checkgetprintjob"));
    }

    /**
     * Checks to see if an applet can get System Clipboard access.
     */
    public void checkSystemClipboardAccess() {
        if( bCheckSecurity && inApplet() && !isSecureLoader() )
            throw(new SandboxSecurityException("checksystemclipboardaccess"));
    }

    /**
     * Checks to see if an applet can get EventQueue access.
     */
    public void checkAwtEventQueueAccess() {
          if( bCheckSecurity && inClassLoader() && !isSecureLoader() ) {
//              throw(new SandboxSecurityException("checkawteventqueueaccess"));
        }
    }

    /**
     * Checks to see if an applet can perform a given operation.
     */
    public void checkSecurityAccess(String action) {
        if( bCheckSecurity && inApplet() && !isSecureLoader() )
            throw(new SandboxSecurityException("checksecurityaccess", action));
    }

    /**
     * Returns the thread group of the applet. We consult the classloader
     * if there is one.
     */
    public ThreadGroup getThreadGroup() {
        /* First we check if any classloaded thing is on the stack. */
        ClassLoader loader = currentClassLoader();
        if (loader != null && (loader instanceof ClassContextImpl)) {
            if( inThreadGroup( Thread.currentThread() ) )
                return Thread.currentThread().getThreadGroup();
            ClassContextImpl appletLoader = (ClassContextImpl)loader;
            return appletLoader.getThreadGroup();
        } else
            return super.getThreadGroup();
    }

    public void debug(String s) {
        if( debug )
            System.err.println(s);
    }

    private boolean isSecureLoader() {
        ClassLoader loader = currentClassLoader();
        if (loader != null) {
            if (loader instanceof ClassContextImpl) {
                return !((ClassContextImpl) loader).checkSecurity();
            } else {
                return true; // fremder ClassLoader: kann machen was er will
            }
        } else {
            return true;
        }
    }
}


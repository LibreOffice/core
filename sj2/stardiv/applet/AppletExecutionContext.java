/*************************************************************************
 *
 *  $RCSfile: AppletExecutionContext.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2001-02-23 16:10:08 $
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

package stardiv.applet;

import java.applet.Applet;
import java.applet.AppletStub;
import java.applet.AppletContext;
import java.applet.AudioClip;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Panel;
import java.awt.Toolkit;

import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;

import java.net.URL;
import java.net.MalformedURLException;

import java.util.Hashtable;
import java.util.Vector;

import sun.misc.Queue;

import com.sun.star.lib.sandbox.ClassContextProxy;
import com.sun.star.lib.sandbox.ExecutionContext;
import com.sun.star.lib.sandbox.JarEntry;
import com.sun.star.lib.sandbox.ResourceProxy;

import com.sun.star.lib.sandbox.CodeSource;
//import com.sun.star.lib.sandbox.PermissionCollection;
//import com.sun.star.lib.sandbox.ProtectionDomain;
//import com.sun.star.lib.sandbox.RuntimePermission;
//import com.sun.star.lib.sandbox.FilePermission;
//import com.sun.star.lib.sandbox.SocketPermission;


public final class AppletExecutionContext extends ExecutionContext
        implements AppletStub, LiveConnectable
{
    private static final boolean DEBUG = true; // Enable / disable debug output

    private Applet _applet;
    private Container _container;

    private DocumentProxy _documentProxy;
    private Hashtable _parameters;

    private String _className;
    private Vector _jarResourceProxys = new Vector();

    private URL _documentURL = null;
    private URL _baseURL = null;

//      private ProtectionDomain _protectionDomain;
//      private PermissionCollection _permissionCollection;

    private Toolkit _toolkit;

    //************** C++ WRAPPER ******************
    private long    pCppJSbxObject;

    synchronized public void ClearNativeHandle() {
        pCppJSbxObject = 0;
        if(DEBUG)System.err.println("### AppletExecutionContext.ClearNativeHandle");
    }

    public AppletExecutionContext(long pCppJSbxObject) {
        this.pCppJSbxObject = pCppJSbxObject;
    }
    //************** C++ WRAPPER ******************

    public AppletExecutionContext( URL documentURL,
                                   Hashtable parameters,
                                   Container container,
                                   long pCppJSbxObject)
    {
        this(pCppJSbxObject);

        if(DEBUG) System.err.println("#### AppletExecutionContext.<init>:" + documentURL + " " + parameters + " " + container + " " + pCppJSbxObject);
        _documentURL = documentURL;
        _parameters = parameters;
        _container = container;

        _toolkit = container.getToolkit();

        _documentProxy = DocumentProxy.getDocumentProxy(documentURL, _toolkit);
        addObserver(_documentProxy);
    }

    public void init() {
        if(DEBUG) System.err.println("#####" + getClass().getName() + ".init()");

        _baseURL = null;

        try {
            String codeBase = getParameter("codebase");

            if (!codeBase.endsWith("/")) {
                codeBase += "/";
            }
            _baseURL = new URL(_documentURL, codeBase);
        }
        catch (MalformedURLException e) {
            try {
                String file = _documentURL.getFile();
                int i = file.lastIndexOf('/');

                if (i > 0 && i < file.length() - 1) {
                    _baseURL = new URL(_documentURL, file.substring(0, i + 1));
                }
            }
            catch (MalformedURLException e2) {
                _baseURL = _documentURL;
            }
        }

        if(_baseURL == null)
            _baseURL = _documentURL;

        _className = getParameter("code");
        String defaultExtension = ".class";
        String oldExtension = ".java";

        int extensionIndex = _className.lastIndexOf('.');
          String extension = "";

        if (extensionIndex != -1) {
            extension = _className.substring(extensionIndex);

            if(!extension.equals(defaultExtension) && !extension.equals(oldExtension)) {
                extension = defaultExtension;
            }
            else
                _className = _className.substring(0, extensionIndex);
        }

        String nm = "applet-" + _className;

          _documentProxy.addExecutionContext(this, _className);

//          _permissionCollection = new PermissionCollection();
//          _protectionDomain = new ProtectionDomain(new CodeSource(_baseURL, null), _permissionCollection);

        super.init(nm, ClassContextProxy.create(_baseURL, null, null));
//          super.init(nm, ClassContextProxy.create(_baseURL, _protectionDomain, null));

//          _permissionCollection.add(new RuntimePermission("modifyThreadGroup", getThreadGroup()));
//          _permissionCollection.add(new SocketPermission(SocketPermission.NETWORK_APPLET, _baseURL.getHost()));

//          try {
//              if(_baseURL.getProtocol().equals("file")) // allow read acces for applet directory
//                  _permissionCollection.add(new FilePermission(new File(_baseURL.getFile()).getCanonicalPath(), "read"));
//          }
//          catch(IOException eio) {
//          }
    }

    void sDispose(long timeout) {
        if(DEBUG) System.err.println("#### AppletExecutionContext.sDispose");

        _container = null;
        _jarResourceProxys = null;

        super.dispose(timeout);
    }

    public void dispose(long timeout) {
        sDispose(timeout);  // call direct

/*  Deadlock with TKT
        class DisposeEvent extends java.awt.AWTEvent
                implements java.awt.peer.ActiveEvent,
                java.awt.ActiveEvent
        {
            private AppletExecutionContext executionContext;
            private long timeout;

            public DisposeEvent(AppletExecutionContext executionContext, long timeout) {
                super(executionContext, 0);

                this.executionContext = executionContext;
                this.timeout = timeout;
            }

            public void dispatch() {
                executionContext.sDispose(timeout);
            }
        }

        toolkit.getSystemEventQueue().postEvent(new DisposeEvent(this, timeout));
*/
    }

    protected int getIntParameter(String name) {
        int value = 0;
        String string = getParameter(name);
        if(string != null)
            value = Integer.valueOf(string).intValue();

        return value;
    }

    protected void xload()
            throws  ClassNotFoundException,
                    InstantiationException,
                    IllegalAccessException
    {
        String archives = getParameter("archive");

        try {
            if(archives != null) {
                int index = archives.indexOf(",");
                while(index > -1) {
                    try { // try to load archive
                        loadArchive(archives.substring(0, index));
                    }
                    catch(MalformedURLException malformedURLException) {
                        System.err.println("#### can't load archive:" + archives.substring(0, index));
                    }
                    catch(IOException ioException) {
                        System.err.println("#### can't load archive:" + archives.substring(0, index) + " reason:" + ioException);
                    }

                    archives = archives.substring(index + 1).trim();

                    index = archives.indexOf(",");
                }
                if(archives.length() > 0) loadArchive(archives);
            }

            Class appletClass = classContext.loadClass(_className);
            synchronized(_className) {
                _applet = (Applet)appletClass.newInstance();
                _applet.setStub(this);
                _className.notifyAll();
            }
        }
        catch(IOException eio) {
            throw new ClassNotFoundException(eio.getMessage());
        }
    }

    protected void xinit() {
        java.awt.Dimension size = new Dimension(getIntParameter("width"), getIntParameter("height"));

          _container.setLayout(null);
          _container.setVisible(true);
        _container.setSize(size);
          _container.add(_applet);

        _applet.setVisible(false);
        _applet.setSize(size);

          _container.validate();

        _applet.init();
    }

    protected void xstart() {
        _applet.setVisible(true);
        _container.validate();

        _applet.start();
    }

    protected void xstop() {
        _applet.stop();
    }

    protected void xdestroy() {
        _applet.destroy();
        _applet.setVisible(false);
        _applet.setStub(null);

          _documentProxy.removeExecutionContext(_applet.getClass().getName());
    }

    protected void xdispose() {
        if(_container != null)
            _container.remove(_applet);

        _applet = null;
    }

    private void loadArchive(String archive) throws MalformedURLException, IOException {
        ResourceProxy jarResourceProxy = ResourceProxy.load(new URL(_baseURL, archive), null /*_protectionDomain*/);
        jarResourceProxy.loadJar(_baseURL);
        _jarResourceProxys.addElement(jarResourceProxy);
    }

    public Applet getApplet() {
        synchronized(_className) {
            if(_applet == null) {
                if(DEBUG)System.err.println("#### AppletExecutionContext.getApplet - waiting for applet");
                try {
                    _className.wait();
                }
                catch(InterruptedException interruptedException) {
                    System.err.println("#### AppletExecutionContext.getApplet:" + interruptedException);
                }
                if(DEBUG)System.err.println("#### AppletExecutionContext.getApplet - got it");
            }
        }
        return _applet;
    }

    /*
     * Methods for AppletStub interface
     */
    public void appletResize(int width, int height) {
        _applet.setSize(new java.awt.Dimension(width, height));
    }

    public AppletContext getAppletContext() {
        return _documentProxy;
    }

    public URL getCodeBase() {
        return classContext.getBase();
    }

    public URL getDocumentBase() {
        return _documentProxy.getDocumentBase();
    }

    public String getParameter(String name) {
        String string = (String)_parameters.get(name.toLowerCase());
        if(string != null)
            string = string.trim();

        return string;
    }

    public boolean isActive() {
        return getStatus() == STARTED && pCppJSbxObject != 0;
    }

    public void finalize() {
        if(DEBUG) System.err.println("#### AppletExecutionContext finalized");
    }

    // sollte eigentlich im DocumentProxy sein, geht aber nicht
    private native void xshowStatus(String status);
    private native void xshowDocument(URL url, String aTarget);

    void printStatus(String status) {
          if(pCppJSbxObject != 0) xshowStatus(status);
    }

    void printDocument(URL url, String aTarget) {
        if(pCppJSbxObject != 0) xshowDocument(url, aTarget);
    }

    native public Object getJavaScriptJSObjectWindow();
}

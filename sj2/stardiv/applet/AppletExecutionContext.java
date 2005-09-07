/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppletExecutionContext.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:16:20 $
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
import java.awt.Window;

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
import com.sun.star.lib.sandbox.SandboxSecurity;
import com.sun.star.lib.sandbox.CodeSource;

import stardiv.controller.SjSettings;

public final class AppletExecutionContext extends ExecutionContext
        implements AppletStub, LiveConnectable
{
    private static final boolean DEBUG = false; // Enable / disable debug output

    private Applet _applet;
    private Container _container;

    private DocumentProxy _documentProxy;
    private Hashtable _parameters;

    private String _className;
    private Vector _jarResourceProxys = new Vector();

    private URL _documentBase = null;
    private URL _baseURL = null;

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

    public AppletExecutionContext( URL documentBase,
                                   Hashtable parameters,
                                   Container container,
                                   long pCppJSbxObject)
    {
        this(pCppJSbxObject);

        if(DEBUG) System.err.println("#### AppletExecutionContext.<init>:" + documentBase + " " + parameters + " " + container + " " + pCppJSbxObject);
        _documentBase = documentBase;
        _parameters = parameters;
        _container = container;

        _toolkit = container.getToolkit();

        _documentProxy = DocumentProxy.getDocumentProxy(documentBase, _toolkit);
        addObserver(_documentProxy);
    }

    public void init() {
        _baseURL = null;

        try {
            String codeBase = getParameter("codebase");

            if (!codeBase.endsWith("/")) {
                codeBase += "/";
            }
            _baseURL = new URL(_documentBase, codeBase);
        }
        catch (MalformedURLException e) {
            if(DEBUG) System.err.println("#### AppletExecutionContext: Could not create base Url");
        }

        if(_baseURL == null)
            _baseURL = _documentBase;

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".init - baseUrl:" + _baseURL);

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

        super.init(nm, ClassContextProxy.create(_baseURL, null, null, false));

        // Set the property stardiv.security.noExit to true. That value will be used in
        // SjSettings.changeProperties in the constructor of the SecurityManager SandboxSecurity
        if (System.getSecurityManager() == null)
            System.setProperty("stardiv.security.noExit", "true");
        // SjSettings.changeProperties puts a lot of applet relating properties into the system properties
        // and it sets the SecurityManager
        SjSettings.changeProperties( System.getProperties());

        if(DEBUG) System.err.println("#####" + getClass().getName() + ".init: _className=" + _className + " _baseURL=" + _baseURL);
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

                appletResize(_container.getSize().width, _container.getSize().height);

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
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".xdestroy");

        _applet.destroy();
        _applet.setVisible(false);
        _applet.setStub(null);

          _documentProxy.removeExecutionContext(_applet.getClass().getName());
    }

    protected void xdispose() {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".xdispose");

        if(_container != null) {
            _container.remove(_applet);

            if(_container instanceof Window)
                ((Window)_container).dispose();
        }

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
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".appletResize: " + width + " " + height);

        _container.setSize(width, height);
        if(_applet != null)
            _applet.setSize(width, height);
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

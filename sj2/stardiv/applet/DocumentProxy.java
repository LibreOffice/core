/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentProxy.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:16:59 $
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

import java.awt.Toolkit;
import java.awt.Image;

import java.applet.Applet;
import java.applet.AppletContext;
import java.applet.AudioClip;

import java.io.IOException;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Observer;
import java.util.Observable;
import java.util.Iterator;
import java.io.InputStream;

import java.net.URL;

import com.sun.star.lib.sandbox.Cachable;
import com.sun.star.lib.sandbox.ExecutionContext;
import com.sun.star.lib.sandbox.WeakRef;
import com.sun.star.lib.sandbox.WeakTable;
import com.sun.star.lib.sandbox.ResourceProxy;

public class DocumentProxy implements AppletContext, Cachable, Observer, LiveConnectable  {
    static private int instances;

    synchronized static public DocumentProxy getDocumentProxy(URL url, Toolkit toolkit) {
        DocumentProxy documentProxy = (DocumentProxy)WeakTable.get("Document: " + url);

        if(documentProxy == null) {
            documentProxy = new DocumentProxy(url, toolkit);
            WeakTable.put("Document: " + url, documentProxy);
        }

        return documentProxy;
    }

    // AppletContext. This method is new since 1.4. We insert it so as to
    // have the project buildable
    public void setStream( String key,InputStream stream)
        throws java.io.IOException {
    }
    // AppletContext. This method is new since 1.4. We insert it so as to
    // have the project buildable
    public InputStream getStream( String key) {
        return null;
    }
    // AppletContext. This method is new since 1.4. We insert it so as to
    // have the project buildable
    public Iterator getStreamKeys() {
        return null;
    }


    /*
    ** interface cachable methods
    */
    private Document document;
    private WeakRef weakRef;

    public DocumentProxy() {
        instances ++;
    }

    public Object getHardObject() {
        return document;
    }

    public void setWeakRef(WeakRef weakRef) {
        document = (Document)weakRef.getRef();

        weakRef.incRefCnt();
        this.weakRef = weakRef;
    }

    public void finalize() {
        weakRef.decRefCnt();
        instances --;
    }

    /*
    ** DocumentProxy methods
    */
    private Toolkit toolkit;

    private DocumentProxy(URL url, Toolkit toolkit) {
        this();
        document = new Document(url, toolkit);
    }

    void addExecutionContext(ExecutionContext executionContext, String name) {
        document.addExecutionContext(executionContext, name);
    }

    void removeExecutionContext(String name) {
        document.removeExecutionContext(name);
    }

    public URL getDocumentBase() {
        return document.getDocumentBase();
    }

    /*
    ** AppletContext interface methods
    */
    public Applet getApplet(String name) {
        return ((AppletExecutionContext)document.getExecutionContext(name)).getApplet();
    }

    public Enumeration getApplets() {
        return new Enumeration() {
            Enumeration contexts = document.getExecutionContexts();

            public boolean hasMoreElements() {
                return contexts.hasMoreElements();
            }

            public Object nextElement() {
                return ((AppletExecutionContext)contexts.nextElement()).getApplet();
            }
        };
    }

    public AudioClip getAudioClip(URL url) {
        return document.getAudioClip(url);
    }

    public Image getImage(URL url) {
        return document.getImage(url);
    }

    public void showDocument(URL url) {
        document.showDocument(url);
    }

    /**
     * Get the javascript environment for this applet.
     */
    /*
      public native Object getJavaScriptJSObjectWindow();
      public native void appletResize( int width, int height );
      public native void showDocument( URL url, String aTarget );
      public native void showStatus( String status );
    */

    public void showDocument(URL url, String aTarget) {
        document.showDocument(url, aTarget);
    }

    public void showStatus(String status) {
        document.showStatus(status);
    }

    public void update(Observable observable, Object object) {
        showStatus((String)object);
    }

    public Object getJavaScriptJSObjectWindow() {
        return document.getJavaScriptJSObjectWindow();
    }
}


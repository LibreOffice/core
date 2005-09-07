/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Document.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:16:36 $
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

import java.awt.Image;

//  import java.applet.Applet;
//  import java.applet.AppletContext;
//  import java.applet.AudioClip;


import java.io.IOException;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Observer;
import java.util.Observable;

import java.net.URL;

import java.applet.Applet;
import java.applet.AppletContext;
import java.applet.AudioClip;

import com.sun.star.lib.sandbox.ExecutionContext;
import com.sun.star.lib.sandbox.ResourceProxy;

class Document implements LiveConnectable {
    private Hashtable executionContexts = new Hashtable();
    private URL documentBase = null;
    private java.awt.Toolkit toolkit;

    Document(URL url, java.awt.Toolkit toolkit) {
    // Create the document base.
    //For example, suppose an applet is contained within the document:
    //http://java.sun.com/products/jdk/1.2/index.html
    //The document base is:
    //http://java.sun.com/products/jdk/1.2/

        String s= url.toString();
        int index= s.lastIndexOf('/');
        if( index != -1)
        {
            s=s.substring(0, index + 1);
        }
        try{
            documentBase = new URL(s);
        }catch(Exception e){
        }
        this.toolkit = toolkit;
    }

    void addExecutionContext(ExecutionContext executionContext, String name) {
        executionContexts.put(name, executionContext);
    }

    void removeExecutionContext(String name) {
        executionContexts.remove(name);
    }

    Enumeration getExecutionContexts() {
        return executionContexts.elements();
    }

    URL getDocumentBase() {
        return documentBase;
    }

    ExecutionContext getExecutionContext(String name) {
        return (ExecutionContext)executionContexts.get(name);
    }

    Enumeration getExcutionContexts() {
        return executionContexts.elements();
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

    public AudioClip getAudioClip(URL url) {
        ResourceProxy resourceProxy = ResourceProxy.load(url, null);
        AudioClip audioClip = resourceProxy.getAudioClip();

        return audioClip;
    }

    public Image getImage(URL url) {
        ResourceProxy resourceProxy = ResourceProxy.load(url, null);
        Image image = toolkit.createImage(resourceProxy.getImageProducer());

        return image;
    }

    AppletExecutionContext getAppletExecutionContext() {
        AppletExecutionContext appletExecutionContext = null;

        for(Enumeration e = executionContexts.elements(); e.hasMoreElements();) {
            Object object = e.nextElement();
            if(object instanceof AppletExecutionContext) {
                appletExecutionContext = (AppletExecutionContext)object;
            }
        }
        return appletExecutionContext;
    }


    void showDocument(URL url, String aTarget) {
        AppletExecutionContext appletExecutionContext = getAppletExecutionContext();
        if(appletExecutionContext != null) appletExecutionContext.printDocument(url, aTarget);
    }

    public void showDocument(URL url) {
         showDocument(url, "_top");
    }

    void showStatus(String status) {
        status = (status == null) ? "" : status;

        AppletExecutionContext appletExecutionContext = getAppletExecutionContext();
          if(appletExecutionContext != null) appletExecutionContext.printStatus(status);
    }

    public Object getJavaScriptJSObjectWindow() {
        Object object = null;

        AppletExecutionContext appletExecutionContext = getAppletExecutionContext();
          if(appletExecutionContext != null)
            object = appletExecutionContext.getJavaScriptJSObjectWindow();

        return object;
    }
}

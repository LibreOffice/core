/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Document.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

/*************************************************************************
 *
 *  $RCSfile: Document.java,v $
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
        documentBase = url;
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

/*************************************************************************
 *
 *  $RCSfile: ResourceProxy.java,v $
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

import java.applet.AudioClip;

import java.awt.image.ImageProducer;

import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import java.net.URL;
import java.net.URLConnection;
import java.net.MalformedURLException;

import java.util.Enumeration;
import java.util.Vector;

import sun.awt.image.URLImageSource;
import sun.applet.AppletAudioClip;

public class ResourceProxy implements Resource, Cachable {
    private static final boolean DEBUG = false;
    private static int instances;

    static boolean isResource(URL url) {
        boolean exists = false;

        ResourceProxy resourceProxy = (ResourceProxy)WeakTable.get(url);
        exists = resourceProxy != null;

        if(!exists) { // not cache
            // Check if the resource exists.
            // It almost works to just try to do an openConnection() but
            // HttpURLConnection will return true on HTTP_BAD_REQUEST
            // when the requested name ends in ".html", ".htm", and ".txt"
            // and we want to be able to handle these
            //
            // Also, cannot just open a connection for things like FileURLConnection,
            // because they suceed when connecting to a non-existant file.
            // So, in those cases we open and close an input stream.


            try {
                URLConnection conn = url.openConnection();
                if (conn instanceof java.net.HttpURLConnection) {
                    java.net.HttpURLConnection hconn = (java.net.HttpURLConnection) conn;
                    int code = hconn.getResponseCode();

                    exists = (code == java.net.HttpURLConnection.HTTP_OK) || (code < java.net.HttpURLConnection.HTTP_BAD_REQUEST);
                }
                else {
                    // our best guess for the other cases
                    InputStream is = url.openStream();
                    is.close();
                    exists = true;
                }
            }
            catch (Exception ex) {
            }
        }

        return exists;
    }

    static public ResourceProxy load(URL url, ProtectionDomain protectionDomain) {
          if(DEBUG) System.err.println("#### ResourceProxy.loadResource:" + url);

//          protectionDomain = (protectionDomain == null)
//              ? new ProtectionDomain(new CodeSource(url), new PermissionCollection())
//              : protectionDomain;

        ResourceProxy resourceProxy = (ResourceProxy)WeakTable.get(url);

        if(resourceProxy == null) {
            resourceProxy = new ResourceProxy(url, null, protectionDomain);
            WeakTable.put(url, resourceProxy);
        }

        return resourceProxy;
    }

    static public ResourceProxy create(URL url, byte bytes[], ProtectionDomain protectionDomain) {
        ResourceProxy resourceProxy = (ResourceProxy)WeakTable.get(url);

        if(resourceProxy == null) {
            resourceProxy = new ResourceProxy(url, bytes, protectionDomain);
            WeakTable.put(url, resourceProxy);
        }
        else
            System.err.println("WARNING! ResourceProxy.create - resource already loaded:" + resourceProxy);

        return resourceProxy;
    }

    /*
    ** Members
    */
    private Resource resource;
    private WeakRef weakRef;

    /*
    ** interface cachable methods
    */
    public ResourceProxy() {
        instances ++;
    }

    public Object getHardObject() {
        return resource;
    }

    public void setWeakRef(WeakRef weakRef) {
        this.resource = (Resource)weakRef.getRef();
        this.weakRef  = weakRef;

        weakRef.incRefCnt();
    }

    public void finalize(){
        weakRef.decRefCnt();
        instances --;
    }

    /*
    ** resource methods
    */
    private ResourceProxy(URL url, byte bytes[], ProtectionDomain protectionDomain) {
        this();
        resource = new ResourceImpl(url, bytes, protectionDomain);
    }

    public ProtectionDomain getProtectionDomain() {
        return resource.getProtectionDomain();
    }

    public synchronized URL getURL() {
        return resource.getURL();
    }

    public synchronized InputStream getInputStream() throws IOException {
        return resource.getInputStream();
    }

    public synchronized byte[] getBytes() throws IOException {
        return resource.getBytes();
    }

    public synchronized ImageProducer getImageProducer() {
        ImageProducer imageProducer = resource.getImageProducer();

        imageProducer = new ImageProducerProxy(imageProducer);

        Holder holder = (Holder)imageProducer;
        holder.addCargo(this);

        return imageProducer;
    }

    public synchronized AudioClip getAudioClip() {
        AudioClip audioClip = resource.getAudioClip();

        audioClip = new AudioProxy(audioClip);

        if(audioClip instanceof Holder) {
            Holder holder = (Holder)audioClip;
            holder.addCargo(this);
        }

        return audioClip;
    }

    public Object getContent() throws IOException {
        Object content = resource.getContent();

        if(content instanceof Holder)
            ((Holder)content).addCargo(this);

        return content;
    }

    public void loadJar(URL url) throws IOException {
        resource.loadJar(url);
    }
}


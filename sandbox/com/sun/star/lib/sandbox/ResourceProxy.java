/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResourceProxy.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:08:10 $
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


        // This is due to a stupid bug in ImageConsumerQueue,
        // which seems to be introduced in jdk1.3, which
        // forbids me to use my own ImageProducer implementation.
//          imageProducer = new ImageProducerProxy(imageProducer);

//          Holder holder = (Holder)imageProducer;
//          holder.addCargo(this);

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


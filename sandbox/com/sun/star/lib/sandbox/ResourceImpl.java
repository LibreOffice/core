/*************************************************************************
 *
 *  $RCSfile: ResourceImpl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2001-03-13 09:31:04 $
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

import sun.awt.image.ByteArrayImageSource;
import sun.awt.image.URLImageSource;

import sun.applet.AppletAudioClip;

class ResourceImpl implements Resource {
    private static int instances;
    private static final boolean DEBUG = false;

    private Object object;
    private byte bytes[];
    private URL url;
    private String type;
    private ProtectionDomain protectionDomain;

    public ResourceImpl(URL url, byte bytes[], ProtectionDomain protectionDomain) {
        if(DEBUG)System.err.println("#### ResourceImpl.<init>:" + url + " " + bytes + " " + protectionDomain);

        instances ++;

        this.protectionDomain = protectionDomain;
        this.url              = url;
        this.bytes            = bytes;
    }

    public void finalize() {
        instances --;
    }

    public ProtectionDomain getProtectionDomain() {
        return protectionDomain;
    }

    public URL getURL() {
        return url;
    }

    public synchronized InputStream getInputStream() throws IOException {
        if(DEBUG) System.err.print("#### ResourceImpl.getInputStream:" + url);
        InputStream inputStream = null;

        if(bytes == null) {
            if(DEBUG)System.err.println(" - from url");
              URLConnection c = url.openConnection();
            c.setAllowUserInteraction(false);
              inputStream = c.getInputStream();
        }
        else {
            if(DEBUG)System.err.println(" - from bytes");
            inputStream = new ByteArrayInputStream(bytes);
        }

        return inputStream;
    }

    public synchronized byte[] getBytes() throws IOException {
        final int chunkSize = 4096;

        if(bytes == null) {
            InputStream inputStream = getInputStream();

            int read = 0;
            int total = 0;
            Vector chunks = new Vector();

            byte chunk[] = new byte[chunkSize];
            read = inputStream.read(chunk, 0, chunkSize);
            while(read != -1) { // until end of file
                if(read < chunkSize) {
                    byte tmpChunk[] = new byte[read];
                    System.arraycopy(chunk, 0, tmpChunk, 0, tmpChunk.length);
                    chunk = tmpChunk;
                }

                chunks.addElement(chunk);
                chunk = new byte[chunkSize];
                total += read;

                read = inputStream.read(chunk, 0, chunkSize);
            }

            inputStream.close();

            byte tmpBytes[] = new byte[total];
            int index = 0;
            for(int i = 0; i < chunks.size(); i++) {
                chunk = (byte [])chunks.elementAt(i);

                System.arraycopy(chunk, 0, tmpBytes, index, chunk.length);
                index += chunk.length;
            }

            bytes = tmpBytes;
        }
        return bytes;
    }

    public synchronized ImageProducer getImageProducer() {
        if(DEBUG) System.err.println("#### Resource.getImageProducer:");

        ImageProducer imageProducer = null;

        if(object != null)
            imageProducer = (ImageProducer)object;
        else {
            if(bytes != null)
                imageProducer = new ByteArrayImageSource(bytes, 0, bytes.length);
            else
                imageProducer = new URLImageSource(url);

            object = imageProducer;
        }

        return imageProducer;
    }

    public synchronized AudioClip getAudioClip() {
        AudioClip audioClip = null;

        if(object != null)
            audioClip = (AudioClip)object;
        else {
            if(bytes != null)
                audioClip = new AppletAudioClip(bytes);
            else
                audioClip = new AppletAudioClip(url);

            object = audioClip;
        }

        return audioClip;
    }

    public synchronized void loadJar(URL baseURL) throws IOException {
        if(object == null) {
            Vector jarResourceProxys = new Vector();
            JarInputStream jar = null;
            byte[] rbuf = new byte[4096];

            try {
                jar = new JarInputStream(getInputStream());

                JarEntry jarEntry = jar.getNextJarEntry();
                while(jarEntry != null) {
                    String extension = null;
                    String fileName = jarEntry.getName();
                    int extensionIndex = fileName.lastIndexOf('.');

                    if(extensionIndex != -1) {
                        extension = jarEntry.getName().substring(extensionIndex);
                        fileName = fileName.substring(0, extensionIndex);
                    }
                    //      System.out.println("AEC entry:" + fileName + " " + extension);
                    fileName = fileName.replace('.', '/');

                    if(extension != null)
                        fileName += extension;


                    // Workaround: available funktioniert nicht!!!
                    ByteArrayOutputStream baos = new ByteArrayOutputStream();

                    int nread;
                    while((nread = jar.read(rbuf)) > 0)
                        baos.write(rbuf, 0, nread);

                    byte bytes[] = baos.toByteArray(); //new byte[jar.available()];
                    //      jar.read(bytes);

                    ProtectionDomain tmpProtectionDomain = null;

                    if(protectionDomain != null) {
                        PermissionCollection permissionCollection = protectionDomain.getPermissions();
                        CodeSource codeSource = new CodeSource(baseURL, jarEntry.getIdentities());
                        tmpProtectionDomain = new ProtectionDomain(codeSource, permissionCollection);
                    }

                      jarResourceProxys.addElement(ResourceProxy.create(new URL(baseURL, fileName), bytes, tmpProtectionDomain));
                      jarResourceProxys.addElement(ResourceProxy.create(new URL(baseURL + "/" +  fileName), bytes, tmpProtectionDomain));

                    jarEntry = jar.getNextJarEntry();
                }
            }
            finally {
                if(jar != null)
                    jar.close();
            }

            object = jarResourceProxys;
        }
    }

    private void guessType() {
        /* Parse entries which have special names, like the
           manifest. We invent "mime types" local to this class */
//          if (type == null) {
//              type = guessManifestType(name);
//          }

        if (type == null) {
            /*  REMIND Get this from MANIFEST.MF. This should
            be changed as soon as manifest support is
            fixed.  Because jvs must support mark, and it
            isn't always obvious since it's a
            FilterInputStream and whatever it used to
            build it is what would give it mark. */
            try {
                InputStream tmpStream = new ByteArrayInputStream(getBytes());
                type = URLConnection.guessContentTypeFromStream(tmpStream);
            }
            catch(IOException iOException) {
                if(DEBUG) System.err.println("#### Resource.guessType - exception:" + iOException);
            }
        }

        if (type == null) {
            // REMIND - this canbe cleaned up if we don't mind
            // exporting access to MANIFEST data. - epll
            type = "input-stream/input-stream";
        }

        if(DEBUG) System.err.println("#### Resource.guessType - guessed:" + type);
    }


    public Object getContent() throws IOException {
        Object content = null;

        guessType();

        if (type.startsWith("image")) {
            content = getImageProducer();
        }
        else if (type.startsWith("audio")) {
            content = getAudioClip();
        }
        else if (type.startsWith("application/java-vm") || type.startsWith("application/x-java-vm")) {

        }
        else if (type.equals("manifest/manifest") || type.equals("manifest/signature-bin")) {
            /* We know what it is. It is handled by JarVerifierStream */

        }
        else {
            content = getInputStream();
        }

        if(DEBUG) System.err.println("#### Resource.getContent:" + content);

        return content;
    }
}


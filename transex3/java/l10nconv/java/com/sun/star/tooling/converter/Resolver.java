/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Resolver.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:10:15 $
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
/*
 * Created on 27.01.2005
 *
 */
package com.sun.star.tooling.converter;

import java.io.IOException;
import java.io.InputStream;

import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * Resolve external entities by provide the
 * XLIFF DTD stored in the jar file
 */
public class Resolver implements EntityResolver {

    /**
     * Resolver constructor.
     */
    public Resolver() {
    }

    /**
     * Allow the application to resolve external entities.
     *
     * The Parser will call this method before opening any external entity
     * except the top-level document entity (including the external DTD subset,
     * external entities referenced within the DTD, and external entities
     * referenced within the document element): the application may request that
     * the parser resolve the entity itself, that it use an alternative URI, or
     * that it use an entirely different input source.
     */

    /* (non-Javadoc)
     * @see org.xml.sax.EntityResolver#resolveEntity(java.lang.String, java.lang.String)
     */

    public InputSource resolveEntity(String publicId, String systemId)
            throws SAXException, IOException {

        if ((publicId != null) && (publicId.equals("-//XLIFF//DTD XLIFF//EN"))) {
            systemId = "dtd:///xliff.dtd";
        }
        if (systemId != null) {
            if (systemId.startsWith("dtd://")) {
                String dtd = "com/sun/star/tooling/converter/dtd"
                        + systemId.substring(6);
                ClassLoader cl = this.getClass().getClassLoader();
                InputStream in = cl.getResourceAsStream(dtd);
                InputSource ins = new InputSource(in);
                ins.setSystemId(systemId);
                return ins;
            } /*
               * else if ( systemId.startsWith("jar:") ) { try { URL url=new
               * URL(systemId); JarURLConnection jarConn =
               * (JarURLConnection)url.openConnection(); InputSource ins=new
               * InputSource(jarConn.getInputStream());
               * ins.setSystemId(systemId); return ins; }
               * catch(MalformedURLException me){ throw new SAXException(me); //
               * Incorrect URL format used } }
               */
        }
        return null;
    }

}
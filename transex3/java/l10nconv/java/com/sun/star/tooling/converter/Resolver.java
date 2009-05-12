/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Resolver.java,v $
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
               * catch(MalformedURLException me){ throw new SAXException(me);
               * Incorrect URL format used } }
               */
        }
        return null;
    }

}
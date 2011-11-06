/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
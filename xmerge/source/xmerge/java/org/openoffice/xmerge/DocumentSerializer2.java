/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.xmerge;

import java.io.IOException;

/**
 *  <p>A <code>DocumentSerializer2</code> represents a converter that
 *  converts a &quot;Office&quot; <code>Document</code> to a
 *  &quot;Device&quot; <code>Document</code> format.</p>
 *
 *  <p>The <code>PluginFactory</code> {@link
 *  org.openoffice.xmerge.DocumentSerializerFactory#createDocumentSerializer
 *  createDocumentSerializer} method creates a <code>DocumentSerializer</code>,
 *  which may or may not implement <code>DocumentSerializer2</code>.   
 *  When it is constructed, a
 *  &quot;Office&quot; <code>Document</code> object is passed in to
 *  be used as input.</p>
 *
 *  @author  Henrik Just
 *  @see     org.openoffice.xmerge.PluginFactory
 *  @see     org.openoffice.xmerge.DocumentSerializerFactory
 */
public interface DocumentSerializer2 extends DocumentSerializer {

    /**
     *  <p>Convert the data passed into the <code>DocumentSerializer2</code>
     *  constructor into the &quot;Device&quot; <code>Document</code>
     *  format. The URL's passed may be used to resolve links and to name
     *  the output device document(s).</p>
     *
     *  <p>This method may or may not be thread-safe.  It is expected
     *  that the user code does not call this method in more than one
     *  thread.  And for most cases, this method is only done once.</p>
     *
     *  @return  <code>ConvertData</code> object to pass back the
     *           converted data.
     *
     *  @param   officeURL         URL of the office document (may be null if unknown)
     *  @param   deviceURL         URL of the device document (may be null if unknown)
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData serialize(String officeURL, String deviceURL) throws
        ConvertException, IOException;
}


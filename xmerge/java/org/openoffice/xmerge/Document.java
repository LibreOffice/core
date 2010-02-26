/************************************************************************
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

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

/**
 *  <p>A <code>Document</code> represents any <code>Document</code>
 *  to be converted and the resulting <code>Document</code> from any
 *  conversion.</p>
 *
 *  <p>It is created by the <code>PluginFactory</code> object's {@link
 *  org.openoffice.xmerge.PluginFactory#createOfficeDocument
 *  createOfficeDocument} method or the {@link
 *  org.openoffice.xmerge.PluginFactory#createDeviceDocument
 *  createDeviceDocument} method.</p>
 *
 *  @author  Herbie Ong
 *  @see     org.openoffice.xmerge.PluginFactory
 */
public interface Document {

    /**
     *  <p>Writes out the <code>Document</code> content to the specified
     *  <code>OutputStream</code>.</p>
     *
     *  <p>This method may not be thread-safe.
     *  Implementations may or may not synchronize this
     *  method.  User code (i.e. caller) must make sure that
     *  calls to this method are thread-safe.</p>
     *
     *  @param  os  <code>OutputStream</code> to write out the
     *              <code>Document</code> content.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException;


    /**
     *  <p>Reads the content from the <code>InputStream</code> into
     *  the <code>Document</code>.</p>
     *
     *  <p>This method may not be thread-safe.
     *  Implementations may or may not synchronize this
     *  method.  User code (i.e. caller) must make sure that
     *  calls to this method are thread-safe.</p>
     *
     *  @param  is  <code>InputStream</code> to read in the
     *              <code>Document</code> content.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
     public void read(InputStream is) throws IOException;


    /**
     *  Returns the <code>Document</code> name with no file extension.
     *
     *  @return  The <code>Document</code> name with no file extension.
     */
    public String getName();


    /**
     *  Returns the <code>Document</code> name with file extension.
     *
     *  @return  The <code>Document</code> name with file extension.
     */
    public String getFileName();
}


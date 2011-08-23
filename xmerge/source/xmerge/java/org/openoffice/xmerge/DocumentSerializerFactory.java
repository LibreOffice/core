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

/**
 *  <p>A <code>DocumentSerializer</code> object is used to convert
 *  from the &quot;Office&quot; <code>Document</code> format to the
 *  &quot;Device&quot; <code>Document</code> format.</p>
 *
 *  <p>All plug-in implementations of the <code>PluginFactory</code>
 *  interface that also support serialization must also
 *  implement this interface.</p>
 *
 *  @see  PluginFactory
 *  @see  DocumentSerializer
 */
public interface DocumentSerializerFactory {

    /**
     *  <p>The <code>DocumentSerializer</code> is used to convert 
     *  from the &quot;Office&quot; <code>Document</code> format 
     *  to the &quot;Device&quot; <code>Document</code> format.</p>
     *
     *  The <code>ConvertData</code> object is passed along to the
     *  created <code>DocumentSerializer</code> via its constructor.
     *  The <code>ConvertData</code> is read and converted when the
     *  the <code>DocumentSerializer</code> object's
     *  <code>serialize</code> method is called.
     *
     *  @param  doc  <code>Document</code> object that the created
     *               <code>DocumentSerializer</code> object uses
     *               as input.
     *
     *  @return  A <code>DocumentSerializer</code> object.
     */
    public DocumentSerializer createDocumentSerializer(Document doc);
}


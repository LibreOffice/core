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
 *  <p>All plug-in implementations of the <code>PluginFactory</code>
 *  interface that also support merging must also implement
 *  this interface.</p>
 *
 *  <p>Merge is useful when an <code>OfficeDocument</code>
 *  is converted to a &quot;Device&quot; <code>Document</code> format,
 *  and the &quot;Device&quot; <code>Document</code> version is modified.
 *  Those changes can be merged back into the original
 *  <code>OfficeDocument</code> with the merger.  The merger is capable
 *  of doing this even if the &quot;Device&quot; format is lossy in
 *  comparison to the <code>OfficeDocument</code> format.</p>
 * 
 *  @see  PluginFactory
 *  @see  DocumentMerger
 *  @see  ConverterCapabilities
 *
 */
public interface DocumentMergerFactory {

    /**
     *  <p>Create a <code>DocumentMerger</code> object given a
     *  <code>Document</code> object.</p>
     *
     *  @param  doc  <code>Document</code> object that the created
     *               <code>DocumentMerger</code> object uses as a base
     *               <code>Document</code> for merging changes into.
     *
     *  @return  A <code>DocumentMerger</code> object or null if none
     *           exists.
     */
    public DocumentMerger createDocumentMerger(Document doc);
}


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
 *  <p>A <code>DocumentMerger</code> can merge changes from a modified
 *  &quot;Device&quot; <code>Document</code> to the assigned original
 *  &quot;Office&quot; <code>Document</code>.</p>
 *
 *  <p>Merge is useful when an <code>OfficeDocument</code>
 *  is converted to a &quot;Device&quot; <code>Document</code> format,
 *  and the &quot;Device&quot; <code>Document</code> version is modified.
 *  Those changes can be merged back into the original
 *  <code>OfficeDocument</code> with the merger.  The merger is capable
 *  of doing this even if the &quot;Device&quot; format is lossy in
 *  comparison to the <code>OfficeDocument</code> format.</p>
 *
 *  <p>The <code>ConverterCapabilities</code> object is what the 
 *  DocumentMerger utilizes to know how the &quot;Office&quot;
 *  <code>Document</code> tags are supported in the &quot;Device&quot;
 *  format.</p>
 * 
 *  <p>The <code>DocumentMerger</code> object is created by a
 *  the <code>DocumentMergerFactory</code> {@link
 *  org.openoffice.xmerge.DocumentMergerFactory#createDocumentMerger
 *  createDocumenMerger} method.  When it is constructed, the
 *  &quot;Original Office&quot; <code>Document</code> object is
 *  passed in to be used as input.</p>
 *
 *  @author  Herbie Ong
 *  @see     org.openoffice.xmerge.PluginFactory
 *  @see     org.openoffice.xmerge.DocumentMergerFactory
 *  @see     org.openoffice.xmerge.ConverterCapabilities
 */
public interface DocumentMerger {

    /**
     *  <p>This method will find the changes that had happened
     *  in the <code>modifiedDoc</code> <code>Document</code>
     *  object given the designated original <code>Document</code>.</p>
     *
     *  <p>Note that this  process may need the knowledge of the
     *  conversion process since some conversion process are lossy.
     *  Items/Data that are lost during the conversion process are not
     *  classified as changes.  The main target of this method
     *  is to apply the changes done in <code>modifiedDoc</code>
     *  into the assigned original <code>Document</code> object, thus
     *  it also will try to preserve items that were originally in
     *  the original <code>Document</code>, but never got transferred
     *  during the
     *  {@link org.openoffice.xmerge.DocumentSerializer#serialize
     *  serialize} process/method call.  After this method call, the
     *  original <code>Document</code> object will contain the changes
     *  applied.</p>
     *
     *  <p>This method may or may not be thread-safe.
     *  Also, it is expected that the user uses only one instance
     *  of a <code>DocumentMerger</code> object per merge process.
     *  Create another <code>DocumentMerger</code> object for another
     *  merge process.</p>
     *
     *  @param  modifiedDoc  device <code>Document</code> object.
     *
     *  @throws  MergeException  If any merge error occurs.
     */
    public void merge(Document modifiedDoc) throws MergeException;
}


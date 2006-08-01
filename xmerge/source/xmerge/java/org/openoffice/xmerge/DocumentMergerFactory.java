/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentMergerFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:19:43 $
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


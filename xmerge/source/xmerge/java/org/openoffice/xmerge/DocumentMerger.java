/************************************************************************
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


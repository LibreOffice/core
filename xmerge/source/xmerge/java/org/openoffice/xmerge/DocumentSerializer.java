/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentSerializer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:19:55 $
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

import java.io.IOException;

/**
 *  <p>A <code>DocumentSerializer</code> represents a converter that
 *  converts a &quot;Office&quot; <code>Document</code> to a
 *  &quot;Device&quot; <code>Document</code> format.</p>
 *
 *  <p>The <code>DocumentSerializer</code> object is created by a
 *  the <code>PluginFactory</code> {@link
 *  org.openoffice.xmerge.DocumentSerializerFactory#createDocumentSerializer
 *  createDocumentSerializer} method.  When it is constructed, a
 *  &quot;Office&quot; <code>Document</code> object is passed in to
 *  be used as input.</p>
 *
 *  @author  Herbie Ong
 *  @see     org.openoffice.xmerge.PluginFactory
 *  @see     org.openoffice.xmerge.DocumentSerializerFactory
 */
public interface DocumentSerializer {

    /**
     *  <p>Convert the data passed into the <code>DocumentSerializer</code>
     *  constructor into the &quot;Device&quot; <code>Document</code>
     *  format.</p>
     *
     *  <p>This method may or may not be thread-safe.  It is expected
     *  that the user code does not call this method in more than one
     *  thread.  And for most cases, this method is only done once.</p>
     *
     *  @return  <code>ConvertData</code> object to pass back the
     *           converted data.
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData serialize() throws ConvertException, IOException;
}


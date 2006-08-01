/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentDeserializer2.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:19:03 $
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
 *  <p>A <code>DocumentDeserializer</code> represents a converter that
 *  converts &quot;Device&quot; <code>Document</code> objects into the
 *  &quot;Office&quot; <code>Document</code> format.</p>
 *
 *  <p>The <code>PluginFactory</code> {@link
 *  org.openoffice.xmerge.DocumentDeserializerFactory#createDocumentDeserializer
 *  createDocumentDeserializer} method creates a <code>DocumentDeserializer</code>,
 *  which may or may not implement <code>DocumentDeserializer2</code>.
 *  When it is constructed, a
 *  <code>ConvertData</code> object is passed in to be used as input.</p>
 *
 *  @author  Henrik Just
 *  @see     org.openoffice.xmerge.PluginFactory
 *  @see     org.openoffice.xmerge.DocumentDeserializerFactory
 */
public interface DocumentDeserializer2 extends DocumentSerializer {

    /**
     *  <p>Convert the data passed into the <code>DocumentDeserializer2</code>
     *  constructor into the &quot;Office&quot; <code>Document</code>
     *  format. The URL's passed may be used to resolve links and to choose the
     *  name of the output office document.</p>
     *
     *  <p>This method may or may not be thread-safe.  It is expected
     *  that the user code does not call this method in more than one
     *  thread.  And for most cases, this method is only done once.</p>
     *
     *  @return  The resulting <code>Document</code> object from conversion.
     *
     *  @param   deviceURL         URL of the device document (may be null if unknown)
     *  @param   officeURL         URL of the office document (may be null if unknown)
     *
     *  @throws  ConvertException  If any Convert error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public Document deserialize(String deviceURL, String officeURL) throws
        ConvertException, IOException;
}


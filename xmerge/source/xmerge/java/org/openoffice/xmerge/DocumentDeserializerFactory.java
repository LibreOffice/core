/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentDeserializerFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:19:17 $
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
 *  <p>A <code>DocumentDeserializer</code> object is used to convert
 *  from the &quot;Device&quot; <code>Documetn</code> format to the
 *  &quot;Office&quot; <code>Document</code> format.</p>
 *
 *  <p>All plug-in implementations of the <code>PluginFactory</code>
 *  interface that also support deserialization must also
 *  implement this interface.</p>
 *
 *  @see  PluginFactory
 *  @see  DocumentDeserializer
 */
public interface DocumentDeserializerFactory {

    /**
     *  The <code>DocumentDeserializer</code> is used to convert
     *  from the &quot;Device&quot; <code>Document</code> format to
     *  the &quot;Office&quot; <code>Document</code> format.</p>
     *
     *  The <code>ConvertData</code> object is passed along to the
     *  created <code>DocumentDeserializer</code> via its constructor.
     *  The <code>ConvertData</code> is read and converted when the
     *  the <code>DocumentDeserializer</code> object's
     *  <code>deserialize</code> method is called.
     *  </p>
     *
     *  @param  cd  <code>ConvertData</code> object that the created
     *              <code>DocumentDeserializer</code> object uses as
     *              input.
     *
     *  @return  A <code>DocumentDeserializer</code> object.
     */

    public DocumentDeserializer createDocumentDeserializer(ConvertData cd);
}


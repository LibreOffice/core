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

import org.openoffice.xmerge.util.registry.ConverterInfo;
import java.io.InputStream;
import java.io.IOException;

/**
 *  <p>A <code>PluginFactory</code> encapsulates the
 *  conversions from one <code>Document</code> format to another.
 *  It provides conversions in both directions.  Refer to the
 *  <a href="package-summary.html#package_description">
 *  package description</a> for its usage.</p>
 *
 *  <p>Conversion from the &quot;Office&quot; <code>Document</code>
 *  format to a &quot;Device&quot; <code>Document</code> format may
 *  be lossy, i.e. some information may be lost.  If a plug-in
 *  implements the <code>DocumentMergerFactory</code> interface,
 *  then there is the possibility for merging the changes done on the
 *  &quot;Device&quot; <code>Document</code> back to the original
 *  &quot;Office&quot; <code>Document</code> via the
 *  <code>DocumentMerger</code> interface.</p>
 *
 *  <p>Plug-ins that convert from the &quot;Device&quot;
 *  <code>Document</code> format to the &quot;Office&quot;
 *  <code>Document</code> format must implement the
 *  <code>DocumentDeserializerFactory</code> interface.  Plug-ins
 *  that convert from the &quot;Office&quot; <code>Document</code>
 *  format to the &quot;Device&quot; format must implement the
 *  <code>DocumentSerializerFactory</code> interface.
 *
 *  <p>All plug-ins should have an associated Plugin Configuration XML
 *  File which describes the capabilities of the plug-in.  If the
 *  plug-in is bundled in a jarfile, then this XML file is also bundled
 *  with the jarfile.  The data in the XML file is managed by the
 *  <code>ConverterInfo</code> object.  The <code>ConverterInfoMgr</code>
 *  manages a registry of all <code>ConverterInfo</code> objects.  For
 *  more information about this XML file, refer to
 *  <a href="converter/xml/sxc/package-summary.html">
 *  org.openoffice.xmerge.util.registry</a>.</p>
 *
 *  @author  Herbie Ong
 *  @see     Document
 *  @see     DocumentSerializer
 *  @see     DocumentSerializerFactory
 *  @see     DocumentDeserializer
 *  @see     DocumentDeserializerFactory
 *  @see     DocumentMerger
 *  @see     DocumentMergerFactory
 *  @see     ConverterInfo
 *  @see     org.openoffice.xmerge.util.registry.ConverterInfoMgr
 */

public abstract class PluginFactory {

   /**
    *  Cached <code>ConvertInfo</code> object.
    */
    private ConverterInfo ciCache;


   /**
    *  Constructor that caches the <code>ConvertInfo</code> that
    *  corresponds to the registry information for this plug-in.
    *
    *  @param  ci  <code>ConvertInfo</code> object.
    */
    public PluginFactory(ConverterInfo ci) {
        ciCache=ci;
    }


   /**
    *  Returns the <code>ConvertInfo</code> that corresponds to this
    *  plug-in.
    *
    *  @return  The <code>ConvertInfo</code> that corresponds to this
    *           plug-in.
    */
    public ConverterInfo getConverterInfo () {
        return ciCache;
    }


    /**
     *  <p>Create a <code>Document</code> object that corresponds to
     *  the Office data passed in via the <code>InputStream</code>
     *  object.  This abstract method must be implemented for each
     *  plug-in.</p>
     *
     *  <p>This method will read from the given <code>InputStream</code>
     *  object.  The returned <code>Document</code> object will contain
     *  the necessary data for the other objects created by the
     *  <code>PluginFactory</code> to process, like a
     *  <code>DocumentSerializer</code> object and a
     *  <code>DocumentMerger</code> object.</p>
     *
     *  @param  name  The <code>Document</code> name.
     *  @param  is    <code>InputStream</code> object corresponding
     *                to the <code>Document</code>.
     *
     *  @return  A <code>Document</code> object representing the
     *           particular <code>Document</code> format for the
     *           <code>PluginFactory</code>.
     *
     *  @throws   IOException   If any I/O error occurs.
     */
    public abstract Document createOfficeDocument(String name, InputStream is)
        throws IOException;


    /**
     *  <p>Create a <code>Document</code> object that corresponds to
     *  the Office data passed in via the <code>InputStream</code>
     *  object.  This abstract method must be implemented for each
     *  plug-in.</p>
     *
     *  <p>This method will read from the given <code>InputStream</code>
     *  object.  The returned <code>Document</code> object will contain
     *  the necessary data for the other objects created by the
     *  <code>PluginFactory</code> to process, like a
     *  <code>DocumentSerializer</code> object and a
     *  <code>DocumentMerger</code> object.</p>
     *
     *  @param  name  The <code>Document</code> name.
     *  @param  is    <code>InputStream</code> object corresponding
     *                to the <code>Document</code>.
     *  @param  isZip <code>boolean</code> to show that the created office
     *                document is to be zipped.
     *
     *  @return  A <code>Document</code> object representing the
     *           particular <code>Document</code> format for the
     *           <code>PluginFactory</code>.
     *
     *  @throws   IOException   If any I/O error occurs.
     */
    public abstract Document createOfficeDocument(String name, InputStream is,boolean isZip)
        throws IOException;


    /**
     *  <p>Create a <code>Document</code> object that corresponds to
     *  the device data passed in via the <code>InputStream</code>
     *  object.  This abstract method must be implemented for each
     *  plug-in.</p>
     *
     *  <p>This method will read from the given <code>InputStream</code>
     *  object.  The returned <code>Document</code> object will contain
     *  the necessary data for the other objects created by the
     *  <code>PluginFactory</code> to process, like a
     *  <code>DocumentSerializer</code> object and a
     *  <code>DocumentMerger</code> object.</p>
     *
     *  @param  name  The <code>Document</code> name.
     *  @param  is    <code>InputStream</code> object corresponding
     *                to the <code>Document</code>.
     *
     *  @return  A <code>Document</code> object representing the
     *           particular <code>Document</code> format for the
     *           <code>PluginFactory</code>.
     *
     *  @throws   IOException   If any I/O error occurs.
     */
    public abstract Document createDeviceDocument(String name, InputStream is)
        throws IOException;
}


/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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


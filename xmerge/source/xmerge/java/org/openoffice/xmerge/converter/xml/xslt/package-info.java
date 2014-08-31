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

/**
 * Provides the tools for doing the conversion of StarWriter XML to and from
 * supported formats, through the use of an XSLT transformation.
 *
 * <p>It follows the {@code org.openoffice.xmerge} framework for the conversion
 * process.</p>
 *
 * <p>This converter does not currently support merge.</p>
 *
 * <h2>XSLT Transformation</h2>
 *
 * <p>The converter makes use of one or more XSLT style sheets, which are used
 * in the DocumentSerializer and DocumentDeserializer, to perform the actual
 * translations. The location of these stylesheets is extracted from the
 * {@link org.openoffice.xmerge.util.registry.ConverterInfo ConverterInfo} data
 * structure, and are specified using the optional converter-XSLT-serialize and
 * converter-XSLT-deserialize tags in a plug-ins converter.xml file. Please
 * refer to the SDK document for more information about how to implement a
 * Plug-in Configuration XML File for a specific plug-in.
 * A sample OpenOffice to HTML stylesheet and HTML to OpenOffice stylesheet, has
 * been provided as a sample implementation. The converter also makes use of an
 * XsltPlugin.properties file, which may be edited by the user to provide
 * MIME-TYPE to file extension mappings. This file is used by the
 * {@link org.openoffice.xmerge.converter.xml.xslt.PluginFactoryImpl
 * getDeviceFileExtension} method.</p>
 *
 * <h2>TODO list</h2>
 *
 * <ol>
 * <li>Expand XSLT style sheets to support more office/HTML capabilities</li>
 * <li>Add support for certain character codes, such as {@literal &} which
 *     currently causes the transformer to break.</li>
 * <li>Change the DocumentDeserializer transformer, so that the DOMResult is
 *     serialized using the xalan serializer and create an SxwDocument from the
 *     result</li>
 * </ol>
 *
 * @see org.openoffice.xmerge.util.registry.ConverterInfo
 */
package org.openoffice.xmerge.converter.xml.xslt;

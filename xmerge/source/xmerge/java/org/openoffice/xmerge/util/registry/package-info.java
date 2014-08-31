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
 * Provides an interface for plug-in registration.
 *
 * <p>Each plug-in must have a corresponding Plug-in Configuration XML File
 * which is named converter.xml. If the plug-in is stored in a jarfile, this
 * converter.xml file is typically stored in the following location in the
 * jarfile:</p>
 *
 * <blockquote>META-INF/converter.xml</blockquote>
 *
 * <p>The Plug-in Configuration XML File must validate against the converter.dtd
 * file provided with this package.  Since a jarfile can contain multiple
 * plug-ins, this DTD supports specifying multiple plug-ins per jarfile.  Please
 * refer to the SDK document for more information about how to implement a
 * Plug-in Configuration XML File for a specific plug-in.</p>
 *
 * <p>All information in the Plug-in Configuration XML File is bundled into one
 * or more {@code ConverterInfo} object.  The {@code ConverterInfoReader} object
 * is used to build a {@code Vector} of {@code ConverterInfo} objects from a
 * jarfile.</p>
 *
 * <p>The {@code ConverterInfoMgr} manages the registry of {@code ConverterInfo}.
 * It is a singleton class, so that only one registry manager will ever exist.
 * It is the client program's responsibility to register {@code ConverterInfo}
 * objects that correspond to the plug-ins that are to be used.</p>
 *
 * <h2>TODO/IDEAS list</h2>
 * <ol>
 * <li>The {@code ConverterInfo} object could contain
 *     {@code org.w3c.dom.Document} fragments that are accessed in a generic
 *     fashion rather than get/set methods for each item in the DTD. This would
 *     provide a more flexible approach, especially for adding custom tags to a
 *     specific Plug-in Configuration XML file (tags that are only used by its
 *     associated plug-in).</li>
 * <li>{@code ConverterInfo} should allow the merge/serialize/deserialize logic
 *     to be included in separate plug-ins, if desired.</li>
 * <li>{@code ConverterInfoMgr} could use the Java Activation Framework (JAF)
 *     to manage registration.</li>
 * </ol>
 */
package org.openoffice.xmerge.util.registry;

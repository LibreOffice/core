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
 * Provides interfaces for converting between two {@code Document} formats, and
 * supports a {@literal "merge"} interface for merging back changes from a
 * {@literal "lossy"} format back into a rich format.
 *
 * <p>The {@link org.openoffice.xmerge.Convert Convert} object encapsulates the
 * conversion of one format to/from another format.  The user requests a
 * {@code Convert} object via the {@code ConverterFactory}.</p>
 *
 * <p> The {@code Convert} class encapsulates a specific plug-in.
 * A plug-in can support deserialization (convert from {@literal "Device"} to
 * {@literal "Office"}) and/or serialization (convert from {@literal "Office"}
 * to {@literal "Device"}).  If a plug-in supports both deserialization and
 * serialization, then it can also support {@literal "merge"}.</p>
 *
 * <p>To support conversions where a single input {@code Document} can create
 * multiple output {@code Document} objects, data is passed in and out of the
 * conversion functions via a {@code ConvertData"} object.
 * This {@code ConvertData} can contain one or more {@code Document} objects.
 * It is assumed that the client will know when to pass multiple files into a
 * specific plug-in, and that the plug-in will know how to handle the multiple
 * files.</p>
 *
 * <p>Merging is useful when converting from a rich {@code Document} format to
 * a more lossy format.  Then the user may modify the {@code Document} in the
 * lossy format, and {@literal "merge"} those changes back into the original
 * {@literal "rich"} {@code Document}.
 * Each merge implementation provides a {@code ConverterCapabilities}
 * implementation so that the merge logic knows what changes from the
 * {@literal "lossy"} format to merge into the original {@literal "rich"}
 * {@code Document}.</p>
 *
 * <p>Each plug-in must be registered via the singleton {@code ConverterInfoMgr}
 * object via its {@link
 * org.openoffice.xmerge.util.registry.ConverterInfoMgr#addPlugIn addPlugIn}
 * method.</p>
 *
 * <h2>Providing implementations</h2>
 *
 * <p>The plug-in implementation must include the {@code getDeviceDocument} and
 * {@code getOfficeDocument} methods.  These functions need to return the
 * appropriate type of {@code Document} for the plug-in.  It may be necessary to
 * create a new implementation of the {@code Document} interface if one does not
 * exist that meets the needs of the plug-in.</p>
 *
 * <p>Currently, base implementations for working with StarWriter XML
 * {@code Document} objects are available via the
 * <a href="converter/xml/sxc/package-summary.html#package_description">
 * org.openoffice.xmerge.xml.sxw</a> package, and StarCalc XML {@code Document}
 * objects via the
 * <a href="converter/xml/sxw/package-summary.html#package_description">
 * org.openoffice.xmerge.xml.sxc</a> package.</p>
 *
 * <h2>TODO/IDEAS list</h2>
 *
 * <ol>
 * <li>An idea is to combine the {@code ConvertData} and the {@code Convert}
 *   classes, so that a {@code ConvertData} knows what it can convert into and
 *   whether or not it can merge.
 *   Then a user would call convert/merge methods on the {@code ConvertData}
 *   class, which returns a {@code ConvertData} object that likewise knows what
 *   it can convert/merge into.</li>
 * <li>{@code DocumentSerialize} constructors and the
 *   {@code DocumentDeserializer.deserializer} method could pass in a
 *   {@code ConvertData} object rather than assuming a single {@code Document}
 *   will represent a {@literal "rich"} {@code Document}.</li>
 * <li>May need to add a {@code PluginFactory.setProperties} method for adding
 *   properties specific to each converter.</li>
 * </ol>
 */
package org.openoffice.xmerge;
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
 * Provides classes for converting Palm database data to/from a
 * {@code PalmDocument} object, which can be used by the framework.
 *
 * <p>This package provides classes that handle the writing of data to an
 * {@code OutputStream} object for the {@link
 * org.openoffice.xmerge.DocumentSerializer DocumentSerializer} interface for;
 * as well as the reading of data from an {@code InputStream} object for the
 * framework's {@link org.openoffice.xmerge.DocumentDeserializer
 * DocumentDeserializer} interface. Both these framework interfaces are simply
 * converters from server-side documents to device specific documents and
 * vice-versa.
 * Since all Palm databases have a general record oriented format, a Palm
 * database converter specific I/O stream format is specified for the Palm sync
 * client application to handle the byte stream in a generic way.
 * This also means that Palm database converters should read and/or write using
 * this I/O stream format as specified in the next section.</p>
 *
 * <a name="streamformat"></a>
 *
 * <h2>Palm database converter specific I/O stream format</h2>
 *
 * <p>Note that the format of the byte stream is not exactly that of a PDB file
 * encoding. It does not need to contain the PDB header information nor record
 * indices section.  Instead, it contains the following ...</p>
 * <pre>
 *    set header
 *       4 bytes - creator id
 *       4 bytes - type id
 *       2 bytes - PDB header version
 *       2 bytes - PDB header attribute
 *       unsigned 2 bytes - number of PDB data to follow
 *
 *    for each PDB,
 *       32 bytes - name of PDB i
 *       unsigned 2 bytes - number of records in PDB i
 *
 *       for each record contained in PDB i,
 *          1 byte - record attributes
 *          unsigned 2 bytes - size of record j in PDB i
 *          x bytes - data
 * </pre>
 *
 * <p>Note that each PDB section is appended by another if there is more than
 * one.</p>
 *
 * <p>Since the {@code PalmDocument} class takes care of the writing and reading
 * of this format through its {@code write} and {@code read} methods,
 * respectively, this format shall also be referred to as the <b>PalmDocument
 * stream format</b>.</p>
 *
 * <h2>Usage of the classes for the specified I/O stream</h2>
 *
 * <p>When converting from a server document to device document(s), the
 * framework requires writing the device document(s) to an {@code OutputStream}
 * object via the {@code DocumentSerializer} interface. Note that a single
 * server document may be converted into multiple PDB's on the Palm device.
 * Each worksheet in the document is converted into a {@code PalmDocument}.
 * Thus, if there is more than one worksheet in the document, more than one
 * {@code PalmDocument} will be produced by the {@code DocumentSerializer}.</p>
 *
 * <p>A {@code DocumentSerializer} creates a {@code ConvertData} object, which
 * contains all of the {@code PalmDocuments}.  The {@link
 * org.openoffice.xmerge.converter.palm.PalmDocument#write write} method to
 * write to the given {@code OutputStream}.
 * The {@code PalmDocument} object will take care of writing the data in the
 * <a href=#streamformat>specified format</a>.</p>
 *
 * <p>A {@code DocumentDeserializer} can use the {@code PalmDocument} object's
 * {@link org.openoffice.xmerge.converter.palm.PalmDocument#read read} method
 * to fill in all the {@code PalmDocument} object's data.</p>
 *
 * <h2>PDB file encoding/decoding</h2>
 *
 * <p>The {@code PalmDocument} object's read and write functions are provided by
 * the {@code PdbDecoder} and {@code PdbEncoder} objects.
 * The {@code PdbEncoder} class provides the functionality of encoding a
 * {@code PalmDB} object into an {@code InputStream}, while the
 * {@code PdbDecoder} class provides the functionality of decoding a PDB file
 * into an {@code OutputStream}.</p>
 *
 * <p>Refer to the class description of each for usage.</p>
 *
 * <h2>Important Note</h2>
 *
 * <p>Methods in these classes are not thread safe for performance reasons.
 * Users of these classes will have to make sure that the usage of these classes
 * are done in a proper manner.  Possibly more on this later.</p>
 *
 * <h2>TODO list</h2>
 *
 * <ol>
 * <li>Merge the PalmDB, PdbDecoder and PdbEncoder classes into the PalmDocument
 *     class.</li>
 * <li>After reading more on the palm file format spec, I realized that there
 *     are certain optional fields that may need to be addressed still, like the
 *     appInfo block and sortInfo block.</li>
 * <li>The current PdbDecoder only returns a PalmDB object.  There are other
 *     information that we may want to expose from the PDB decoding process.</li>
 * <li>Investigate on different language encoding on the Palm and how that
 *     affects the PDB name.</li>
 * </ol>
 */
package org.openoffice.xmerge.converter.palm;

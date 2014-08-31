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
 * Provides classes for converting basic document types to/from a
 * {@code DOMDocument} object, which can be used by the framework.
 *
 * <p>This package provides classes that handle the writing of data to an
 * {@code OutputStream} object for the {@link
 * org.openoffice.xmerge.DocumentSerializer DocumentSerializer} interface for;
 * as well as the reading of data from an {@code InputStream} object for the
 * framework's {@link org.openoffice.xmerge.DocumentDeserializer
 * DocumentDeserializer} interface. Both these framework interfaces are simply
 * converters from server-side documents to device specific documents and
 * vice-versa.</p>
 *
 * <a name="streamformat"></a>
 *
 * <h2>Important Note</h2>
 *
 * <p>Methods in these classes are not thread safe for performance reasons.
 * Users of these classes will have to make sure that the usage of these classes
 * are done in a proper manner.  Possibly more on this later.</p>
 */
package org.openoffice.xmerge.converter.dom;

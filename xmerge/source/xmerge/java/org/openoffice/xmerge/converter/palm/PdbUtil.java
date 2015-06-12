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

package org.openoffice.xmerge.converter.palm;

/**
 * Contains common static methods and constants for use within the package.
 */
public final class PdbUtil {

    /** Difference in seconds from Jan 01, 1904 to Jan 01, 1970. */
    static final long TIME_DIFF = 2082844800;

    /** Encoding scheme used. */
    static final String ENCODING = "8859_1";

    /** Size of a PDB header in bytes. */
    static final int HEADER_SIZE = 78;

}
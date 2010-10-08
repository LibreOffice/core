/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


// NOTE:
// This class does not yet exist before 1.8.0.8. When we move our shipped
// version to 1.8.0.8 or higher, this file here can be removed from CVS.

package org.hsqldb.lib;

/** is a RuntimeException which indicates failure during basic IO
 *  operations in a FileAccess implementation.
 *
 * @author frank.schoenheit@sun.com
 *
 * @version 1.8.0.8
 * @since 1.8.0.8
 */
public class FileSystemRuntimeException extends java.lang.RuntimeException {

    public static final int fileAccessRemoveElementFailed = 1;
    public static final int fileAccessRenameElementFailed = 2;

    private final int errorCode;

    public FileSystemRuntimeException(int _errorCode) {
        super();
        errorCode = _errorCode;
    }

    public FileSystemRuntimeException(String _message, int _errorCode) {
        super(_message);
        errorCode = _errorCode;
    }

    public FileSystemRuntimeException(String _message, java.lang.Throwable _cause, int _errorCode) {
        super(_message, _cause);
        errorCode = _errorCode;
    }

    public FileSystemRuntimeException(java.lang.Throwable _cause, int _errorCode) {
        super(_cause);
        errorCode = _errorCode;
    }

    public final int getErrorCode() {
        return errorCode;
    }
}
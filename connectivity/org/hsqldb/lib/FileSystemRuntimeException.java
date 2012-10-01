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


// NOTE:
// This class does not yet exist before 1.8.0.8. When we move our shipped
// version to 1.8.0.8 or higher, this file here can be removed from CVS.

package org.hsqldb.lib;

/** is a RuntimeException which indicates failure during basic IO
 *  operations in a FileAccess implementation.
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

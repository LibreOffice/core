/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




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
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FileSystemRuntimeException.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 15:49:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
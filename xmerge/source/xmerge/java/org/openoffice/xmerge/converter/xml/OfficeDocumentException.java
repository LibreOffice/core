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

package org.openoffice.xmerge.converter.xml;

import java.io.IOException;

import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import org.openoffice.xmerge.util.Resources;

/**
 *  Used by OfficeDocument to encapsulate exceptions.  It will add
 *  more details to the message string if it is of type
 *  <code>SAXParseException</code>.
 *
 *  @author      Herbie Ong
 */

public final class OfficeDocumentException extends IOException {

    StringBuffer message = null;


   /**
    *  Constructor, capturing additional information from the
    *  <code>SAXException</code>.
    *
    *  @param  e  The <code>SAXException</code>.
    */
    public OfficeDocumentException(SAXException e) {
        super(e.toString());
        message = new StringBuffer();
        if (e instanceof SAXParseException) {
            String msgParseError =
                Resources.getInstance().getString("PARSE_ERROR");
            String msgLine =
                Resources.getInstance().getString("LINE");
            String msgColumn =
                Resources.getInstance().getString("COLUMN");
            String msgPublicId =
                Resources.getInstance().getString("PUBLIC_ID");
            String msgSystemId =
                Resources.getInstance().getString("SYSTEM_ID");
            SAXParseException spe = (SAXParseException) e;
            message.append(msgParseError);
            message.append(": ");
            message.append(msgLine);
            message.append(": ");
            message.append(spe.getLineNumber());
            message.append(", ");
            message.append(msgColumn);
            message.append(": ");
            message.append(spe.getColumnNumber());
            message.append(", ");
            message.append(msgSystemId);
            message.append(": ");
            message.append(spe.getSystemId());
            message.append(", ");
            message.append(msgPublicId);
            message.append(": ");
            message.append(spe.getPublicId());
            message.append("\n");
        }

        // if there exists an embedded exception
        Exception ex = e.getException();
        if (ex != null) {
            message.append(ex.getMessage());
        }
    }


   /**
    *  Constructor, creates exception with provided message.
    *
    *  @param  s  Message value for the exception.
    */
    public OfficeDocumentException(String s) {
        super(s);
    }


   /**
    *  Constructor, creates exception with the message
    *  corresponding to the message value of the provided
    *  exception.
    *
    *  @param  e  The Exception.
    */
    public OfficeDocumentException(Exception e) {
        super(e.getMessage());
    }


   /**
    *  Returns the message value for the <code>Exception</code>.
    *
    * @return  The message value for the <code>Exception</code>.
    */
    public String getMessage() {
        return message.toString() + super.getMessage();
    }
}


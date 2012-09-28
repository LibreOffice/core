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

package org.openoffice.xmerge.converter.xml;

import java.io.IOException;

import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import org.openoffice.xmerge.util.Resources;

/**
 *  Used by OfficeDocument to encapsulate exceptions.  It will add
 *  more details to the message string if it is of type
 *  <code>SAXParseException</code>.
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


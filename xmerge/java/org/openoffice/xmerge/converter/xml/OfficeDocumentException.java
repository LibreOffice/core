/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml;

import java.io.IOException;

import javax.xml.parsers.ParserConfigurationException;
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


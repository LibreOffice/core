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
package integration.forms;

/** a helper "enumeration class" for classifying a document type
*/
public class DocumentType extends com.sun.star.uno.Enum
{
    private DocumentType( int value )
    {
        super( value );
    }



    public static final DocumentType WRITER = new DocumentType(0);
    public static final DocumentType CALC = new DocumentType(1);
    public static final DocumentType DRAWING = new DocumentType(2);
    public static final DocumentType XMLFORM = new DocumentType(3);
    public static final DocumentType UNKNOWN = new DocumentType(-1);


}


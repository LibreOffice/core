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
package com.sun.star.wizards.common;

import com.sun.star.lang.XMultiServiceFactory;

public class InvalidQueryException extends java.lang.Throwable
{
    // TODO don't show messages in Exceptions
    public InvalidQueryException(XMultiServiceFactory xMSF, String sCommand)
    {
        SystemDialog.showErrorBox(xMSF, "RID_REPORT_65", "<STATEMENT>", sCommand); // Querycreationnotpossible
    }
    // TODO don't show messages in Exceptions
    public InvalidQueryException(XMultiServiceFactory xMSF, String sCommand, Throwable cause)
    {
        super(cause);
        SystemDialog.showErrorBox(xMSF, "RID_REPORT_65", "<STATEMENT>", sCommand); // Querycreationnotpossible
    }
}

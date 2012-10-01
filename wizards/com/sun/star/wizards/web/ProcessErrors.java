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
package com.sun.star.wizards.web;

/**
 * Error IDs for errors that can accure
 * in the interaction with the Process class.
 */
public interface ProcessErrors
{

    public final static int ERROR_MKDIR = 0;
    public final static int ERROR_EXPORT = 1;
    public final static int ERROR_EXPORT_MKDIR = 2;
    public final static int ERROR_DOC_VALIDATE = 3;
    public final static int ERROR_EXPORT_IO = 4;
    public final static int ERROR_EXPORT_SECURITY = 5;
    public final static int ERROR_GENERATE_XSLT = 6;
    public final static int ERROR_GENERATE_COPY = 7;
    public final static int ERROR_PUBLISH = 8;
    public final static int ERROR_PUBLISH_MEDIA = 9;
    public final static int ERROR_CLEANUP = 10;
}

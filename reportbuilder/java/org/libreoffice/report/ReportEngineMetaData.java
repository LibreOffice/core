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
package org.libreoffice.report;

public interface ReportEngineMetaData
{

    /**
     * Checks, whether a certain output type is available.
     * Feed the mime-type of the output type in and you'll get
     * a true or false back.
     *
     * @param mimeType
     * @return true, if the output type is supported; false otherwise.
     */
    boolean isOutputSupported(String mimeType);

    /**
     * Lists all supported output parameters for the given mime-type.
     * This listing can be used to build a generic user interface for
     * configuring a certain output.
     *
     * @param mimeType
     * @return
     */
    String[] getOutputParameters(String mimeType);

    Class getParameterType(String parameter);

    boolean isMandatory(String parameter);

    boolean isEnumeration(String parameter);

    Object[] getEnumerationValues(String parameter);
}

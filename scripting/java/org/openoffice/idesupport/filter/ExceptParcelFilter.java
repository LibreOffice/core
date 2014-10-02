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

package org.openoffice.idesupport.filter;

public class ExceptParcelFilter implements FileFilter {
    private static final String DESCRIPTION = "Remove specified Parcel";
    private static final ExceptParcelFilter filter = new ExceptParcelFilter();
    private static String parcelName = null;

    private ExceptParcelFilter() {
    }

    public void setParcelToRemove(String parcelName) {
        ExceptParcelFilter.parcelName = parcelName;
    }

    public static ExceptParcelFilter getInstance() {
        return filter;
    }

    public boolean validate(String name) {
        return name.startsWith(ExceptParcelFilter.parcelName);
    }

    @Override
    public String toString() {
        return DESCRIPTION + ": " + "<" + ExceptParcelFilter.parcelName + ">";
    }
}

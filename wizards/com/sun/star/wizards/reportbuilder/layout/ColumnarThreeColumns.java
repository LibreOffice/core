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
package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.wizards.common.Resource;
import com.sun.star.wizards.report.IReportDefinitionReadAccess;
import com.sun.star.wizards.ui.*;

public class ColumnarThreeColumns extends ColumnarTwoColumns
{

    public ColumnarThreeColumns(IReportDefinitionReadAccess _xDefinitionAccess, Resource _aResource)
    {
        super(_xDefinitionAccess, _aResource);
    }

    @Override
    public String getName()
    {
        return "ColumnarThreeColumnsLayoutOfData";
    }

    @Override
    public String getLocalizedName()
    {
        return getResource().getResText(UIConsts.RID_REPORT + 83);
    }

    @Override
    protected void insertDetailFields()
    {
        insertDetailFields(3);
    }
}

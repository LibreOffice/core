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
package org.libreoffice.report.pentaho.parser.stylemapper.style;

import org.libreoffice.report.pentaho.parser.stylemapper.OneOfConstantsMapper;

import org.jfree.layouting.input.style.keys.line.LineStyleKeys;
import org.jfree.layouting.input.style.values.CSSAutoValue;
import org.jfree.layouting.input.style.values.CSSConstant;

public class VerticalAlignMapper extends OneOfConstantsMapper
{

    public VerticalAlignMapper()
    {
        super(LineStyleKeys.VERTICAL_ALIGN);
        addMapping("top", new CSSConstant("top"));
        addMapping("bottom", new CSSConstant("bottom"));
        addMapping("middle", new CSSConstant("middle"));
        addMapping("baseline", new CSSConstant("baseline"));
        addMapping("auto", CSSAutoValue.getInstance());
    }
}

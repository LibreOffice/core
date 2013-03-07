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
package org.libreoffice.report.pentaho.parser.stylemapper.fo;

import org.libreoffice.report.pentaho.parser.stylemapper.OneOfConstantsMapper;

import org.jfree.layouting.input.style.keys.text.TextAlign;
import org.jfree.layouting.input.style.keys.text.TextStyleKeys;

public class TextAlignMapper extends OneOfConstantsMapper
{

    public TextAlignMapper()
    {
        super(TextStyleKeys.TEXT_ALIGN);
        addMapping("start", TextAlign.START);
        addMapping("end", TextAlign.END);
        addMapping("left", TextAlign.LEFT);
        addMapping("center", TextAlign.CENTER);
        addMapping("right", TextAlign.RIGHT);
        addMapping("justify", TextAlign.JUSTIFY);
    }
}

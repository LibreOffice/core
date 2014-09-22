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
package com.sun.star.wizards.db;

import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;

public class BlindtextCreator
{

    public static final String BlindText =
            "Ut wisi enim ad minim veniam, quis nostrud exerci tation " + "ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor " + "in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at " + "vero et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore " + "te feugait nulla facilisi. Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy " + "nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, " + "quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. " + "Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum " + "dolore eu feugiat nulla facilisis at vero et accumsan et iusto odio dignissim qui blandit praesent " + "luptatum zzril delenit augue duis dolore te feugait nulla facilisi. Nam liber tempor cum soluta nobis " + "eleifend option congue nihil imperdiet doming id quod mazim placerat facer possim assum.";

    public static String adjustBlindTextlength(String FieldTitle, int FieldWidth, boolean bIsCurLandscape, boolean bIsGroupTable, String[] _RecordFieldNames)
    {
        String BlindTextString = PropertyNames.EMPTY_STRING;
        if (bIsGroupTable)
        {
            return getBlindTextString(FieldTitle, FieldWidth, FieldWidth);
        }
        int MaxFieldCount = getMaxFieldCount(bIsCurLandscape);
        if (_RecordFieldNames.length <= 2 * MaxFieldCount)
        {
            if (_RecordFieldNames.length <= MaxFieldCount)
            {
                BlindTextString = getBlindTextString(FieldTitle, FieldWidth, FieldWidth);
            }
            else
            {
                BlindTextString = getBlindTextString(FieldTitle, FieldWidth, (int) (0.5 * FieldWidth));
            }
        }
        else
        {
            BlindTextString = getBlindTextString(FieldTitle, FieldWidth, (int) 1.1 * FieldTitle.length());
        }
        return BlindTextString;
    }

    public static String getBlindTextString(String FieldTitle, int FieldWidth, int MaxWidth)
    {
        String[] BlindTextArray = JavaTools.ArrayoutofString(BlindText, PropertyNames.SPACE);
        String PartBlindText = BlindTextArray[0];
        String NewPartBlindText;
        int MaxHeaderWidth;
        int Titlelength = (int) 1.1 * FieldTitle.length(); // We assume that the TableHeading is bold

        if (Titlelength > PartBlindText.length())
        {
            MaxHeaderWidth = Titlelength;
        }
        else
        {
            MaxHeaderWidth = PartBlindText.length();
        }
        if (MaxHeaderWidth > MaxWidth)
        {
            MaxWidth = MaxHeaderWidth;
        }
        int i = 1;
        do
        {
            NewPartBlindText = PartBlindText + PropertyNames.SPACE + BlindTextArray[i];
            if (NewPartBlindText.length() < MaxWidth)
            {
                PartBlindText = NewPartBlindText;
                i += 1;
            }
        }
        while (NewPartBlindText.length() < MaxWidth);
        return PartBlindText;
    }

    private static int getMaxFieldCount(boolean bIsCurLandscape)
    {
        if (bIsCurLandscape)
        {
            return 5;
        }
        else
        {
            return 3;
        }
    }
}

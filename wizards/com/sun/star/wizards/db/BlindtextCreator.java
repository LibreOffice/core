/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.wizards.db;

import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;

/**
 *
 * @author  bc93774
 */
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

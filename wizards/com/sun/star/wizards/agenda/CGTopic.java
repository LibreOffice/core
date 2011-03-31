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
package com.sun.star.wizards.agenda;


import com.sun.star.beans.PropertyValue;
import com.sun.star.wizards.common.ConfigGroup;
import com.sun.star.wizards.common.Indexable;
import com.sun.star.wizards.common.PropertyNames;

/**
 * CGTopic means: Configuration Group Topic.
 * This object encapsulates a configuration group with topic information.
 * Since the topics gui conftrol uses its own data model, there is
 * also code here to convert from the data model to CGTopic object (the constructor)
 * and vice versa (setDataToRow method - used when loading the last session...)
 */
public class CGTopic extends ConfigGroup implements Indexable {

    /** sort order  */
    public int cp_Index;
    /** topic name  */
    public String cp_Topic;
    /** responsible */
    public String cp_Responsible;
    /** time */
    public String cp_Time;

    public CGTopic() {}

    /**
     * create a new CGTopic object with data from the given row.
     * the row object is a PropertyValue array, as used
     * by the TopicsControl's data model.
     * @param row PropertyValue array as used by the TopicsControl data model.
     */
    public CGTopic( Object row) {
        PropertyValue[] pv = (PropertyValue[])row;
        String num = (String)pv[0].Value;
        cp_Index = Integer.valueOf(num.substring(0,num.length() - 1)).intValue();
        cp_Topic = (String)pv[1].Value;
        cp_Responsible = (String)pv[2].Value;
        cp_Time = (String)pv[3].Value;
    }

    /**
     * copies the data in this CGTopic object
     * to the given row.
     * @param row the row object (PropertyValue array) to
     * copy the data to.
     */
    public void setDataToRow(Object row) {
        PropertyValue[] pv = (PropertyValue[])row;
        pv[0].Value = PropertyNames.EMPTY_STRING + cp_Index + ".";
        pv[1].Value = cp_Topic;
        pv[2].Value = cp_Responsible;
        pv[3].Value = cp_Time;
    }

    public int getIndex() {
        return cp_Index;
    }

}

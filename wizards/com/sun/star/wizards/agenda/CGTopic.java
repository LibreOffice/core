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

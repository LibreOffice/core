/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CGTopic.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:16:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/package com.sun.star.wizards.agenda;


import com.sun.star.beans.PropertyValue;
import com.sun.star.wizards.common.ConfigGroup;
import com.sun.star.wizards.common.Indexable;

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
        pv[0].Value = "" + cp_Index + ".";
        pv[1].Value = cp_Topic;
        pv[2].Value = cp_Responsible;
        pv[3].Value = cp_Time;
    }

    public int getIndex() {
        return cp_Index;
    }

}

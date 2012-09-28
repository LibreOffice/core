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

package helper;

import com.sun.star.inspection.PropertyCategoryDescriptor;
import com.sun.star.inspection.XObjectInspectorModel;

/**
 * This is an implementation of <CODE>ObjectInspectorModel</CODE>.
 * @see com.sun.star.inspection.XObjectInspectorModel
 */
public class ObjectInspectorModelImpl implements XObjectInspectorModel{

    /**
     * class variable which contains the implementations of
     * <CODE>PropertyCategoryDescriptor</CODE>
     * @see com.sun.star.inspection.PropertyCategoryDescriptor
     */
    PropertyCategoryDescriptor[] m_Categories;

    /**
     * class variable which contains the count of implementations of
     *  <CODE>PropertyCategoryDescriptor</CODE>
     */
    int m_count;

    /**
     * Creates a new instance of ObjectInspectorModelImpl
     * For every count given in parameter <CODE>count</CODE> an
     * <CODE>PropertyCategoryDescriptor</CODE> was created an filled with valuable content.
     * @param count count of <CODE>PropertyCategoryDescriptor</CODE> to create
     */
    public ObjectInspectorModelImpl(int count) {
        m_count = count;
        m_Categories = new PropertyCategoryDescriptor[m_count];
        int CategoryMem = 0;
        int inCat = 0;
        for (int n=0; n < m_count; n++ ){

            m_Categories[n] = new PropertyCategoryDescriptor();

            int category = n / 2;
            inCat =(CategoryMem == category)? ++inCat: 1;
            CategoryMem = category;

            //System.out.println("Category" + category + "Number" + inCat);
            m_Categories[n].ProgrammaticName = "Category" + category;
            m_Categories[n].UIName = "Category" + category + "Number" + inCat;
            m_Categories[n].HelpURL = "h:" + n;
        }
    }

    /**
     * returns the catrgories
     * @return returns the catrgories
     */
    public PropertyCategoryDescriptor[] describeCategories() {
        return m_Categories;
    }

    /**
     * returns currently nothing
     * @return nothing
     */
    public Object[] getHandlerFactories() {
        return null;
    }

    /** determines whether the object inspector should have a help section
        @return false
    */
    public boolean getHasHelpSection() {
        return false;
    }

    /** returns minimum number of lines in the help text section.
        @return 3
    */
    public int getMinHelpTextLines() {
        return 3;
    }

    /** returns maximum number of lines in the help text section.
        @return 8
    */
    public int getMaxHelpTextLines() {
        return 8;
    }

    /** returns whether or not the inspector's UI should be read-only
    */
    public boolean getIsReadOnly() {
        return false;
    }

    /** sets the inspector's read-only state
    */
    public void setIsReadOnly( boolean _IsReadOnly ) {
        // not supported, and not used so far in our test cases
    }

    /**
     * retrieves an index in a global property ordering, for a given property name
     * @param UIName the property whose global order index should be retrieved
     * @throws com.sun.star.beans.UnknownPropertyException if the given property is unknown
     * @return the global order index of PropertyName
     */
    public int getPropertyOrderIndex(String UIName) {
        int index = 0;
        for (int n=0; n < m_Categories.length; n++){
            if (m_Categories[n].UIName.equals(UIName)){
                index = n;
                break;
            }
        }
        return index;
    }

 }

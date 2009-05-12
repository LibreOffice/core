/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ObjectInspectorModelImpl.java,v $
 * $Revision: 1.6 $
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
 ************************************************************************
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
    };

    /** returns maximum number of lines in the help text section.
        @return 8
    */
    public int getMaxHelpTextLines() {
        return 8;
    };

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

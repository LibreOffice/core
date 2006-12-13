/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectInspectorModelImpl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 11:53:35 $
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

/*************************************************************************
 *
 *  $RCSfile: DataAware.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:08:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */

package com.sun.star.wizards.ui.event;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public abstract class DataAware {
    protected static Class[] EMPTY_ARRAY = new Class[0];

    protected Object dataObject;
    //protected Method setMethod;
    //protected Method getMethod;
    protected Value value;

    protected DataAware(Object dataObject_, Value value_) {
        dataObject = dataObject_;
        value = value_;
        //getMethod = createGetMethod(dataPropName, dataObject);
        //setMethod = createSetMethod(dataPropName, dataObject, getMethod.getReturnType());
    }

    public Object getDataObject() {
        return dataObject;
    }

    public void setDataObject(Object obj, boolean updateUI) {

        if (!value.isAssignable(obj.getClass()))
            throw new ClassCastException("can not cast new DataObject to original Class");

        dataObject = obj;

        if (updateUI)
            updateUI();

    }

    protected void setToData(Object newValue) {
        value.set(newValue,getDataObject());
    }

    protected Object getFromData() {
        return value.get(getDataObject());
    }

    protected abstract void setToUI(Object newValue);

    protected abstract Object getFromUI();

    public void updateUI() {
        Object data = getFromData();
        Object ui = getFromUI();
        if (!equals(data, ui))
            try {
                setToUI(data);
            } catch (Exception ex) {
                ex.printStackTrace();
                //TODO tell user...
            }
        enableControls(data);
    }

    protected void enableControls(Object currentValue) {
    }

    public void updateData() {
        Object data = getFromData();
        Object ui = getFromUI();
        if (!equals(data, ui))
            setToData(ui);
        enableControls(ui);
    }

    public interface Listener {
        public void eventPerformed(Object event);
    }

    protected boolean equals(Object a, Object b) {
        if (a == null && b == null)
            return true;
        if (a == null || b == null)
            return false;
        if (a.getClass().isArray()) {
            if (b.getClass().isArray())
                return Arrays.equals((short[]) a, (short[]) b);
            else
                return false;
        }
        return a.equals(b);
    }

    public static void updateUI(Collection dataAwares) {
        for (Iterator i = dataAwares.iterator(); i.hasNext();)
             ((DataAware) i.next()).updateUI();
    }

    public static void updateData(Collection dataAwares) {
        for (Iterator i = dataAwares.iterator(); i.hasNext();)
             ((DataAware) i.next()).updateData();
    }

    public static void setDataObject(Collection dataAwares, Object dataObject, boolean updateUI) {
        for (Iterator i = dataAwares.iterator(); i.hasNext();)
             ((DataAware) i.next()).setDataObject(dataObject, updateUI);
    }

    public interface Value {
        public Object get(Object target);
        public void set(Object value, Object target);
        public boolean isAssignable(Class type);
    }

    public static class PropertyValue implements Value {

        private Method getMethod;
        private Method setMethod;

        public PropertyValue(String propertyName, Object propertyOwner) {
            getMethod = createGetMethod(propertyName, propertyOwner);
            setMethod = createSetMethod(propertyName, propertyOwner, getMethod.getReturnType());
        }

        protected Method createGetMethod(String propName, Object obj) {
            Method m = null;
            try { //try to get a "get" method.
                m = obj.getClass().getMethod("get" + propName, EMPTY_ARRAY);
            } catch (NoSuchMethodException ex1) {
                throw new IllegalArgumentException("get" + propName + "() method does not exist on " + obj.getClass().getName());
            }
            return m;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target) {
            try {
                return getMethod.invoke(target, EMPTY_ARRAY);
            } catch (IllegalAccessException ex1) {
                ex1.printStackTrace();
            } catch (InvocationTargetException ex2) {
                ex2.printStackTrace();
            } catch (NullPointerException npe) {
                if (getMethod.getReturnType().equals(String.class))
                    return "";
                if (getMethod.getReturnType().equals(Short.class))
                    return new Short((short) 0);
                if (getMethod.getReturnType().equals(Integer.class))
                    return new Integer(0);
                if (getMethod.getReturnType().equals(short[].class))
                    return new short[0];
            }
            return null;

        }

        protected Method createSetMethod(String propName, Object obj, Class paramClass) {
            Method m = null;
            try {
                m = obj.getClass().getMethod("set" + propName, new Class[] { paramClass });
            } catch (NoSuchMethodException ex1) {
                throw new IllegalArgumentException("set" + propName + "(" + getMethod.getReturnType().getName() + ") method does not exist on " + obj.getClass().getName());
            }
            return m;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#set(java.lang.Object, java.lang.Object)
         */
        public void set(Object value, Object target) {
            try {
                setMethod.invoke(target, new Object[] {value});
            } catch (IllegalAccessException ex1) {
                ex1.printStackTrace();
            } catch (InvocationTargetException ex2) {
                ex2.printStackTrace();
            }
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#isAssignable(java.lang.Class)
         */
        public boolean isAssignable(Class type) {
            return getMethod.getDeclaringClass().isAssignableFrom(type) &&
                setMethod.getDeclaringClass().isAssignableFrom(type);
        }
    }
}

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
package lib;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.UnoRuntime;

import java.lang.reflect.Method;

import util.ValueChanger;
import util.ValueComparer;
import util.utils;

import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
 * MultiPropertyTest extends the functionality of MultiMethodTest to support
 * services testing. Since, in most cases, service tests has one method testing
 * most of its properties, the MultiPropertyTest provides unified version of
 * the method: testProperty().
 *
 * <p>The testProperty() is called, when the MultiMethodTest's testing method
 * is not found in the subclass. So, by defining such methods for properties
 * the standard testing behavioutr can be changed.
 *
 * <p>The testing behaviour also can be changed by overriding compare(),
 * getNewVAlue() or toString(Object) methods, or by extending PropertyTester
 * class.
 *
 * @see MultiMethodTest
 * @see #testProperty(String)
 * @see #testProperty(String, PropertyTester)
 * @see #getNewValue
 * @see #compare
 * @see #toString(Object)
 */
public class MultiPropertyTest extends MultiMethodTest
{

    /**
     * Contains a XPropertySet interface of the tested object. Is initialized
     * in MultiMethodTest code.
     */
    public XPropertySet oObj;
    protected boolean optionalService = false;

    /**
     * Overrides super.before() to check the service is supported by the object.
     */
    protected void before()
    {
        XServiceInfo xInfo = UnoRuntime.queryInterface(
                XServiceInfo.class, oObj);

        optionalService = entry.isOptional;

        String theService = getTestedClassName();
        if (xInfo != null && !xInfo.supportsService(theService))
        {
            log.println("Service " + theService + " not available");
            if (optionalService)
            {
                log.println("This is OK since it is optional");
            }
            else
            {
                Status.failed(theService + " is not supported");
            }
        }
    }

    /**
     * Overrides MultiMethodTest.invokeTestMethod(). If the test for the
     * <code>meth</code> is not available (<code>meth</code> == <tt>null</tt>)
     * calls testProperty method for the method. Otherwise calls
     * super.invokeTestMethod().
     *
     * @see MultiMethodTest#invokeTestMethod
     */
    protected void invokeTestMethod(Method meth, String methName)
    {
        if (meth != null)
        {
            super.invokeTestMethod(meth, methName);
        }
        else
        {
            testProperty(methName);
        }
    }

    /**
     * PropertyTester class defines how to test a property and defined
     * to allow subclasses of MultiPropertyTest to change the testing
     * behaviour more flexible, since the behaviour can be customized for
     * each property separately, by providing subclass of PropertyTester
     * and passing it to testProperty(String, PropertyTester method).
     */
    public class PropertyTester
    {

        /**
         * The method defines the whole process of testing propName
         * property.
         *
         * <p>First, it checks if the property exists(it maybe optional).
         * Then, a value to set the property with is calculated with
         * getNewValue method. Normally, the new value is calculated
         * based on old value, but subclasses can override the behaviour
         * (for example, if old value is null) and specify their own value.
         * Then the property is set with that new value and the result(
         * it maybe an exception too, for example a PropertyVetoException)
         * is checked with checkResult method.
         *
         * @param propName - the property to test.
         * @result - adds the result of testing propName property to
         *           MultiMethodTest.tRes.
         */
        protected void testProperty(String propName)
        {
            XPropertySetInfo info = oObj.getPropertySetInfo();

            if (info != null)
            {
                final boolean bHasProperty = info.hasPropertyByName(propName);
                if (!bHasProperty)
                {
                    if (isOptional(propName) || optionalService)
                    {
                        // skipping optional property test
                        log.println("Property '" + propName + "' is optional and not supported");
                        tRes.tested(propName, true);
                        return;
                    }
                    else
                    {
                        // cannot test the property
                        log.println("Tested XPropertySet does not contain'" + propName + "' property");
                        tRes.tested(propName, false);
                        return;
                    }
                }
            }

            try
            {
                Object oldValue = oObj.getPropertyValue(propName);

                if( (oldValue==null) || utils.isVoid(oldValue) )
                {
                    // #i111560# method getNewValue() does not work with an empty oldValue
                    Property prop = info.getPropertyByName(propName);
                    if( (prop.Attributes & PropertyAttribute.MAYBEVOID) != 0 )
                    {
                        // todo: implement a new test independent from method getNewValue()
                        log.println("changing initially empty MAYBEVOID properties is not supported by the test framework so far - skip test of property: " + propName);
                        tRes.tested(propName, true);
                        return;
                    }
                    else
                    {
                        log.println( "property '"+propName+"' is not set but is not MAYBEVOID");
                        tRes.tested(propName, false);
                        return;
                    }
                }

                Object newValue;

                // trying to create new value
                try
                {
                    newValue = getNewValue(propName, oldValue);
                }
                catch (java.lang.IllegalArgumentException e)
                {
                    // skipping test since new value is not available
                    Status.failed("Cannot create new value for '" + propName + " : " + e.getMessage());
                    return;
                }

                // for an exception thrown during setting new value
                // to pass it to checkResult method
                Exception exception = null;

                try
                {
                    log.println("try to set:");
                    log.println("old = " + toString(oldValue));
                    log.println("new = " + toString(newValue));
                    oObj.setPropertyValue(propName, newValue);
                }
                catch (IllegalArgumentException e)
                {
                    exception = e;
                }
                catch (PropertyVetoException e)
                {
                    exception = e;
                }
                catch (WrappedTargetException e)
                {
                    exception = e;
                }
                catch (UnknownPropertyException e)
                {
                    exception = e;
                }
                catch (RuntimeException e)
                {
                    exception = e;
                }

                // getting result value
                Object resValue = oObj.getPropertyValue(propName);

                // checking results
                checkResult(propName, oldValue, newValue, resValue, exception);
            }
            catch (Exception e)
            {
                log.println("Exception occurred while testing property '" + propName + "'");
                e.printStackTrace(log);
                tRes.tested(propName, false);
            }
        }

        /**
         * The method checks result of setting a new value to the
         * property based o the following arguments:
         * @param propName - the property to test
         * @param oldValue - the old value of the property, before changing it.
         * @param newValue - the new value the property has been set with
         * @param resValue - the value of the property after having changed it
         * @param exception - if not null - the exception thrown by
         *                 XPropertySet.setPropertyValue, else indicates
         *                 normal method completion.
         *
         * <p>If the property is READ_ONLY, than either PropertyVetoException
         * should be thrown or the value of property should not have changed
         * (resValue is compared with oldValue with compare method).
         *
         * <p>If the property is not READ_ONLY, checks that the new value has
         * been successfully set(resValue is compared with newValue with
         * compare method).
         *
         * <p>If the exception is not null then(except the case of read-only
         * property and PropertyVetoException above) it is rethrown to allow
         * further catching it if needed.
         *
         * <p>Subclasses can override to change this behaviour.
         */
        protected void checkResult(String propName, Object oldValue,
                Object newValue, Object resValue, Exception exception)
                throws Exception
        {
            XPropertySetInfo info = oObj.getPropertySetInfo();
            if (info == null)
            {
                log.println("Can't get XPropertySetInfo for property " + propName);
                tRes.tested(propName, false);
                return;
            }
            Property prop = info.getPropertyByName(propName);

            boolean readOnly = (prop.Attributes & PropertyAttribute.READONLY) != 0;
            boolean maybeVoid = (prop.Attributes & PropertyAttribute.MAYBEVOID) != 0;
            //check get-set methods
            if (maybeVoid)
            {
                log.println("Property " + propName + " is void");
            }
            if (readOnly)
            {
                log.println("Property " + propName + " is readOnly");
            }
            if (util.utils.isVoid(oldValue) && !maybeVoid)
            {
                log.println(propName + " is void, but it's not MAYBEVOID");
                tRes.tested(propName, false);
            }
            else if (oldValue == null)
            {
                log.println(propName + " has null value, and therefore can't be changed");
                tRes.tested(propName, true);
            }
            else if (readOnly)
            {
                // check if exception was thrown
                if (exception != null)
                {
                    if (exception instanceof PropertyVetoException)
                    {
                        // the change of read only prohibited - OK
                        log.println("Property is ReadOnly and wasn't changed");
                        log.println("Property '" + propName + "' OK");
                        tRes.tested(propName, true);
                    }
                    else if (exception instanceof IllegalArgumentException)
                    {
                        // the change of read only prohibited - OK
                        log.println("Property is ReadOnly and wasn't changed");
                        log.println("Property '" + propName + "' OK");
                        tRes.tested(propName, true);
                    }
                    else if (exception instanceof UnknownPropertyException)
                    {
                        // the change of read only prohibited - OK
                        log.println("Property is ReadOnly and wasn't changed");
                        log.println("Property '" + propName + "' OK");
                        tRes.tested(propName, true);
                    }
                    else if (exception instanceof RuntimeException)
                    {
                        // the change of read only prohibited - OK
                        log.println("Property is ReadOnly and wasn't changed");
                        log.println("Property '" + propName + "' OK");
                        tRes.tested(propName, true);
                    }
                    else
                    {
                        throw exception;
                    }
                }
                else
                {
                    // if no exception - check that value
                    // has not changed
                    if (!compare(resValue, oldValue))
                    {
                        log.println("Read only property '" + propName + "' has changed");
                        try
                        {
                            if (!util.utils.isVoid(oldValue) && oldValue instanceof Any)
                            {
                                oldValue = AnyConverter.toObject(new Type(((Any) oldValue).getClass()), oldValue);
                            }
//                            log.println("old = " + toString(oldValue));
//                            log.println("new = " + toString(newValue));
                            log.println("result = " + toString(resValue));
                        }
                        catch (com.sun.star.lang.IllegalArgumentException iae)
                        {
                            log.println("NOTIFY: this property needs further investigations.");
                            log.println("\t The type seems to be an Any with value of NULL.");
                            log.println("\t Maybe the property should get it's own test method.");
                        }

                        tRes.tested(propName, false);
                    }
                    else
                    {
                        log.println("Read only property '" + propName + "' hasn't changed");
                        log.println("Property '" + propName + "' OK");
                        tRes.tested(propName, true);
                    }
                }
            }
            else
            {
                if (exception == null)
                {
                    // if no exception thrown
                    // check that the new value is set
                    if ((!compare(resValue, newValue)) || (compare(resValue, oldValue)))
                    {
                        log.println("Value for '" + propName + "' hasn't changed as expected");
                        try
                        {
                            if (!util.utils.isVoid(oldValue) && oldValue instanceof Any)
                            {
                                oldValue = AnyConverter.toObject(new Type(((Any) oldValue).getClass()), oldValue);
                            }
//                            log.println("old = " + toString(oldValue));
//                            log.println("new = " + toString(newValue));
                            log.println("result = " + toString(resValue));
                        }
                        catch (com.sun.star.lang.IllegalArgumentException iae)
                        {
                            log.println("NOTIFY: this property needs further investigations.");
                            log.println("\t The type seems to be an Any with value of NULL.");
                            log.println("\t Maybe the property should get it's own test method.");
                        }
                        if (resValue != null)
                        {
                            if ((!compare(resValue, oldValue)) || (!resValue.equals(oldValue)))
                            {
                                log.println("But it has changed.");
                                tRes.tested(propName, true);
                            }
                            else
                            {
                                tRes.tested(propName, false);
                            }
                        }
                        else
                        {
                            tRes.tested(propName, false);
                        }
                        //tRes.tested(propName, false);
                    }
                    else
                    {
                        log.println("Property '" + propName + "' OK");
                        try
                        {
                            if (!util.utils.isVoid(oldValue) && oldValue instanceof Any)
                            {
                                oldValue = AnyConverter.toObject(new Type(((Any) oldValue).getClass()), oldValue);
                            }
//                            log.println("old = " + toString(oldValue));
//                            log.println("new = " + toString(newValue));
                            log.println("result = " + toString(resValue));
                        }
                        catch (com.sun.star.lang.IllegalArgumentException iae)
                        {
                        }
                        tRes.tested(propName, true);
                    }
                }
                else
                {
                    throw exception;
                }
            }
        }

        /**
         * The method produces new value of the property from the oldValue.
         * It returns the result of ValueChanger.changePValue method.
         * Subclasses can override the method to return their own value,
         * when the changePValue beahviour is not enough, for example,
         * when oldValue is null.
         */
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException
        {
            return ValueChanger.changePValue(oldValue);
        }

        /**
         * The method compares obj1 and obj2. It calls
         * MultiPropertyTest.compare, but subclasses can override to change
         * the behaviour, since normally compare calls Object.equals method
         * which is not apropriate in some cases(e.g., structs with equals
         * not overridden).
         */
        protected boolean compare(Object obj1, Object obj2)
        {
            return callCompare(obj1, obj2);
        }

        /**
         * The method returns a String representation of the obj. It calls
         * MultipropertyTest.toString(Object), but subclasses can override
         * to change the behaviour.
         */
        protected String toString(Object obj)
        {
            return callToString(obj);
        }
    }

    /**
     * Extension for <code>PropertyTester</code> which switches two
     * different values. <code>getNewValue()</code> method of this
     * class returns one of these two values depending on the
     * old value, so new value is not equal to old value.
     */
    public class PropertyValueSwitcher extends PropertyTester
    {

        Object val1 = null;
        Object val2 = null;

        /**
         * Constructs a property tester with two different values
         * specified as parameters.
         *
         * @param val1 Not <code>null</code> value for the property
         *     tested.
         * @param val2 Not <code>null</code> value for the property
         *     tested which differs from the first value.
         */
        public PropertyValueSwitcher(Object val1, Object val2)
        {
            this.val1 = val1;
            this.val2 = val2;
        }

        /**
         * Overriden method of <code>PropertyTester</code> which
         * retruns new value from two values specified.
         *
         * @return The second value if old value is equal to the first
         * one, the first value otherwise.
         */
        protected Object getNewValue(String propName, Object old)
        {
            if (ValueComparer.equalValue(val1, old))
            {
                return val2;
            }
            else
            {
                return val1;
            }
        }
    }

    /**
     * The method performs testing of propName property using propTester.
     */
    protected void testProperty(String propName, PropertyTester propTester)
    {
        propTester.testProperty(propName);
    }

    /**
     * The method performs testing of propName property. It uses PropertyTester
     * instance for testing.
     */
    protected void testProperty(String propName)
    {
        testProperty(propName, new PropertyTester());
    }

    /**
     * Tests the property using <code>PropertyValueSwitcher</code>
     * tester and two values for this property.
     *
     * @see PropertyValueSwitcher
     */
    protected void testProperty(String propName, Object val1, Object val2)
    {
        testProperty(propName, new PropertyValueSwitcher(val1, val2));
    }

    /**
     * The method just calls compare. This is a workaround to CodeWarrior's
     * compiler bug.
     */
    private boolean callCompare(Object obj1, Object obj2)
    {
        return compare(obj1, obj2);
    }

    /**
     * Compares two object. In the implementation calls obj1.equals(obj2).
     */
    protected boolean compare(Object obj1, Object obj2)
    {
        return ValueComparer.equalValue(obj1, obj2);
    }

    /**
     * The method just calls toString. This is a workaround to
     * CodeWarrior's compiler bug.
     */
    private String callToString(Object obj)
    {
        return toString(obj);
    }

    /**
     * Gets string representation of the obj. In the implementation
     * returns obj.toString().
     */
    protected String toString(Object obj)
    {
        return obj == null ? "null" : obj.toString();
    }
}

/*
 * AbstractListener.java
 *
 * Created on 1. September 2003, 13:22
 */
package com.sun.star.wizards.ui.event;

import com.sun.star.awt.XControl;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;

import java.lang.reflect.InvocationTargetException;
import java.util.Hashtable;

/**
 * This class is a base class for listener classes.
 * It uses a hashtable to map between a ComponentName, EventName and a MethodInvokation Object.
 * To use this class do the following:<br/>
 * <list>
 * <li>Write a subclass which implements the needed Listener(s).</li>
 * in the even methods, use invoke(...).
 * <li>When instanciating the component, register the subclass as the event listener.</li>
 * <li>Write the methods which should be performed when the event occures.</li>
 * <li>call the "add" method, to define a component-event-action mapping.</li>
 * </list>
 * @author  rpiterman
 */
public class AbstractListener {
    private Hashtable mHashtable = new Hashtable();

    /** Creates a new instance of AbstractListener */
    public AbstractListener() {
    }

    public void add(String componentName, String eventName, String methodName, Object target) {
        try {
            add(componentName, eventName, new MethodInvocation(methodName, target));
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public void add(String componentName, String eventName, MethodInvocation mi) {
        mHashtable.put(componentName + eventName, mi);
    }

    public MethodInvocation get(String componentName, String eventName) {
        return (MethodInvocation) mHashtable.get(componentName + eventName);
    }

    public Object invoke(String componentName, String eventName, Object param) {
        try {
            MethodInvocation mi = get(componentName, eventName);
            if (mi != null)
                return mi.invoke(param);
            else
                return null;
        } catch (InvocationTargetException ite) {
            ite.printStackTrace();
            throw (RuntimeException) ite.getTargetException();
        } catch (IllegalAccessException iae) {
            iae.printStackTrace();
        }

        return null;
    }

    /**
     * Rerurns the property "name" of the Object which is the source of the event.
     */
    public static String getEventSourceName(EventObject eventObject) {
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, eventObject.Source);
        return (String) Helper.getUnoPropertyValue(xControl.getModel(), "Name", String.class);
    }
}

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

import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.inspection.LineDescriptor;
import com.sun.star.inspection.XPropertyControlFactory;
import com.sun.star.inspection.XPropertyHandler;

/**
 * This implementation of <CODE>PropertyHandler</CODE> do currently nothig.
 * All methods are implemented, but not filled with content.
 * @see com.sun.star.inspection.XPropertyHandler
 */
public class PropertyHandlerImpl implements XPropertyHandler{

    /** Creates a new instance of PropertyHandlerImpl */
    public PropertyHandlerImpl() {
    }

    /**
     * This method currently does nothing
     * @param ActuatingPropertyName the id of the actuating property.
     * @param NewValue the new value of the property
     * @param OldValue the old value of the property
     * @param InspectorUI a callback for updating the object inspector UI
     * @param FirstTimeInit If true , the method is called for the first-time update of the respective property, that is, when the property browser is just initializing with the properties of the introspected object.
     * If false , there was a real ::com::sun::star::beans::XPropertyChangeListener::propertyChange event which triggered the call.
     *
     * In some cases it may be necessary to differentiate between both situations. For instance, if you want to set the value of another property when an actuating property's value changed, you should definately not do this when FirstTimeInit is true .
     * @throws com.sun.star.lang.NullPointerException ::com::sun::star::lang::NullPointerException if InspectorUI is NULL
     */
    public void actuatingPropertyChanged(
            String ActuatingPropertyName,
            Object NewValue,
            Object OldValue,
            com.sun.star.inspection.XObjectInspectorUI InspectorUI,
            boolean FirstTimeInit)
            throws com.sun.star.lang.NullPointerException {
    }

    /**
     * This method currently does nothing
     * @param xEventListener the listener to notify about changes
     */
    public void addEventListener(com.sun.star.lang.XEventListener xEventListener) {
    }

    /**
     * This method currently do nothig
     * @param xPropertyChangeListener the listener to notify about property changes
     * @throws com.sun.star.lang.NullPointerException com::sun::star::lang::NullPointerException if the listener is NULL
     */
    public void addPropertyChangeListener(com.sun.star.beans.XPropertyChangeListener xPropertyChangeListener) throws com.sun.star.lang.NullPointerException {
    }

    /**
     * This method currently does nothing
     * @param PropertyName The name of the property whose value is to be converted.
     * @param PropertyValue The to-be-converted property value.
     * @param ControlValueType The target type of the conversion. This type is determined by the control which is used to display the property, which in turn is determined by the handler itself in describePropertyLine .
     *    Speaking strictly, this is passed for convenience only, since every XPropertyHandler implementation should know exactly which type to expect, since it implicitly determined this type in describePropertyLine by creating an appropriate XPropertyControl .
     *
     * @throws com.sun.star.beans.UnknownPropertyException ::com::sun::star::beans::UnknownPropertyException if the given property is not supported by the property handler
     * @return null
     */
    public Object convertToControlValue(
            String PropertyName,
            Object PropertyValue,
            com.sun.star.uno.Type ControlValueType)
            throws com.sun.star.beans.UnknownPropertyException {
        return null;
    }

    /**
     * This method currently does nothing
     * @param PropertyName The name of the conversion's target property.
     * @param ControlValue The to-be-converted control value. This value has been obtained from an XPropertyControl , using its Value attribute.
     * @throws com.sun.star.beans.UnknownPropertyException ::com::sun::star::beans::UnknownPropertyException if the given property is not supported by the property handler
     * @return null
     */
    public Object convertToPropertyValue(String PropertyName, Object ControlValue) throws com.sun.star.beans.UnknownPropertyException {
        return null;
    }

    /**
     * This method currently does nothing
     * @param PropertyName the name of the property whose user interface is to be described
     * @param  out_Descriptor the descriptor of the property line, to be filled by the XPropertyHandler implementation
     * @param ControlFactory a factory for creating XPropertyControl instances. Must not be NULL .
     * @throws com.sun.star.beans.UnknownPropertyException ::com::sun::star::beans::UnknownPropertyException if the given property is not supported by this handler
     * @throws com.sun.star.lang.NullPointerException ::com::sun::star::lang::NullPointerException if ControlFactory is NULL .
     */
    public LineDescriptor describePropertyLine(
            String PropertyName,
            com.sun.star.inspection.XPropertyControlFactory ControlFactory)
            throws com.sun.star.beans.UnknownPropertyException,
            com.sun.star.lang.NullPointerException {
        return null;
    }

    /**
     * This method currently does nothing
     */
    public void dispose() {
    }

    /**
     * This method currently does nothing
     * @return null
     */
    public String[] getActuatingProperties() {
        return null;
    }

    /**
     * This method currently does nothing
     * @param PropertyName the name of the property whose state is to be retrieved
     * @throws com.sun.star.beans.UnknownPropertyException ::com::sun::star::beans::UnknownPropertyException if the given property is not supported by the property handler
     * @return null
     */
    public com.sun.star.beans.PropertyState getPropertyState(String PropertyName)
    throws com.sun.star.beans.UnknownPropertyException {
        return null;
    }

    /**
     * This method currently does nothing
     * @param PropertyName the name of the property whose value is to be retrieved
     * @throws com.sun.star.beans.UnknownPropertyException ::com::sun::star::beans::UnknownPropertyException if the given property is not supported by the property handler
     * @return null
     */
    public Object getPropertyValue(String PropertyName) throws com.sun.star.beans.UnknownPropertyException {
        return null;
    }

    /**
     * This method currently does nothing
     * @return null
     */
    public String[] getSupersededProperties() {
        return null;
    }

    /**
     * This method currently do nothig
     * @return null
     */
    public com.sun.star.beans.Property[] getSupportedProperties() {
        return null;
    }

    /**
     * This method currently does nothing
     * @param Component the component to inspect. Must not be NULL
     * @throws com.sun.star.lang.NullPointerException com::sun::star::lang::NullPointerException if the component is NULL
     */
    public void inspect(Object Component) throws com.sun.star.lang.NullPointerException {
    }

    /**
     * This method currently does nothing
     * @param PropertyName the name of the property whose composability is to be determined
     * @throws com.sun.star.beans.UnknownPropertyException ::com::sun::star::beans::UnknownPropertyException if the given property is not supported by the property handler
     *
     *
     * @return null
     */
    public boolean isComposable(String PropertyName) throws com.sun.star.beans.UnknownPropertyException {
        return false;
    }

    /**
     * This method currently does nothing
     * @param PropertyName The name of the property whose browse button has been clicked
     * @param Primary true if and only if the primary button has been clicked, false otherwise
     * @param out_Data If the method returns InteractiveSelectionResult::ObtainedValue , then _rData contains the value which has been interactively obtained from the user, and which still needs to be set at the inspected component.
     * @param  InspectorUI     provides access to the object inspector UI. Implementations should use this if the property selection requires non-modal user input. In those cases, onInteractivePropertySelection should return InteractiveSelectionResult::Pending , and the UI for (at least) the property whose input is still pending should be disabled.
     *
     * @throws com.sun.star.beans.UnknownPropertyException ::com::sun::star::beans::UnknownPropertyException if the given property is not supported by the property handler
     * @throws com.sun.star.lang.NullPointerException ::com::sun::star::lang::NullPointerException if InspectorUI is NULL
     * @return null
     */
    public com.sun.star.inspection.InteractiveSelectionResult onInteractivePropertySelection(
            String PropertyName,
            boolean Primary,
            Object[] out_Data,
            com.sun.star.inspection.XObjectInspectorUI InspectorUI)
            throws com.sun.star.beans.UnknownPropertyException,
            com.sun.star.lang.NullPointerException {
        return null;
    }

    /**
     * This method currently do nothig
     * @param xEventListener the listener to be revoked
     */
    public void removeEventListener(com.sun.star.lang.XEventListener xEventListener) {
    }

    /**
     * This method currently does nothing
     * @param xPropertyChangeListener the listener to be revoke
     */
    public void removePropertyChangeListener(com.sun.star.beans.XPropertyChangeListener xPropertyChangeListener) {
    }

    /**
     * This method currently does nothing
     * @param PropertyName the name of the property whose value is to be set
     * @param Value the property value to set
     * @throws com.sun.star.beans.UnknownPropertyException ::com::sun::star::beans::UnknownPropertyException if the given property is not supported by the property handler
     */
    public void setPropertyValue(String PropertyName, Object Value) throws com.sun.star.beans.UnknownPropertyException {
    }

    /**
     * This method currently does nothing
     * @param Suspend Whether the handler is to be suspended true or reactivated ( false ). The latter happens if a handler was successfully suspended, but an external instance vetoed the whole suspension process.
     * @return false
     */
    public boolean suspend(boolean Suspend) {
        return false;
    }

    /**
     * This method currently does nothing
     */
    public void describePropertyLine(String string, LineDescriptor[] lineDescriptor, XPropertyControlFactory xPropertyControlFactory) throws UnknownPropertyException, com.sun.star.lang.NullPointerException {
    }

}

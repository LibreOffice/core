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

package com.sun.star.lib.uno.helper;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.lang.DisposedException;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.WrappedTargetRuntimeException;
import com.sun.star.lang.XComponent;
import com.sun.star.reflection.XCompoundTypeDescription;
import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlField2;
import com.sun.star.reflection.XIdlReflection;
import com.sun.star.reflection.XIndirectTypeDescription;
import com.sun.star.reflection.XInterfaceAttributeTypeDescription2;
import com.sun.star.reflection.XInterfaceMemberTypeDescription;
import com.sun.star.reflection.XInterfaceTypeDescription2;
import com.sun.star.reflection.XStructTypeDescription;
import com.sun.star.reflection.XTypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.DeploymentException;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;

/**
   A helper mixin to implement certain UNO interfaces related to property set
   handling on top of the attributes of a given UNO interface type.

   <p>A client will mix in this class by keeping a reference to an instance of
   this class, and forwarding all methods of (a subset of the interfaces)
   <code>com.sun.star.beans.XPropertySet</code>,
   <code>com.sun.star.beans.XFastPropertySet</code>, and
   <code>com.sun.star.beans.XPropertyAccess</code> to it.</p>

   <p>Client code should not use the monitors associated with instances of this
   class, as they are used for internal purposes.</p>

   @since UDK 3.2
*/
public final class PropertySetMixin {
    /**
       The constructor.

       @param context the component context used by this instance; must not be
       null, and must supply the service
       <code>com.sun.star.reflection.CoreReflection</code> and the singleton
       <code>com.sun.star.reflection.theTypeDescriptionManager</code>

       @param object the client UNO object into which this instance is mixed in;
       must not be null, and must support the given <code>type</code>

       @param type the UNO interface type whose attributes are mapped to
       properties; must not be null, and must represent a UNO interface type

       @param absentOptional a list of optional properties that are not present,
       and should thus not be visible via
       <code>com.sun.star.beans.XPropertySet.getPropertySetInfo</code>,
       <code>com.sun.star.beans.XPropertySet.addPropertyChangeListener</code>,
       <code>com.sun.star.beans.XPropertySet.removePropertyChangeListener<!--
       --></code>,
       <code>com.sun.star.beans.XPropertySet.addVetoableChangeListener</code>,
       and <code>com.sun.star.beans.XPropertySet.<!--
       -->removeVetoableChangeListener</code>; null is treated the same as an
       empty list; if non-null, the given array must not be modified after it is
       passed to this constructor.  For consistency reasons, the given
       <code>absentOptional</code> should only contain the names of attributes
       that represent optional properties that are not present (that is, the
       attribute getters and setters always throw a
       <code>com.sun.star.beans.UnknownPropertyException</code>), and should
       contain each such name only once.  If an optional property is not present
       (that is, the corresponding attribute getter and setter always throw a
       <code>com.sun.star.beans.UnknownPropertyException</code>) but is not
       contained in the given <code>absentOptional</code>, then it will be
       visible via
       <code>com.sun.star.beans.XPropertySet.getPropertySetInfo</code> as a
       <code>com.sun.star.beans.Property</code> with a set
       <code>com.sun.star.beans.PropertyAttribute.OPTIONAL</code>.  If the given
       <code>object</code> does not implement
       <code>com.sun.star.beans.XPropertySet</code>, then the given
       <code>absentOptional</code> is effectively ignored and can be null or
       empty.
    */
    public PropertySetMixin(
        XComponentContext context, XInterface object, Type type,
        String[] absentOptional)
    {
        // assert context != null && object != null && type != null
        //     && type.getTypeClass() == TypeClass.INTERFACE;
        this.context = context;
        this.object = object;
        this.type = type;
        this.absentOptional = absentOptional;
        idlClass = getReflection(type.getTypeName());
        XTypeDescription ifc;
        try {
            ifc = UnoRuntime.queryInterface(
                XTypeDescription.class,
                (UnoRuntime.queryInterface(
                    XHierarchicalNameAccess.class,
                    context.getValueByName(
                        "/singletons/com.sun.star.reflection."
                        + "theTypeDescriptionManager")).
                 getByHierarchicalName(type.getTypeName())));
        } catch (NoSuchElementException e) {
            throw new RuntimeException(
                "unexpected com.sun.star.container.NoSuchElementException: "
                + e.getMessage());
        }
        HashMap<String,PropertyData> map = new HashMap<String,PropertyData>();
        ArrayList<String> handleNames = new ArrayList<String>();
        initProperties(ifc, map, handleNames, new HashSet<String>());
        properties = map;
        handleMap = handleNames.toArray(new String[handleNames.size()]);
    }

    /**
       A method used by clients when implementing UNO interface type attribute
       setter functions.

       <p>First, this method checks whether this instance has already been
       disposed (see {@link #dispose}), and throws a
       <code>com.sun.star.beans.DisposedException</code> if applicable.  For a
       constrained attribute (whose setter can explicitly raise
       <code>com.sun.star.beans.PropertyVetoException</code>), this method
       notifies any <code>com.sun.star.beans.XVetoableChangeListener</code>s.
       For a bound attribute, this method modifies the passed-in
       <code>bound</code> so that it can afterwards be used to notify any
       <code>com.sun.star.beans.XPropertyChangeListener</code>s.  This method
       should be called before storing the new attribute value, and
       <code>bound.notifyListeners()</code> should be called exactly once after
       storing the new attribute value (in case the attribute is bound;
       otherwise, calling <code>bound.notifyListeners()</code> is ignored).
       Furthermore, <code>bound.notifyListeners()</code> and this method have to
       be called from the same thread.</p>

       @param propertyName the name of the property (which is the same as the
       name of the attribute that is going to be set)

       @param oldValue the property value corresponding to the old attribute
       value.  This is only used as
       <code>com.sun.star.beans.PropertyChangeEvent.OldValue</code>, which is
       rather useless, anyway (see &ldquo;Using the Observer Pattern&rdquo; in
       <a href="http://tools.openoffice.org/CodingGuidelines.sxw">
       <cite>OpenOffice.org Coding Guidelines</cite></a>).  If the attribute
       that is going to be set is neither bound nor constrained, or if
       <code>com.sun.star.beans.PropertyChangeEvent.OldValue</code> should not
       be set, {@link Any#VOID} can be used instead.

       @param newValue the property value corresponding to the new
       attribute value.  This is only used as
       <code>com.sun.star.beans.PropertyChangeEvent.NewValue</code>, which is
       rather useless, anyway (see &ldquo;Using the Observer Pattern&rdquo: in
       <a href="http://tools.openoffice.org/CodingGuidelines.sxw">
       <cite>OpenOffice.org Coding Guidelines</cite></a>), <em>unless</em> the
       attribute that is going to be set is constrained.  If the attribute
       that is going to be set is neither bound nor constrained, or if it is
       only bound but
       <code>com.sun.star.beans.PropertyChangeEvent.NewValue</code> should not
       be set, {@link Any#VOID} can be used instead.

       @param bound a reference to a fresh {@link BoundListeners} instance
       (which has not been passed to this method before, and on which
       {@link BoundListeners#notifyListeners} has not yet been called); may only
       be null if the attribute that is going to be set is not bound
    */
    @SuppressWarnings("unchecked")
    public void prepareSet(
        String propertyName, Object oldValue, Object newValue,
        BoundListeners bound)
        throws PropertyVetoException
    {
        // assert properties.get(propertyName) != null;
        Property p = properties.get(propertyName).property;
        ArrayList<XVetoableChangeListener> specificVeto = null;
        ArrayList<XVetoableChangeListener> unspecificVeto = null;
        synchronized (this) {
            if (disposed) {
                throw new DisposedException("disposed", object);
            }
            if ((p.Attributes & PropertyAttribute.CONSTRAINED) != 0) {
                ArrayList<XVetoableChangeListener> o = vetoListeners.get(propertyName);
                if (o != null) {
                    specificVeto = (ArrayList<XVetoableChangeListener>) o.clone();
                }
                o = vetoListeners.get("");
                if (o != null) {
                    unspecificVeto = (ArrayList<XVetoableChangeListener>) o.clone();
                }
            }
            if ((p.Attributes & PropertyAttribute.BOUND) != 0) {
                // assert bound != null;
                ArrayList<XPropertyChangeListener> o = boundListeners.get(propertyName);
                if (o != null) {
                    bound.specificListeners = (ArrayList<XPropertyChangeListener>) o.clone();
                }
                o = boundListeners.get("");
                if (o != null) {
                    bound.unspecificListeners = (ArrayList<XPropertyChangeListener>) o.clone();
                }
            }
        }
        if ((p.Attributes & PropertyAttribute.CONSTRAINED) != 0) {
            PropertyChangeEvent event = new PropertyChangeEvent(
                object, propertyName, false, p.Handle, oldValue, newValue);
            if (specificVeto != null) {
                for (Iterator<XVetoableChangeListener> i = specificVeto.iterator(); i.hasNext();) {
                    try {
                        i.next().vetoableChange(event);
                    } catch (DisposedException e) {}
                }
            }
            if (unspecificVeto != null) {
                for (Iterator<XVetoableChangeListener> i = unspecificVeto.iterator(); i.hasNext();) {
                    try {
                        i.next().vetoableChange(event);
                    } catch (DisposedException e) {}
                }
            }
        }
        if ((p.Attributes & PropertyAttribute.BOUND) != 0) {
            // assert bound != null;
            bound.event = new PropertyChangeEvent(
                object, propertyName, false, p.Handle, oldValue, newValue);
        }
    }

    /**
       A simplified version of {@link #prepareSet(String, Object, Object,
       PropertySetMixin.BoundListeners)}.

       <p>This method is useful for attributes that are not constrained.</p>

       @param propertyName the name of the property (which is the same as the
       name of the attribute that is going to be set)

       @param bound a reference to a fresh {@link BoundListeners} instance
       (which has not been passed to this method before, and on which
       {@link BoundListeners#notifyListeners} has not yet been called); may only
       be null if the attribute that is going to be set is not bound
    */
    public void prepareSet(String propertyName, BoundListeners bound) {
        try {
            prepareSet(propertyName, Any.VOID, Any.VOID, bound);
        } catch (PropertyVetoException e) {
            throw new RuntimeException("unexpected " + e);
        }
    }

    /**
       Marks this instance as being disposed.

       <p>See <code>com.sun.star.lang.XComponent</code> for the general concept
       of disposing UNO objects.  On the first call to this method, all
       registered listeners
       (<code>com.sun.star.beans.XPropertyChangeListener</code>s and
       <code>com.sun.star.beans.XVetoableChangeListener</code>s) are notified of
       the disposing source.  Any subsequent calls to this method are
       ignored.</p>
     */
    public void dispose() {
        HashMap<String,ArrayList<XPropertyChangeListener>> bound;
        HashMap<String,ArrayList<XVetoableChangeListener>> veto;
        synchronized (this) {
            bound = boundListeners;
            boundListeners = null;
            veto = vetoListeners;
            vetoListeners = null;
            disposed = true;
        }
        EventObject event = new EventObject(object);
        if (bound != null) {
            for (Iterator<ArrayList<XPropertyChangeListener>> i = bound.values().iterator(); i.hasNext();) {
                for (Iterator<XPropertyChangeListener> j = i.next().iterator(); j.hasNext();)
                {
                    j.next().disposing(event);
                }
            }
        }
        if (veto != null) {
            for (Iterator<ArrayList<XVetoableChangeListener>> i = veto.values().iterator(); i.hasNext();) {
                for (Iterator<XVetoableChangeListener> j = i.next().iterator(); j.hasNext();)
                {
                    j.next().disposing(event);
                }
            }
        }
    }

    /**
       Implements
       <code>com.sun.star.beans.XPropertySet.getPropertySetInfo</code>.
    */
    public XPropertySetInfo getPropertySetInfo() {
        return new Info(properties);
    }

    /**
       Implements <code>com.sun.star.beans.XPropertySet.setPropertyValue</code>.
    */
    public void setPropertyValue(String propertyName, Object value)
        throws UnknownPropertyException, PropertyVetoException,
        com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        setProperty(propertyName, value, false, false, (short) 1);
    }

    /**
       Implements <code>com.sun.star.beans.XPropertySet.getPropertyValue</code>.
    */
    public Object getPropertyValue(String propertyName)
        throws UnknownPropertyException, WrappedTargetException
    {
        return getProperty(propertyName, null);
    }

    /**
       Implements
       <code>com.sun.star.beans.XPropertySet.addPropertyChangeListener</code>.

       <p>If a listener is added more than once, it will receive all relevant
       notifications multiple times.</p>
    */
    public void addPropertyChangeListener(
        String propertyName, XPropertyChangeListener listener)
        throws UnknownPropertyException, WrappedTargetException
    {
        // assert listener != null;
        checkUnknown(propertyName);
        boolean disp;
        synchronized (this) {
            disp = disposed;
            if (!disp) {
                ArrayList<XPropertyChangeListener> v = boundListeners.get(propertyName);
                if (v == null) {
                    v = new ArrayList<XPropertyChangeListener>();
                    boundListeners.put(propertyName, v);
                }
                v.add(listener);
            }
        }
        if (disp) {
            listener.disposing(new EventObject(object));
        }
    }

    /**
       Implements <code>
       com.sun.star.beans.XPropertySet.removePropertyChangeListener</code>.
    */
    public void removePropertyChangeListener(
        String propertyName, XPropertyChangeListener listener)
        throws UnknownPropertyException, WrappedTargetException
    {
        // assert listener != null;
        checkUnknown(propertyName);
        synchronized (this) {
            if (boundListeners != null) {
                ArrayList<XPropertyChangeListener> v = boundListeners.get(propertyName);
                if (v != null) {
                    v.remove(listener);
                }
            }
        }
    }

    /**
       Implements
       <code>com.sun.star.beans.XPropertySet.addVetoableChangeListener</code>.

       <p>If a listener is added more than once, it will receive all relevant
       notifications multiple times.</p>
    */
    public void addVetoableChangeListener(
        String propertyName, XVetoableChangeListener listener)
        throws UnknownPropertyException, WrappedTargetException
    {
        // assert listener != null;
        checkUnknown(propertyName);
        boolean disp;
        synchronized (this) {
            disp = disposed;
            if (!disp) {
                ArrayList<XVetoableChangeListener> v = vetoListeners.get(propertyName);
                if (v == null) {
                    v = new ArrayList<XVetoableChangeListener>();
                    vetoListeners.put(propertyName, v);
                }
                v.add(listener);
            }
        }
        if (disp) {
            listener.disposing(new EventObject(object));
        }
    }

    /**
       Implements <code>
       com.sun.star.beans.XPropertySet.removeVetoableChangeListener</code>.
    */
    public void removeVetoableChangeListener(
        String propertyName, XVetoableChangeListener listener)
        throws UnknownPropertyException, WrappedTargetException
    {
        // assert listener != null;
        checkUnknown(propertyName);
        synchronized (this) {
            if (vetoListeners != null) {
                ArrayList<XVetoableChangeListener> v =  vetoListeners.get(propertyName);
                if (v != null) {
                    v.remove(listener);
                }
            }
        }
    }

    /**
       Implements
       <code>com.sun.star.beans.XFastPropertySet.setFastPropertyValue</code>.
    */
    public void setFastPropertyValue(int handle, Object value)
        throws UnknownPropertyException, PropertyVetoException,
        com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        setProperty(translateHandle(handle), value, false, false, (short) 1);
    }

    /**
       Implements
       <code>com.sun.star.beans.XFastPropertySet.getFastPropertyValue</code>.
    */
    public Object getFastPropertyValue(int handle)
        throws UnknownPropertyException, WrappedTargetException
    {
        return getProperty(translateHandle(handle), null);
    }

    /**
       Implements
       <code>com.sun.star.beans.XPropertyAccess.getPropertyValues</code>.
    */
    public PropertyValue[] getPropertyValues() {
        PropertyValue[] s = new PropertyValue[handleMap.length];
        int n = 0;
        for (int i = 0; i < handleMap.length; ++i) {
            PropertyState[] state = new PropertyState[1];
            Object value;
            try {
                value = getProperty(handleMap[i], state);
            } catch (UnknownPropertyException e) {
                continue;
            } catch (WrappedTargetException e) {
                throw new WrappedTargetRuntimeException(
                    e.getMessage(), object, e.TargetException);
            }
            s[n++] = new PropertyValue(handleMap[i], i, value, state[0]);
        }
        if (n < handleMap.length) {
            PropertyValue[] s2 = new PropertyValue[n];
            System.arraycopy(s, 0, s2, 0, n);
            s = s2;
        }
        return s;
    }

    /**
       Implements
       <code>com.sun.star.beans.XPropertyAccess.setPropertyValues</code>.
    */
    public void setPropertyValues(PropertyValue[] props)
        throws UnknownPropertyException, PropertyVetoException,
        com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        for (int i = 0; i < props.length; ++i) {
            if (props[i].Handle != -1
                && !props[i].Name.equals(translateHandle(props[i].Handle)))
            {
                throw new UnknownPropertyException(
                    ("name " + props[i].Name + " does not match handle "
                     + props[i].Handle),
                    object);
            }
            setProperty(
                props[i].Name, props[i].Value,
                props[i].State == PropertyState.AMBIGUOUS_VALUE,
                props[i].State == PropertyState.DEFAULT_VALUE, (short) 0);
        }
    }

    /**
       A class used by clients of {@link PropertySetMixin} when implementing UNO
       interface type attribute setter functions.

       @see #prepareSet(String, Object, Object, PropertySetMixin.BoundListeners)
    */
    public static final class BoundListeners {
        /**
           The constructor.
        */
        public BoundListeners() {}

        /**
           Notifies any
           <code>com.sun.star.beans.XPropertyChangeListener</code>s.

           @see #prepareSet(String, Object, Object,
           PropertySetMixin.BoundListeners)
        */
        public void notifyListeners() {
            if (specificListeners != null) {
                for (Iterator<XPropertyChangeListener> i = specificListeners.iterator(); i.hasNext();) {
                    try {
                        i.next().propertyChange(event);
                    } catch (DisposedException e) {}
                }
            }
            if (unspecificListeners != null) {
                for (Iterator<XPropertyChangeListener> i = unspecificListeners.iterator(); i.hasNext();)
                {
                    try {
                        i.next().propertyChange(event);
                    } catch (DisposedException e) {}
                }
            }
        }

        private ArrayList<XPropertyChangeListener> specificListeners = null;
        private ArrayList<XPropertyChangeListener> unspecificListeners = null;
        private PropertyChangeEvent event = null;
    }

    private XIdlClass getReflection(String typeName) {
        XIdlReflection refl;
        try {
            refl = UnoRuntime.queryInterface(
                XIdlReflection.class,
                context.getServiceManager().createInstanceWithContext(
                    "com.sun.star.reflection.CoreReflection", context));
        } catch (com.sun.star.uno.Exception e) {
            throw new DeploymentException(
                ("component context fails to supply service"
                 + " com.sun.star.reflection.CoreReflection: "
                 + e.getMessage()),
                context);
        }
        try {
            return refl.forName(typeName);
        } finally {
            XComponent comp = UnoRuntime.queryInterface(XComponent.class, refl);
            if (comp != null) {
                comp.dispose();
            }
        }
    }

    private void initProperties(
        XTypeDescription type, HashMap<String,PropertyData> map, ArrayList<String> handleNames, HashSet<String> seen)
    {
        XInterfaceTypeDescription2 ifc = UnoRuntime.queryInterface(
            XInterfaceTypeDescription2.class, resolveTypedefs(type));
        if (seen.add(ifc.getName())) {
            XTypeDescription[] bases = ifc.getBaseTypes();
            for (int i = 0; i < bases.length; ++i) {
                initProperties(bases[i], map, handleNames, seen);
            }
            XInterfaceMemberTypeDescription[] members = ifc.getMembers();
            for (int i = 0; i < members.length; ++i) {
                if (members[i].getTypeClass() == TypeClass.INTERFACE_ATTRIBUTE)
                {
                    XInterfaceAttributeTypeDescription2 attr =
                        UnoRuntime.queryInterface(
                            XInterfaceAttributeTypeDescription2.class,
                            members[i]);
                    short attrAttribs = 0;
                    if (attr.isBound()) {
                        attrAttribs |= PropertyAttribute.BOUND;
                    }
                    boolean setUnknown = false;
                    if (attr.isReadOnly()) {
                        attrAttribs |= PropertyAttribute.READONLY;
                        setUnknown = true;
                    }
                    XCompoundTypeDescription[] excs = attr.getGetExceptions();
                    boolean getUnknown = false;
                    //XXX  Special interpretation of getter/setter exceptions
                    // only works if the specified exceptions are of the exact
                    // type, not of a supertype:
                    for (int j = 0; j < excs.length; ++j) {
                        if (excs[j].getName().equals(
                                "com.sun.star.beans.UnknownPropertyException"))
                        {
                            getUnknown = true;
                            break;
                        }
                    }
                    excs = attr.getSetExceptions();
                    for (int j = 0; j < excs.length; ++j) {
                        if (excs[j].getName().equals(
                                "com.sun.star.beans.UnknownPropertyException"))
                        {
                            setUnknown = true;
                        } else if (excs[j].getName().equals(
                                       "com.sun.star.beans."
                                       + "PropertyVetoException"))
                        {
                            attrAttribs |= PropertyAttribute.CONSTRAINED;
                        }
                    }
                    if (getUnknown && setUnknown) {
                        attrAttribs |= PropertyAttribute.OPTIONAL;
                    }
                    XTypeDescription t = attr.getType();
                    for (;;) {
                        t = resolveTypedefs(t);
                        short n;
                        if (t.getName().startsWith(
                                "com.sun.star.beans.Ambiguous<"))
                        {
                            n = PropertyAttribute.MAYBEAMBIGUOUS;
                        } else if (t.getName().startsWith(
                                       "com.sun.star.beans.Defaulted<"))
                        {
                            n = PropertyAttribute.MAYBEDEFAULT;
                        } else if (t.getName().startsWith(
                                       "com.sun.star.beans.Optional<"))
                        {
                            n = PropertyAttribute.MAYBEVOID;
                        } else {
                            break;
                        }
                        attrAttribs |= n;
                        t = (UnoRuntime.queryInterface(
                                 XStructTypeDescription.class, t)).
                            getTypeArguments()[0];
                    }
                    String name = members[i].getMemberName();
                    boolean present = true;
                    if (absentOptional != null) {
                        for (int j = 0; j < absentOptional.length; ++j) {
                            if (name.equals(absentOptional[j])) {
                                present = false;
                                break;
                            }
                        }
                    }
                    if (map.put(
                            name,
                            new PropertyData(
                                new Property(
                                    name, handleNames.size(),
                                    new Type(t.getName(), t.getTypeClass()),
                                    attrAttribs),
                                present))
                        != null)
                    {
                        throw new RuntimeException(
                            "inconsistent UNO type registry");
                    }
                    handleNames.add(name);
                }
            }
        }
    }

    private String translateHandle(int handle) throws UnknownPropertyException {
        if (handle < 0 || handle >= handleMap.length) {
            throw new UnknownPropertyException("bad handle " + handle, object);
        }
        return handleMap[handle];
    }

    private void setProperty(
        String name, Object value, boolean isAmbiguous, boolean isDefaulted,
        short illegalArgumentPosition)
        throws UnknownPropertyException, PropertyVetoException,
        com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        PropertyData p = properties.get(name);
        if (p == null) {
            throw new UnknownPropertyException(name, object);
        }
        if ((isAmbiguous
             && (p.property.Attributes & PropertyAttribute.MAYBEAMBIGUOUS) == 0)
            || (isDefaulted
                && ((p.property.Attributes & PropertyAttribute.MAYBEDEFAULT)
                    == 0)))
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                ("flagging as ambiguous/defaulted non-ambiguous/defaulted"
                 + " property " + name),
                object, illegalArgumentPosition);

        }
        XIdlField2 f = UnoRuntime.queryInterface(
            XIdlField2.class, idlClass.getField(name));
        Object[] o = new Object[] {
                new Any(type, UnoRuntime.queryInterface(type, object)) };
        Object v = wrapValue(
            value,
            UnoRuntime.queryInterface(
                XIdlField2.class, idlClass.getField(name)).getType(),
            (p.property.Attributes & PropertyAttribute.MAYBEAMBIGUOUS) != 0,
            isAmbiguous,
            (p.property.Attributes & PropertyAttribute.MAYBEDEFAULT) != 0,
            isDefaulted,
            (p.property.Attributes & PropertyAttribute.MAYBEVOID) != 0);
        try {
            f.set(o, v);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            if (e.ArgumentPosition == 1) {
                throw new com.sun.star.lang.IllegalArgumentException(
                    e.getMessage(), object, illegalArgumentPosition);
            } else {
                throw new RuntimeException(
                    "unexpected com.sun.star.lang.IllegalArgumentException: "
                    + e.getMessage());
            }
        } catch (com.sun.star.lang.IllegalAccessException e) {
            //TODO  Clarify whether PropertyVetoException is the correct
            // exception to throw when trying to set a read-only property:
            throw new PropertyVetoException(
                "cannot set read-only property " + name, object);
        } catch (WrappedTargetRuntimeException e) {
            //FIXME  A WrappedTargetRuntimeException from XIdlField2.get is not
            // guaranteed to originate directly within XIdlField2.get (and thus
            // have the expected semantics); it might also be passed through
            // from lower layers.
            if (new Type(UnknownPropertyException.class).isSupertypeOf(
                    AnyConverter.getType(e.TargetException))
                && (p.property.Attributes & PropertyAttribute.OPTIONAL) != 0)
            {
                throw new UnknownPropertyException(name, object);
            } else if (new Type(PropertyVetoException.class).isSupertypeOf(
                           AnyConverter.getType(e.TargetException))
                       && ((p.property.Attributes
                            & PropertyAttribute.CONSTRAINED)
                           != 0))
            {
                throw new PropertyVetoException(name, object);
            } else {
                throw new WrappedTargetException(
                    e.getMessage(), object, e.TargetException);
            }
        }
    }

    Object getProperty(String name, PropertyState[] state)
        throws UnknownPropertyException, WrappedTargetException
    {
        PropertyData p = properties.get(name);
        if (p == null) {
            throw new UnknownPropertyException(name, object);
        }
        XIdlField2 field = UnoRuntime.queryInterface(
            XIdlField2.class, idlClass.getField(name));
        Object value;
        try {
            value = field.get(
                new Any(type, UnoRuntime.queryInterface(type, object)));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            throw new RuntimeException(
                "unexpected com.sun.star.lang.IllegalArgumentException: "
                + e.getMessage());
        } catch (WrappedTargetRuntimeException e) {
            //FIXME  A WrappedTargetRuntimeException from XIdlField2.get is not
            // guaranteed to originate directly within XIdlField2.get (and thus
            // have the expected semantics); it might also be passed through
            // from lower layers.
            if (new Type(UnknownPropertyException.class).isSupertypeOf(
                    AnyConverter.getType(e.TargetException))
                && (p.property.Attributes & PropertyAttribute.OPTIONAL) != 0)
            {
                throw new UnknownPropertyException(name, object);
            } else {
                throw new WrappedTargetException(
                    e.getMessage(), object, e.TargetException);
            }
        }
        boolean undoAmbiguous
            = (p.property.Attributes & PropertyAttribute.MAYBEAMBIGUOUS) != 0;
        boolean undoDefaulted
            = (p.property.Attributes & PropertyAttribute.MAYBEDEFAULT) != 0;
        boolean undoOptional
            = (p.property.Attributes & PropertyAttribute.MAYBEVOID) != 0;
        boolean isAmbiguous = false;
        boolean isDefaulted = false;
        while (undoAmbiguous || undoDefaulted || undoOptional) {
            String typeName = AnyConverter.getType(value).getTypeName();
            if (undoAmbiguous
                && typeName.startsWith("com.sun.star.beans.Ambiguous<"))
            {
                XIdlClass ambiguous = getReflection(typeName);
                try {
                    isAmbiguous = AnyConverter.toBoolean(
                        UnoRuntime.queryInterface(
                            XIdlField2.class,
                            ambiguous.getField("IsAmbiguous")).get(value));
                    value = UnoRuntime.queryInterface(
                        XIdlField2.class,
                        ambiguous.getField("Value")).get(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    throw new RuntimeException(
                        "unexpected"
                        + " com.sun.star.lang.IllegalArgumentException: "
                        + e.getMessage());
                }
                undoAmbiguous = false;
            } else if (undoDefaulted
                       && typeName.startsWith("com.sun.star.beans.Defaulted<"))
            {
                XIdlClass defaulted = getReflection(typeName);
                try {
                    isDefaulted = AnyConverter.toBoolean(
                        UnoRuntime.queryInterface(
                            XIdlField2.class,
                            defaulted.getField("IsDefaulted")).get(value));
                    value = UnoRuntime.queryInterface(
                        XIdlField2.class,
                        defaulted.getField("Value")).get(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    throw new RuntimeException(
                        "unexpected"
                        + " com.sun.star.lang.IllegalArgumentException: "
                        + e.getMessage());
                }
                undoDefaulted = false;
            } else if (undoOptional
                       && typeName.startsWith("com.sun.star.beans.Optional<"))
            {
                XIdlClass optional = getReflection(typeName);
                try {
                    boolean present = AnyConverter.toBoolean(
                        UnoRuntime.queryInterface(
                            XIdlField2.class,
                            optional.getField("IsPresent")).get(value));
                    if (!present) {
                        value = Any.VOID;
                        break;
                    }
                    value = UnoRuntime.queryInterface(
                        XIdlField2.class,
                        optional.getField("Value")).get(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                    throw new RuntimeException(
                        "unexpected"
                        + " com.sun.star.lang.IllegalArgumentException: "
                        + e.getMessage());
                }
                undoOptional = false;
            } else {
                throw new RuntimeException(
                    "unexpected type of attribute " + name);
            }
        }
        if (state != null) {
            //XXX  If isAmbiguous && isDefaulted, arbitrarily choose
            // AMBIGUOUS_VALUE over DEFAULT_VALUE:
            state[0] = isAmbiguous
                ? PropertyState.AMBIGUOUS_VALUE
                : isDefaulted
                ? PropertyState.DEFAULT_VALUE : PropertyState.DIRECT_VALUE;
        }
        return value;
    }

    private Object wrapValue(
        Object value, XIdlClass type, boolean wrapAmbiguous,
        boolean isAmbiguous, boolean wrapDefaulted, boolean isDefaulted,
        boolean wrapOptional)
    {
        // assert (wrapAmbiguous || !isAmbiguous)
        //     && (wrapDefaulted || !isDefaulted);
        if (wrapAmbiguous
            && type.getName().startsWith("com.sun.star.beans.Ambiguous<"))
        {
            Object[] strct = new Object[1];
            type.createObject(strct);
            try {
                XIdlField2 field = UnoRuntime.queryInterface(
                    XIdlField2.class, type.getField("Value"));
                field.set(
                    strct,
                    wrapValue(
                        value, field.getType(), false, false, wrapDefaulted,
                        isDefaulted, wrapOptional));
                UnoRuntime.queryInterface(
                    XIdlField2.class, type.getField("IsAmbiguous")).set(
                        strct, new Boolean(isAmbiguous));
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                throw new RuntimeException(
                    "unexpected com.sun.star.lang.IllegalArgumentException: "
                    + e.getMessage());
            } catch (com.sun.star.lang.IllegalAccessException e) {
                throw new RuntimeException(
                    "unexpected com.sun.star.lang.IllegalAccessException: "
                    + e.getMessage());
            }
            return strct[0];
        } else if (wrapDefaulted
                   && type.getName().startsWith(
                       "com.sun.star.beans.Defaulted<"))
        {
            Object[] strct = new Object[1];
            type.createObject(strct);
            try {
                XIdlField2 field = UnoRuntime.queryInterface(
                    XIdlField2.class, type.getField("Value"));
                field.set(
                    strct,
                    wrapValue(
                        value, field.getType(), wrapAmbiguous, isAmbiguous,
                        false, false, wrapOptional));
                UnoRuntime.queryInterface(
                    XIdlField2.class, type.getField("IsDefaulted")).set(
                        strct, new Boolean(isDefaulted));
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                throw new RuntimeException(
                    "unexpected com.sun.star.lang.IllegalArgumentException: "
                    + e.getMessage());
            } catch (com.sun.star.lang.IllegalAccessException e) {
                throw new RuntimeException(
                    "unexpected com.sun.star.lang.IllegalAccessException: "
                    + e.getMessage());
            }
            return strct[0];
        } else if (wrapOptional
                   && type.getName().startsWith("com.sun.star.beans.Optional<"))
        {
            Object[] strct = new Object[1];
            type.createObject(strct);
            boolean present = !AnyConverter.isVoid(value);
            try {
                UnoRuntime.queryInterface(
                    XIdlField2.class, type.getField("IsPresent")).set(
                        strct, new Boolean(present));
                if (present) {
                    XIdlField2 field = UnoRuntime.queryInterface(
                        XIdlField2.class, type.getField("Value"));
                    field.set(
                        strct,
                        wrapValue(
                            value, field.getType(), wrapAmbiguous, isAmbiguous,
                            wrapDefaulted, isDefaulted, false));
                }
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                throw new RuntimeException(
                    "unexpected com.sun.star.lang.IllegalArgumentException: "
                    + e.getMessage());
            } catch (com.sun.star.lang.IllegalAccessException e) {
                throw new RuntimeException(
                    "unexpected com.sun.star.lang.IllegalAccessException: "
                    + e.getMessage());
            }
            return strct[0];
        } else {
            if (wrapAmbiguous || wrapDefaulted || wrapOptional) {
                throw new RuntimeException("unexpected type of attribute");
            }
            return value;
        }
    }

    private static XTypeDescription resolveTypedefs(XTypeDescription type) {
        while (type.getTypeClass() == TypeClass.TYPEDEF) {
            type = UnoRuntime.queryInterface(
                XIndirectTypeDescription.class, type).getReferencedType();
        }
        return type;
    }

    private PropertyData get(Object object, String propertyName)
        throws UnknownPropertyException
    {
        PropertyData p = properties.get(propertyName);
        if (p == null || !p.present) {
            throw new UnknownPropertyException(propertyName, object);
        }
        return p;
    }

    private void checkUnknown(String propertyName)
        throws UnknownPropertyException
    {
        if (!propertyName.equals("")) {
            get(this, propertyName);
        }
    }

    private static final class PropertyData {
        public PropertyData(Property property, boolean present) {
            this.property = property;
            this.present = present;
        }

        public final Property property;
        public final boolean present;
    }

    private final class Info extends WeakBase implements XPropertySetInfo
    {
        public Info(Map<String,PropertyData> properties) {
            this.properties = properties;
        }

        public Property[] getProperties() {
            ArrayList<Property> al = new ArrayList<Property>(properties.size());
            for (Iterator<PropertyData> i = properties.values().iterator(); i.hasNext();) {
                PropertyData p = i.next();
                if (p.present) {
                    al.add(p.property);
                }
            }
            return al.toArray(new Property[al.size()]);
        }

        public Property getPropertyByName(String name)
            throws UnknownPropertyException
        {
            return get(this, name).property;
        }

        public boolean hasPropertyByName(String name) {
            PropertyData p = properties.get(name);
            return p != null && p.present;
        }

        private final Map<String,PropertyData> properties;
    }

    private final XComponentContext context;
    private final XInterface object;
    private final Type type;
    private final String[] absentOptional;
    private final XIdlClass idlClass;
    private final Map<String,PropertyData> properties; // from String to Property
    private final String[] handleMap;

    private HashMap<String,ArrayList<XPropertyChangeListener>> boundListeners
                  = new HashMap<String,ArrayList<XPropertyChangeListener>>();
        // from String to Vector of XPropertyChangeListener
    private HashMap<String,ArrayList<XVetoableChangeListener>> vetoListeners
                  = new HashMap<String,ArrayList<XVetoableChangeListener>>();
        // from String to Vector of XVetoableChangeListener
    private boolean disposed = false;
}

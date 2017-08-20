/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package com.sun.star.sdbcx.comp.postgresql.sdbcx;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ContainerEvent;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XContainer;
import com.sun.star.container.XContainerListener;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.InterfaceContainer;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XColumnLocate;
import com.sun.star.sdbcx.XAppend;
import com.sun.star.sdbcx.XDataDescriptorFactory;
import com.sun.star.sdbcx.XDrop;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.sdbcx.comp.postgresql.comphelper.OEnumerationByIndex;
import com.sun.star.sdbcx.comp.postgresql.util.StandardSQLState;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.util.XRefreshListener;
import com.sun.star.util.XRefreshable;

public abstract class OContainer extends WeakBase implements
        XNameAccess, XIndexAccess, XEnumerationAccess,
        XContainer, XColumnLocate, XRefreshable, XDataDescriptorFactory,
        XAppend, XDrop, XServiceInfo {

    private static String[] services = new String[] {
            "com.sun.star.sdbcx.Container"
    };

    protected final Object lock;
    private final boolean isCaseSensitive;
    private TreeMap<String,HashMap<Long,XPropertySet>> entriesByNameAndId;
    private ArrayList<PropertyInfo> entriesByIndex;
    private long nextId;
    private InterfaceContainer containerListeners = new InterfaceContainer();
    private InterfaceContainer refreshListeners = new InterfaceContainer();

    /// Names aren't necessarily unique, we have to de-duplicate by id.
    private static class PropertyInfo {
        String name;
        long id;

        PropertyInfo(String name, long id) {
            this.name = name;
            this.id = id;
        }
    }

    private Comparator<String> caseSensitiveComparator = new Comparator<String>() {
        @Override
        public int compare(String x, String y) {
            if (isCaseSensitive) {
                return x.compareTo(y);
            } else {
                return x.compareToIgnoreCase(y);
            }
        }
    };

    public OContainer(Object lock, boolean isCaseSensitive, List<String> names) {
        this.lock = lock;
        this.isCaseSensitive = isCaseSensitive;
        this.entriesByNameAndId = new TreeMap<String,HashMap<Long,XPropertySet>>(caseSensitiveComparator);
        this.entriesByIndex = new ArrayList<>(names.size());
        for (String name : names) {
            HashMap<Long,XPropertySet> entriesById = entriesByNameAndId.get(name);
            if (entriesById == null) {
                entriesById = new HashMap<>();
                entriesByNameAndId.put(name, entriesById);
            }
            entriesById.put(nextId, null);

            entriesByIndex.add(new PropertyInfo(name, nextId));
            ++nextId;
        }
    }

    // Would be from XComponent ;)

    public void dispose() {
        EventObject event = new EventObject(this);
        containerListeners.disposeAndClear(event);
        refreshListeners.disposeAndClear(event);

        synchronized (lock) {
            for (Map<Long,XPropertySet> entriesById : entriesByNameAndId.values()) {
                for (XPropertySet propertySet : entriesById.values()) {
                    CompHelper.disposeComponent(propertySet);
                }
            }
            entriesByNameAndId.clear();
            entriesByIndex.clear();
        }
    }

    // XServiceInfo

    public String getImplementationName() {
        return "com.sun.star.sdbcx.VContainer";
    }

    @Override
    public String[] getSupportedServiceNames() {
        return services.clone();
    }

    @Override
    public boolean supportsService(String serviceName) {
        for (String service : services) {
            if (service.equals(serviceName)) {
                return true;
            }
        }
        return false;
    }

    // XIndexAccess

    @Override
    public Object getByIndex(int index) throws IndexOutOfBoundsException, WrappedTargetException {
        synchronized (lock) {
            if (index < 0 || index >= entriesByIndex.size()) {
                throw new IndexOutOfBoundsException(Integer.toString(index), this);
            }
            return getObject(index);
        }
    }

    @Override
    public int getCount() {
        synchronized (lock) {
            return entriesByIndex.size();
        }
    }

    // XNameAccess

    @Override
    public boolean hasByName(String name) {
        synchronized (lock) {
            return entriesByNameAndId.containsKey(name);
        }
    }

    @Override
    public Object getByName(String name) throws NoSuchElementException, WrappedTargetException {
        synchronized (lock) {
            if (!entriesByNameAndId.containsKey(name)) {
                String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                        Resources.STR_NO_ELEMENT_NAME, "$name$", name);
                throw new NoSuchElementException(error, this);
            }
            return getObject(indexOf(name));
        }
    }

    @Override
    public String[] getElementNames() {
        synchronized (lock) {
            String[] names = new String[entriesByIndex.size()];
            int next = 0;
            for (PropertyInfo propertyInfo : entriesByIndex) {
                names[next++] = propertyInfo.name;
            }
            return names;
        }
    }

    // XRefreshable

    @Override
    public void refresh() {
        Iterator iterator;
        synchronized (lock) {
            for (Map<Long,XPropertySet> entriesById : entriesByNameAndId.values()) {
                for (XPropertySet propertySet : entriesById.values()) {
                    CompHelper.disposeComponent(propertySet);
                }
            }
            entriesByNameAndId.clear();
            entriesByIndex.clear();

            impl_refresh();

            iterator = refreshListeners.iterator();
        }
        EventObject event = new EventObject(this);
        while (iterator.hasNext()) {
            XRefreshListener listener = (XRefreshListener) iterator.next();
            listener.refreshed(event);
        }
    }

    // XDataDescriptorFactory

    @Override
    public XPropertySet createDataDescriptor() {
        synchronized (lock) {
            return createDescriptor();
        }
    }

    protected String getNameForObject(XPropertySet object) {
        try {
            Object ret = object.getPropertyValue("Name");
            return AnyConverter.toString(ret);
        } catch (WrappedTargetException wrappedTargetException) {
        } catch (UnknownPropertyException unknownPropertyException) {
        } catch (IllegalArgumentException illegalArgumentException) {
        }
        return null;
    }

    // XAppend

    @Override
    public void appendByDescriptor(XPropertySet descriptor) throws SQLException, ElementExistException {
        Iterator iterator;
        ContainerEvent event;
        synchronized (lock) {
            String name = getNameForObject(descriptor);

            if (entriesByNameAndId.containsKey(name)) {
                throw new ElementExistException(name, this);
            }

            XPropertySet newlyCreated = appendObject(name, descriptor);
            if (newlyCreated == null) {
                throw new RuntimeException();
            }

            name = getNameForObject(newlyCreated);
            HashMap<Long,XPropertySet> entriesById = entriesByNameAndId.get(name);
            if (entriesById == null) { // this may happen when the derived class included it itself
                entriesById = new HashMap<>();
                entriesById.put(nextId, newlyCreated);
                entriesByNameAndId.put(name, entriesById);
                entriesByIndex.add(new PropertyInfo(name, nextId));
                nextId++;
            }

            // notify our container listeners
            event = new ContainerEvent(this, name, newlyCreated, null);
            iterator = containerListeners.iterator();
        }
        while (iterator.hasNext()) {
            XContainerListener listener = (XContainerListener) iterator.next();
            listener.elementInserted(event);
        }

    }

    // XDrop

    @Override
    public void dropByName(String name) throws SQLException, NoSuchElementException {
        synchronized (lock) {
            if (!entriesByNameAndId.containsKey(name)) {
                throw new NoSuchElementException(name, this);
            }
            dropImpl(indexOf(name));
        }
    }

    @Override
    public void dropByIndex(int index) throws SQLException, IndexOutOfBoundsException {
        synchronized (lock) {
            if (index < 0 || index >= entriesByIndex.size()) {
                throw new IndexOutOfBoundsException(Integer.toString(index), this);
            }
            dropImpl(index);
        }
    }


    private void dropImpl(int index) throws SQLException {
        dropImpl(index, true);
    }

    private void dropImpl(int index, boolean reallyDrop) throws SQLException {
        PropertyInfo propertyInfo = entriesByIndex.get(index);
        if (reallyDrop) {
            dropObject(index, propertyInfo.name);
        }
        HashMap<Long,XPropertySet> entriesById = entriesByNameAndId.get(propertyInfo.name);
        XPropertySet propertySet = entriesById.remove(propertyInfo.id);
        if (entriesById.isEmpty()) {
            entriesByNameAndId.remove(propertyInfo.name);
        }
        CompHelper.disposeComponent(propertySet);
        entriesByIndex.remove(index);

        ContainerEvent event = new ContainerEvent(this, propertyInfo.name, null, null);
        for (Iterator iterator = containerListeners.iterator(); iterator.hasNext(); ) {
            XContainerListener listener = (XContainerListener) iterator.next();
            listener.elementRemoved(event);
        }
    }

    // XColumnLocate

    @Override
    public int findColumn(String name) throws SQLException {
        if (!entriesByNameAndId.containsKey(name)) {
            String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                    Resources.STR_UNKNOWN_COLUMN_NAME, "$columnname$", name);
            throw new SQLException(error, this, StandardSQLState.SQL_COLUMN_NOT_FOUND.text(), 0, null);
        }
        return indexOf(name) + 1; // because columns start at one
    }


    // XEnumerationAccess

    @Override
    public XEnumeration createEnumeration() {
        return new OEnumerationByIndex(this);
    }

    @Override
    public void addContainerListener(XContainerListener listener) {
        containerListeners.add(listener);
    }

    @Override
    public void removeContainerListener(XContainerListener listener) {
        containerListeners.remove(listener);
    }

    @Override
    public Type getElementType() {
        return new Type(XPropertySet.class);
    }

    @Override
    public boolean hasElements() {
        synchronized (lock) {
            return !entriesByNameAndId.isEmpty();
        }
    }

    @Override
    public void addRefreshListener(XRefreshListener listener) {
        synchronized (lock) {
            refreshListeners.add(listener);
        }
    }

    @Override
    public void removeRefreshListener(XRefreshListener listener) {
        synchronized (lock) {
            refreshListeners.remove(listener);
        }
    }

    protected int indexOf(String name) {
        for (int i = 0; i < entriesByIndex.size(); i++) {
            if (entriesByIndex.get(i).name.equals(name)) {
                return i;
            }
        }
        return -1;
    }

    protected Object getObject(int index) throws WrappedTargetException {
        PropertyInfo propertyInfo = entriesByIndex.get(index);
        HashMap<Long,XPropertySet> entriesById = entriesByNameAndId.get(propertyInfo.name);
        XPropertySet propertySet = entriesById.get(propertyInfo.id);
        if (propertySet == null) {
            try {
                propertySet = createObject(propertyInfo.name);
            } catch (SQLException e) {
                try {
                    dropImpl(index, false);
                } catch (Exception ignored) {
                }
                throw new WrappedTargetException(e.getMessage(), this, e);
            }
            entriesById.put(propertyInfo.id, propertySet);
        }
        return propertySet;
    }

    protected XPropertySet cloneDescriptor(XPropertySet descriptor) {
        XPropertySet newDescriptor = createDescriptor();
        CompHelper.copyProperties(descriptor, newDescriptor);
        return newDescriptor;
    }

    protected boolean isCaseSensitive() {
        return isCaseSensitive;
    }

    // will be called when a object was requested by one of the accessing methods like getByIndex
    public abstract XPropertySet createObject(final String name) throws SQLException;

    public abstract void dropObject(int index, String name) throws SQLException;

    // the implementing class should refresh their elements
    public abstract void impl_refresh();

    // will be called when a new object should be generated by a call of createDataDescriptor
    // the returned object is empty will be filled outside and added to the collection
    public abstract XPropertySet createDescriptor();

    /** appends an object described by a descriptor, under a given name
        @param _rForName
            is the name under which the object should be appended. Guaranteed to not be empty.
            This is passed for convenience only, since it's the result of a call of
            getNameForObject for the given descriptor
        @param descriptor
            describes the object to append
        @return
            the new object which is to be inserted into the collection. This might be the result
            of a call of <code>createObject( _rForName )</code>, or a clone of the descriptor.
    */
    public abstract XPropertySet appendObject(String _rForName, XPropertySet descriptor) throws SQLException;

}

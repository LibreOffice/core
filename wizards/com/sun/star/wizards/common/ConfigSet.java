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
package com.sun.star.wizards.common;

import java.util.*;

import javax.swing.ListModel;
import javax.swing.event.ListDataEvent;


import org.w3c.dom.*;

public class ConfigSet implements ConfigNode, XMLProvider, ListModel
{

    private Class<?> childClass;
    private Map<String, Object> childrenMap = new HashMap<String, Object>();
    private List<Object> childrenList = new ArrayList<Object>();
    public Object root;
    /**
     * After reading the configuration set items,
     * the ConfigSet checks this field.
     * If it is true, it will remove any nulls from
     * the vector.
     * subclasses can change this field in the constructor
     * to avoid this "deletion" of nulls.
     */
    protected boolean noNulls = true;
    /** Utility field used by event firing mechanism. */
    private javax.swing.event.EventListenerList listenerList = null;

    public ConfigSet(Class<?> childType)
    {
        childClass = childType;
    }

    public void add(String name, Object child)
    {
        childrenMap.put(name, child);
        try
        {
            int i = ((Indexable) child).getIndex();
            int oldSize = getSize();
            while (getSize() <= i)
            {
                childrenList.add(null);
            }
            childrenList.set(i, child);
            if (oldSize > i)
            {
                oldSize = i;
            }
            fireListDataListenerIntervalAdded(oldSize, i);
        }
        catch (ClassCastException cce)
        {
            childrenList.add(child);
            fireListDataListenerIntervalAdded(getSize() - 1, getSize() - 1);
        }
    }

    public void add(int i, Object o)
    {
        int name = i;
        while (getElement(PropertyNames.EMPTY_STRING + name) != null)
        {
            name++;
        }
        childrenMap.put(PropertyNames.EMPTY_STRING + name, o);
        childrenList.add(i, o);

        fireListDataListenerIntervalAdded(i, i);
    }

    protected Object createChild() throws InstantiationException, IllegalAccessException
    {
        return childClass.newInstance();
    }

    public void writeConfiguration(Object configView, Object param)
    {
        Object[] names = childrenMap.keySet().toArray();

        if (ConfigNode.class.isAssignableFrom(childClass))
        {
            //first I remove all the children from the configuration.
            String children[] = Configuration.getChildrenNames(configView);
            for (int i = 0; i < children.length; i++)
            {
                try
                {
                    Configuration.removeNode(configView, children[i]);
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }            // and add them new.
            }
            for (int i = 0; i < names.length; i++)
            {
                try
                {
                    ConfigNode child = (ConfigNode) getElement(names[i]);
                    Object childView = Configuration.addConfigNode(configView, (String) names[i]);
                    child.writeConfiguration(childView, param);
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }
            }
        }
        //for a set of primitive / String type.
        else
        {
            throw new IllegalArgumentException("Unable to write primitive sets to configuration (not implemented)");
        }
    }

    public void readConfiguration(Object configurationView, Object param)
    {
        String[] names = Configuration.getChildrenNames(configurationView);
        System.out.println(names.length);
        if (ConfigNode.class.isAssignableFrom(childClass))
        {

            for (int i = 0; i < names.length; i++)
            {
                try
                {
                    ConfigNode child = (ConfigNode) createChild();
                    child.setRoot(root);
                    child.readConfiguration(Configuration.getNode(names[i], configurationView), param);
                    add(names[i], child);
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }
            }
            //remove any nulls from the list
            if (noNulls)
            {
                for (int i = 0; i < childrenList.size(); i++)
                {
                    if (childrenList.get(i) == null)
                    {
                        childrenList.remove(i--);
                    }
                }
            }
        }
        //for a set of primitive / String type.
        else
        {
            for (int i = 0; i < names.length; i++)
            {
                try
                {
                    Object child = Configuration.getNode(names[i], configurationView);
                    add(names[i], child);
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }
            }
        }
    }

    public void remove(Object obj)
    {
        Object key = getKey(obj);
        childrenMap.remove(key);
        int i = childrenList.indexOf(obj);
        childrenList.remove(obj);
        fireListDataListenerIntervalRemoved(i, i);
    }

    public void remove(int i)
    {
        Object o = getElementAt(i);
        remove(o);
    }

    public void clear()
    {
        childrenMap.clear();
        childrenList.clear();
    }

    public void update(int i)
    {
        fireListDataListenerContentsChanged(i, i);
    }

    public Node createDOM(Node parent)
    {

        Object[] items = items();

        for (int i = 0; i < items.length; i++)
        {
            Object item = items[i];
            if (item instanceof XMLProvider)
            {
                ((XMLProvider) item).createDOM(parent);
            }
        }
        return parent;
    }

    public Object[] items()
    {
        return childrenList.toArray();
    }

    public Object getKey(Object object)
    {
        for (Iterator<Map.Entry<String,Object>> i = childrenMap.entrySet().iterator(); i.hasNext();)
        {

            Map.Entry<String,Object> me = i.next();
            if (me.getValue() == object)
            {
                return me.getKey();
            }
        }
        return null;
    }

    public Object getKey(int i)
    {
        int c = 0;
        while (i > -1)
        {
            if (getElementAt(c) != null)
            {
                i--;
            }
            c++;
        }
        if (c == 0)
        {
            return null;
        }
        else
        {
            return getKey(getElementAt(c - 1));
        }
    }

    public void setRoot(Object newRoot)
    {
        root = newRoot;
    }

    /** Registers ListDataListener to receive events.
     * @param listener The listener to register.
     *
     */
    public synchronized void addListDataListener(javax.swing.event.ListDataListener listener)
    {
        if (listenerList == null)
        {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add(javax.swing.event.ListDataListener.class, listener);
    }

    /** Removes ListDataListener from the list of listeners.
     * @param listener The listener to remove.
     *
     */
    public synchronized void removeListDataListener(javax.swing.event.ListDataListener listener)
    {
        listenerList.remove(javax.swing.event.ListDataListener.class, listener);
    }

    /** Notifies all registered listeners about the event.
     *
     */
    private void fireListDataListenerIntervalAdded(int i0, int i1)
    {
        ListDataEvent event = new ListDataEvent(this, ListDataEvent.INTERVAL_ADDED, i0, i1);
        if (listenerList == null)
        {
            return;
        }
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i] == javax.swing.event.ListDataListener.class)
            {
                ((javax.swing.event.ListDataListener) listeners[i + 1]).intervalAdded(event);
            }
        }
    }

    /** Notifies all registered listeners about the event.
     *
     */
    private void fireListDataListenerIntervalRemoved(int i0, int i1)
    {
        ListDataEvent event = new ListDataEvent(this, ListDataEvent.INTERVAL_REMOVED, i0, i1);
        if (listenerList == null)
        {
            return;
        }
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i] == javax.swing.event.ListDataListener.class)
            {
                ((javax.swing.event.ListDataListener) listeners[i + 1]).intervalRemoved(event);
            }
        }
    }

    /** Notifies all registered listeners about the event.
     *
     */
    private void fireListDataListenerContentsChanged(int i0, int i1)
    {
        ListDataEvent event = new ListDataEvent(this, ListDataEvent.CONTENTS_CHANGED, i0, i1);
        if (listenerList == null)
        {
            return;
        }
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i] == javax.swing.event.ListDataListener.class)
            {
                ((javax.swing.event.ListDataListener) listeners[i + 1]).contentsChanged(event);
            }
        }
    }

    public Object getElementAt(int i)
    {
        return childrenList.get(i);
    }

    public Object getElement(Object o)
    {
        return childrenMap.get(o);
    }

    public int getSize()
    {
        return childrenList.size();
    }

    public Set<String> keys()
    {
        return childrenMap.keySet();
    }

    public int getIndexOf(Object item)
    {
        return childrenList.indexOf(item);
    }

    /**
     * Set members might include a property
     * which orders them.
     * This method reindexes the given member to be 
     * the index number 0
     * Do not forget to call commit() after calling this method.
     * @param confView
     * @param memberName
     */
    public void reindexSet(Object confView, String memberName, String indexPropertyName) throws Exception
    {
        /*
         * First I read all memebrs of the set,
         * except the one that should be number 0
         * to a vector, ordered by there index property
         */
        String[] names = Configuration.getChildrenNames(confView);
        ArrayList<Object> v = new ArrayList<Object>(names.length);
        Object member = null;
        int index = 0;
        for (int i = 0; i < names.length; i++)
        {
            if (!names[i].equals(memberName))
            {
                member = Configuration.getConfigurationNode(names[i], confView);
                index = Configuration.getInt(indexPropertyName, member);
                while (index >= v.size())
                {
                    v.add(null);
                }
                v.set(index, member);

            }
        /**
         * Now I reindex them
         */
        }
        index = 1;
        for (int i = 0; i < v.size(); i++)
        {
            member = v.get(i);
            if (member != null)
            {
                Configuration.set(index++, indexPropertyName, member);
            }
        }

    }

    public void sort(Comparator<Object> comparator)
    {
        Collections.sort(this.childrenList, comparator);
    }
}

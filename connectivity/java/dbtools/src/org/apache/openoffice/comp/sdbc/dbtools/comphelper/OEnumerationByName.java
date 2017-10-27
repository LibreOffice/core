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

package com.sun.star.sdbcx.comp.postgresql.comphelper;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;

public class OEnumerationByName extends WeakBase implements XEnumeration, XEventListener {
    private String[] names;
    private int position;
    private XNameAccess collection;
    private boolean isListening;

    public OEnumerationByName(XNameAccess collection) {
        this.collection = collection;
        names = collection.getElementNames();
        startDisposeListening();
    }

    @Override
    public void disposing(EventObject event) {
        synchronized (this) {
            if (event.Source == collection) {
                collection = null;
            }
        }
    }

    @Override
    public boolean hasMoreElements() {
        synchronized (this) {
            if (collection != null) {
                if (position < names.length) {
                    return true;
                } else {
                    stopDisposeListening();
                    collection = null;
                }
            }
            return false;
        }
    }

    @Override
    public Object nextElement()
            throws NoSuchElementException, WrappedTargetException {
        Object value = null;
        synchronized (this) {
            if (collection != null) {
                if (position < names.length) {
                    value = collection.getByName(names[position++]);
                }
                if (position >= names.length) {
                    stopDisposeListening();
                    collection = null;
                }
            }
        }
        if (value == null) {
            throw new NoSuchElementException();
        }
        return value;
    }

    private void startDisposeListening() {
        synchronized (this) {
            if (isListening) {
                return;
            }
            XComponent component = UnoRuntime.queryInterface(XComponent.class, collection);
            if (component != null) {
                component.addEventListener(this);
                isListening = true;
            }
        }
    }

    private void stopDisposeListening() {
        synchronized (this) {
            if (!isListening) {
                return;
            }
            XComponent component = UnoRuntime.queryInterface(XComponent.class, collection);
            if (component != null) {
                component.removeEventListener(this);
                isListening = false;
            }
        }
    }
}

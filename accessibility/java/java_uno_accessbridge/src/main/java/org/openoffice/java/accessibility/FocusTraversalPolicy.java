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



package org.openoffice.java.accessibility;


public class FocusTraversalPolicy extends java.awt.FocusTraversalPolicy {

    protected javax.accessibility.Accessible getSelectedAccessibleChild(javax.accessibility.Accessible a) {
        javax.accessibility.AccessibleContext ac = a.getAccessibleContext();
        if (ac != null) {
            javax.accessibility.AccessibleSelection as = ac.getAccessibleSelection();
            if (as != null) {
                return as.getAccessibleSelection(0);
            }
        }
        return null;
    }

    /** Returns the Component that should receive the focus after aComponent */
    public java.awt.Component getComponentAfter(java.awt.Container focusCycleRoot,
        java.awt.Component aComponent) {
        return null;
    }

    /** Returns the Component that should receive the focus before aComponent */
    public java.awt.Component getComponentBefore(java.awt.Container focusCycleRoot,
        java.awt.Component aComponent) {
        return null;
    }

    /** Returns the default Component to focus */
    public java.awt.Component getDefaultComponent(java.awt.Container focusCycleRoot) {
        // getDefaultComponent must not return null for Windows to make them focusable.
        if (focusCycleRoot instanceof NativeFrame) {
            java.awt.Component c = ((NativeFrame) focusCycleRoot).getInitialComponent();
            if (c != null) {
                return c;
            }
        }

        if (focusCycleRoot instanceof javax.accessibility.Accessible) {
            return (java.awt.Component) getSelectedAccessibleChild((javax.accessibility.Accessible) focusCycleRoot);
        }
        return null;
    }

    /** Returns the first Component in the traversal cycle */
    public java.awt.Component getFirstComponent(java.awt.Container focusCycleRoot) {
        return null;
    }

    /** Returns the Component that should receive the focus when a Window is made visible for the first time */
    public java.awt.Component getInitialComponent(java.awt.Window window) {
        if (window instanceof NativeFrame) {
            return ((NativeFrame) window).getInitialComponent();
        }
        return null;
    }

    /** Returns the last Component in the traversal cycle */
    public java.awt.Component getLastComponent(java.awt.Container focusCycleRoot) {
        return null;
    }
}

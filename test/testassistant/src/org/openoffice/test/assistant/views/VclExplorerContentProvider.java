/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.assistant.views;

import java.util.ArrayList;

import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.Viewer;

public class VclExplorerContentProvider implements IStructuredContentProvider {
    public VclExplorerContentProvider() {

    }

    public void inputChanged(Viewer v, Object oldInput, Object newInput) {
        ArrayList<ControlInfo> oldControlInfos = (ArrayList<ControlInfo>) oldInput;
        if (oldControlInfos != null) {
            for (ControlInfo info : oldControlInfos) {
                info.dispose();
            }
        }
    }

    public void dispose() {

    }

    public Object[] getElements(Object parent) {
        ArrayList<ControlInfo> controlInfos = (ArrayList<ControlInfo>) parent;
        if (controlInfos != null)
            return controlInfos.toArray();

        return new Object[0];
    }
}
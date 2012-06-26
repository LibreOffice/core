/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.vclauto.assistant.views;

import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.swt.graphics.Image;

public class VclExplorerLabelProvider extends LabelProvider implements ITableLabelProvider {
    public VclExplorerLabelProvider() {

    }

    public String getColumnText(Object obj, int index) {
        ControlInfo info = (ControlInfo) obj;
        switch (index) {
        case 0:
            return info.id.toString();
        case 1:
            return info.name;
        case 2:
            return Long.toString(info.type) + " " + info.getVclType();
        case 3:
            return info.tip;
        }
        return "";
    }

    public Image getColumnImage(Object obj, int index) {
        return null;
    }
}
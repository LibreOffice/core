/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.assistant.views;


import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.util.HashMap;

import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Rectangle;
import org.openoffice.test.vcl.client.SmartId;
import org.openoffice.test.vcl.widgets.VclControl;

public class ControlInfo {

    private static final HashMap<Long, String> vcltypes = new  HashMap<Long, String>();

    static {
        Field[] fields = VclControl.class.getDeclaredFields();
        for (Field field : fields) {
            int modifiers = field.getModifiers();
            if (modifiers == (Modifier.PUBLIC | Modifier.STATIC | Modifier.FINAL) && field.getType() == Integer.TYPE) {
                try {
                    int v = (Integer) field.get(null);
                    vcltypes.put((long)v, field.getName());
                } catch (Exception e) {
                    //ignore
                }
            }
        }
    }

    public SmartId id;

    public long type;

    public String tip;

    public Image appearance;

    public Rectangle rectangle;

    public String name;

    public ControlInfo(SmartId id, long type, String tip) {
        super();
        this.id = id;
        this.type = type;
        this.tip = tip;
    }

    public String getVclType() {
        return vcltypes.get(type);
    }

    public void shot() {
        if (id.getId() != 0 || id.getSid() != null) {
            VclControl vclControl = new VclControl(id);
            try {
                if (!vclControl.exists())
                    return;
                java.awt.Rectangle rect = vclControl.getScreenRectangle();
//              Display display = Display.getDefault();
//              GC gc = new GC(display);
                rectangle = new Rectangle(rect.x, rect.y, rect.width, rect.height);
//              appearance = new Image(display, rectangle);
//              gc.copyArea(appearance, 0, 0);
//              gc.dispose();
            } catch (Throwable t) {

            }

        }
    }

    public void dispose() {
        if (appearance != null)
            appearance.dispose();
    }
}

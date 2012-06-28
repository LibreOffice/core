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

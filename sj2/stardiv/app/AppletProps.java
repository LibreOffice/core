/*************************************************************************
 *
 *  $RCSfile: AppletProps.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:54:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package stardiv.app;

import stardiv.app.AppletMessageHandler;
import stardiv.applet.AppletExecutionContext;

import java.awt.*;
import java.io.*;
import java.util.Properties;
import sun.net.www.http.HttpClient;
import sun.net.ftp.FtpClient;

public class AppletProps extends Frame {
    TextField proxyHost;
    TextField proxyPort;
    Choice networkMode;
    Choice accessMode;
    Choice unsignedMode;

    AppletExecutionContext appletExecutionContext;

    AppletProps(AppletExecutionContext appletExecutionContext) {
        this.appletExecutionContext = appletExecutionContext;

        setTitle(amh.getMessage("title"));
        Panel p = new Panel();
        p.setLayout(new GridLayout(0, 2));

        p.add(new Label(amh.getMessage("label.http.server", "Http proxy server:")));
        p.add(proxyHost = new TextField());

        p.add(new Label(amh.getMessage("label.http.proxy")));
        p.add(proxyPort = new TextField());

        p.add(new Label(amh.getMessage("label.network")));
        p.add(networkMode = new Choice());
        networkMode.addItem(amh.getMessage("choice.network.item.none"));
        networkMode.addItem(amh.getMessage("choice.network.item.applethost"));
        networkMode.addItem(amh.getMessage("choice.network.item.unrestricted"));

        String securityMode = System.getProperty("appletviewer.security.mode");
        securityMode = (securityMode == null) ? "none" : securityMode;
        securityMode = securityMode.equals("host") ? "applethost" : securityMode;
        networkMode.select(amh.getMessage("choice.network.item." + securityMode));

        p.add(new Label(amh.getMessage("label.class")));
        p.add(accessMode = new Choice());
        accessMode.addItem(amh.getMessage("choice.class.item.restricted"));
        accessMode.addItem(amh.getMessage("choice.class.item.unrestricted"));

        accessMode.select(Boolean.getBoolean("package.restrict.access.sun")
                          ? amh.getMessage("choice.class.item.restricted")
                          : amh.getMessage("choice.class.item.unrestricted"));

        p.add(new Label(amh.getMessage("label.unsignedapplet")));
        p.add(unsignedMode = new Choice());
        unsignedMode.addItem(amh.getMessage("choice.unsignedapplet.no"));
        unsignedMode.addItem(amh.getMessage("choice.unsignedapplet.yes"));

        add("Center", p);
        p = new Panel();
        p.add(new Button(amh.getMessage("button.apply")));
        p.add(new Button(amh.getMessage("button.reset")));
        p.add(new Button(amh.getMessage("button.cancel")));
        add("South", p);
        setLocation(200, 150);
        pack();
        reset();
    }

    void reset() {
        //      if (Boolean.getBoolean("package.restrict.access.sun")) {
        //              accessMode.select(amh.getMessage("choice.class.item.restricted"));
        //      } else {
        //              accessMode.select(amh.getMessage("choice.class.item.unrestricted"));
        //      }

        if (System.getProperty("http.proxyHost") != null) {
            proxyHost.setText(System.getProperty("http.proxyHost"));
            proxyPort.setText(System.getProperty("http.proxyPort"));
            HttpClient.proxyPort = Integer.valueOf(System.getProperty("http.proxyPort")).intValue();
        }
        else {
            proxyHost.setText("");
            proxyPort.setText("");
        }

        //      if (Boolean.getBoolean("appletviewer.security.allowUnsigned")) {
        //              unsignedMode.select(amh.getMessage("choice.unsignedapplet.yes"));
        //      } else {
        //              unsignedMode.select(amh.getMessage("choice.unsignedapplet.no"));
        //      }
    }

    void apply() {
        // Get properties, set version
        Properties props = System.getProperties();
        if (proxyHost.getText().length() > 0) {
            props.put("http.proxyHost", proxyHost.getText().trim());
            props.put("http.proxyPort", proxyPort.getText().trim());
        } else {
            props.remove("http.proxyHost");
        }
        if ("None".equals(networkMode.getSelectedItem())) {
            props.put("appletviewer.security.mode", "none");
        } else if ("Unrestricted".equals(networkMode.getSelectedItem())) {
            props.put("appletviewer.security.mode", "unrestricted");
        } else {
            props.put("appletviewer.security.mode", "host");
        }

        if ("Restricted".equals(accessMode.getSelectedItem())) {
            props.put("package.restrict.access.sun", "true");
            props.put("package.restrict.access.netscape", "true");
            props.put("package.restrict.access.stardiv", "true");
        } else {
            props.put("package.restrict.access.sun", "false");
            props.put("package.restrict.access.netscape", "false");
            props.put("package.restrict.access.stardiv", "false");
        }

        if ("Yes".equals(unsignedMode.getSelectedItem())) {
            props.put("appletviewer.security.allowUnsigned", "true");
        } else {
            props.put("appletviewer.security.allowUnsigned", "false");
        }

        // Save properties
        try {
            reset();

            FileOutputStream out = new FileOutputStream(AppletViewer.theUserPropertiesFile);
            props.save(out, "AppletViewer");
            out.close();
            setVisible( false );
        } catch (IOException e) {
            System.out.println(amh.getMessage("apply.exception", e));
            e.printStackTrace();
            reset();
        }
    }

    public boolean action(Event evt, Object obj) {
        if (amh.getMessage("button.apply").equals(obj)) {
            apply();
            return true;
        }
        if (amh.getMessage("button.reset").equals(obj)) {
            reset();
            return true;
        }
        if (amh.getMessage("button.cancel").equals(obj)) {
            setVisible( false );
            return true;
        }
        return false;
    }

    private static AppletMessageHandler amh = new AppletMessageHandler("appletprops");

}

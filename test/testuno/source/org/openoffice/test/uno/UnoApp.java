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

package org.openoffice.test.uno;

import java.io.File;
import java.util.Timer;
import java.util.TimerTask;

import org.openoffice.test.OpenOffice;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.SystemUtil;

import com.sun.star.beans.PropertyValue;
import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XModifiable;

public class UnoApp {

    private OpenOffice openOffice = null;

    private String unoUrl = null;

    private XComponentContext componentContext = null;

    private XMultiComponentFactory componentFactory = null;

    private XMultiServiceFactory serviceFactory = null;

    private XDesktop desktop = null;

    private double reconnectInterval = 2;

    private int reconnectCount = 10;

    public UnoApp() {
        this.openOffice = OpenOffice.getDefault();
    }

    public UnoApp(OpenOffice openOffice) {
        this.openOffice = openOffice;
    }

    public UnoApp(String unoUrl) {
        this.unoUrl = unoUrl;
    }

    /**
     * Start OpenOffice and connect to it
     */
    public void start() {
        if (openOffice != null) {
            openOffice.start();
            unoUrl = openOffice.getUnoUrl();
        }

        for (int i = 0; i < reconnectCount; i++) {
            try {
                XUnoUrlResolver resolver = UnoUrlResolver.create(Bootstrap.createInitialComponentContext(null));
                componentContext = UnoRuntime.queryInterface(XComponentContext.class, resolver.resolve("uno:" + unoUrl + ";StarOffice.ComponentContext"));
                componentFactory = componentContext.getServiceManager();
                serviceFactory = UnoRuntime.queryInterface(XMultiServiceFactory.class, componentFactory);
                desktop = UnoRuntime.queryInterface(XDesktop.class, serviceFactory.createInstance("com.sun.star.comp.framework.Desktop"));
                return;
            } catch (Exception e) {
                // e.printStackTrace(); // for debugging
            }

            SystemUtil.sleep(reconnectInterval);
        }

        throw new RuntimeException("Failed to connect to uno url: " + unoUrl);
    }

    private Timer timer = new Timer(true);

    private TimerTask timerTask = null;

    /**
     * Shut down the connection and close OpenOffice
     */
    public void close() {
        try {
            timerTask = new TimerTask() {
                public void run() {
                    if (openOffice != null)
                        openOffice.kill();
                }
            };
            timer.schedule(timerTask, 1000 * 2);
            desktop.terminate();
        } catch (Exception e) {
            // e.printStackTrace(); // for debugging
        } finally {
            if (openOffice != null)
                openOffice.kill();

            timerTask.cancel();
            timerTask = null;
            componentContext = null;
            componentFactory = null;
            serviceFactory = null;
            desktop = null;
        }
    }

    /**
     * Get the XComponentContext of the connected OpenOffice instance
     *
     * @return
     */
    public XComponentContext getComponentContext() {
        return componentContext;
    }

    public XMultiComponentFactory getComponentFactory() {
        return componentFactory;
    }

    public XMultiServiceFactory getServiceFactory() {
        return serviceFactory;
    }

    public XDesktop getDesktop() {
        return desktop;
    }

    public XComponent loadDocument(String file) throws Exception {
        XComponentLoader componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, desktop);
        return componentLoader.loadComponentFromURL(FileUtil.getUrl(file), "_blank", 0, new PropertyValue[0]);
    }


    public XComponent loadDocumentFromURL(String url) throws Exception {
        XComponentLoader componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, desktop);
        return componentLoader.loadComponentFromURL(url, "_blank", 0, new PropertyValue[0]);
    }

    public XComponent newDocument(String type) throws Exception {
        XComponentLoader componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, desktop);
        return componentLoader.loadComponentFromURL("private:factory/" + type, "_blank", 0, new PropertyValue[0]);
    }

    public void saveDocument(XComponent doc, String toPath) throws Exception {
        XStorable m_xstorable = (XStorable)UnoRuntime.queryInterface(XStorable.class, doc);
        String fileUrl = FileUtil.getUrl(new File(toPath));
        m_xstorable.storeAsURL(fileUrl, new PropertyValue[0]);
    }

    public void closeDocument(XComponent doc) {
        try {
            XModifiable modified = (XModifiable) UnoRuntime.queryInterface(XModifiable.class, doc);
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, doc);
            if (modified != null)
                modified.setModified(false);
            closer.close(true);
        } catch (Exception e) {
            // ignore
        }
    }
}

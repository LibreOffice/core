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


package com.sun.star.wizards.web;

import org.w3c.dom.Document;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.util.URL;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.UCB;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.CGLayout;
import com.sun.star.wizards.web.data.CGSettings;

/**
 * @author rpiterman
 * This class both copies necessary files to
 * a temporary directory, generates a temporary TOC page,
 * and opens the generated html document in a web browser,
 * by default "index.html" (unchangeable).
 * <br/>
 * Since the files are both static and dynamic (some are always the same,
 * while other change according to user choices)
 * I divide this tasks to two: all necessary
 * static files, which should not regularly update are copied upon
 * instantiation.
 * The TOC is generated in refresh(...);
 */
public class TOCPreview
{

    private String tempDir = null;
    private XMultiServiceFactory xmsf;
    private FileAccess fileAccess;
    private WebWizardDialogResources resources;
    private URL openHyperlink;
    private XDispatch xDispatch;
    private PropertyValue[] loadArgs;
    private UCB ucb;
    private XFrame xFrame;

    /**
     * @param xmsf_
     * @param settings web wizard settings
     * @param res resources
     * @param tempDir_ destination
     * @throws Exception
     */
    public TOCPreview(XMultiServiceFactory xmsf_, CGSettings settings, WebWizardDialogResources res, String tempDir_, XFrame _xFrame)
            throws Exception
    {
        xFrame = _xFrame;
        xmsf = xmsf_;
        resources = res;
        fileAccess = new FileAccess(xmsf);
        tempDir = tempDir_;
        loadArgs = loadArgs(FileAccess.connectURLs(tempDir, "/index.html"));
        openHyperlink = Desktop.getDispatchURL(xmsf, ".uno:OpenHyperlink");
        xDispatch = Desktop.getDispatcher(xmsf, xFrame, "_top", openHyperlink);
        ucb = new UCB(xmsf);

        Process.copyStaticImages(ucb, settings, tempDir);
    }

    /**
     * generates a TOC, copies the layout-specific files, and
     * calles a browser to show "index.html".
     * @param settings
     * @throws Exception
     */
    public void refresh(CGSettings settings)
            throws Exception
    {
        Document doc = (Document) settings.cp_DefaultSession.createDOM();
        CGLayout layout = settings.cp_DefaultSession.getLayout();
        Task task = new Task(PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING, 10000);
        Process.generate(xmsf, layout, doc, fileAccess, tempDir, task);
        Process.copyLayoutFiles(ucb, fileAccess, settings, layout, tempDir);
        xDispatch.dispatch(openHyperlink, loadArgs); //Dispatch.dispatch(openHyperlink, loadArgs);
    }

    private PropertyValue[] loadArgs(String url)
    {
        PropertyValue pv = new PropertyValue();
        pv.Name = PropertyNames.URL;
        pv.Value = url;
        return new PropertyValue[]
                {
                    pv
                };
    }
}

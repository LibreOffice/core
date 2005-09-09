/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TOCPreview.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:58:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.wizards.web;

import org.w3c.dom.Document;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.UCB;
import com.sun.star.wizards.document.OfficeDocument;
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
 * static files, which should not regularily update are copied upon
 * instanciation.
 * The TOC is generated in refresh(...);
 */

public class TOCPreview {
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
        loadArgs = loadArgs( FileAccess.connectURLs(tempDir,"/index.html") );
        openHyperlink = Desktop.getDispatchURL(xmsf, ".uno:OpenHyperlink");
        xDispatch = Desktop.getDispatcher(xmsf, xFrame, "_top", openHyperlink);
        ucb = new UCB(xmsf);

        Process.copyStaticImages(ucb ,settings,tempDir);
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
        Document doc = (Document)settings.cp_DefaultSession.createDOM();
        CGLayout layout = settings.cp_DefaultSession.getLayout();
        Task task = new Task("","",10000);
        Process.generate(xmsf, layout, doc, fileAccess, tempDir,  task);
        Process.copyLayoutFiles(ucb,fileAccess,settings,layout,tempDir);
        xDispatch.dispatch(openHyperlink, loadArgs); //Dispatch.dispatch(openHyperlink, loadArgs);
    }


    private PropertyValue[] loadArgs(String url) {
        PropertyValue pv = new PropertyValue();
        pv.Name = "URL";
        pv.Value = url;
        return new PropertyValue[] {pv};
    }
}

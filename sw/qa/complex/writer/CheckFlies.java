/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: $
 * $Revision: $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package complex.writer;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNamed;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import java.math.BigInteger;
import java.util.Collection;
import java.util.ArrayList;

public class CheckFlies extends ComplexTestCase {
    private XMultiServiceFactory m_xMsf = null;
    private XTextDocument m_xDoc = null;

    public String[] getTestMethodNames() {
        return new String[]{"checkFlies"};
    }

    public void checkFlies()
        throws com.sun.star.uno.Exception,
            com.sun.star.io.IOException
    {
        try {
            m_xMsf = (XMultiServiceFactory)param.getMSF();
            m_xDoc = util.WriterTools.loadTextDoc(m_xMsf, util.utils.getFullTestURL("CheckFlies.odt"));
            com.sun.star.text.XTextFramesSupplier xTFS = (com.sun.star.text.XTextFramesSupplier)UnoRuntime.queryInterface(
                com.sun.star.text.XTextFramesSupplier.class,
                m_xDoc);
            checkTextFrames(xTFS);
            com.sun.star.text.XTextGraphicObjectsSupplier xTGOS = (com.sun.star.text.XTextGraphicObjectsSupplier)UnoRuntime.queryInterface(
                com.sun.star.text.XTextGraphicObjectsSupplier.class,
                m_xDoc);
            checkGraphicFrames(xTGOS);
            com.sun.star.text.XTextEmbeddedObjectsSupplier xTEOS = (com.sun.star.text.XTextEmbeddedObjectsSupplier)UnoRuntime.queryInterface(
                com.sun.star.text.XTextEmbeddedObjectsSupplier.class,
                m_xDoc);
            checkEmbeddedFrames(xTEOS);
        } finally {
            // closing test document
            if(m_xDoc != null)
                util.DesktopTools.closeDoc(m_xDoc);
        }
    }

    private void checkEmbeddedFrames(com.sun.star.text.XTextEmbeddedObjectsSupplier xTGOS)
        throws com.sun.star.lang.WrappedTargetException
    {
        Collection<String> vExpectedEmbeddedFrames = new ArrayList<String>();
        vExpectedEmbeddedFrames.add("Object1");
        int nEmbeddedFrames = vExpectedEmbeddedFrames.size();
        com.sun.star.container.XNameAccess xEmbeddedFrames = xTGOS.getEmbeddedObjects();
        for(String sFrameName : xEmbeddedFrames.getElementNames())
        {
            if(!vExpectedEmbeddedFrames.remove(sFrameName))
                failed("Unexpected frame name");
            try
            {
                xEmbeddedFrames.getByName(sFrameName);
            }
            catch(com.sun.star.container.NoSuchElementException e)
            {
                failed("Could not get embedded frame by name.");
            }
            if(!xEmbeddedFrames.hasByName(sFrameName))
                failed("Could not find embedded frame by name.");
        }
        if(!vExpectedEmbeddedFrames.isEmpty())
            failed("Missing expected embedded frames.");
        try
        {
            xEmbeddedFrames.getByName("Nonexisting embedded frame");
            failed("Got nonexisting embedded frame");
        }
        catch(com.sun.star.container.NoSuchElementException e)
        {}
        if(xEmbeddedFrames.hasByName("Nonexisting embedded frame"))
            failed("Has nonexisting embedded frame");

        com.sun.star.container.XIndexAccess xEmbeddedFramesIdx = (com.sun.star.container.XIndexAccess)UnoRuntime.queryInterface(
            com.sun.star.container.XIndexAccess.class,
            xEmbeddedFrames);
        if(xEmbeddedFramesIdx.getCount() != nEmbeddedFrames)
            failed("Unexpected number of embedded frames reported.");
        for(int nCurrentFrameIdx = 0; nCurrentFrameIdx < xEmbeddedFramesIdx.getCount(); nCurrentFrameIdx++)
        {
            try
            {
                xEmbeddedFramesIdx.getByIndex(nCurrentFrameIdx);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e)
            {
                failed("Could not get embedded frame by index.");
            }
        }
    }

    private void checkGraphicFrames(com.sun.star.text.XTextGraphicObjectsSupplier xTGOS)
        throws com.sun.star.lang.WrappedTargetException
    {
        Collection<String> vExpectedGraphicFrames = new ArrayList<String>();
        vExpectedGraphicFrames.add("graphics1");
        int nGraphicFrames = vExpectedGraphicFrames.size();
        com.sun.star.container.XNameAccess xGraphicFrames = xTGOS.getGraphicObjects();
        for(String sFrameName : xGraphicFrames.getElementNames())
        {
            if(!vExpectedGraphicFrames.remove(sFrameName))
                failed("Unexpected frame name");
            try
            {
                xGraphicFrames.getByName(sFrameName);
            }
            catch(com.sun.star.container.NoSuchElementException e)
            {
                failed("Could not get graphics frame by name.");
            }
            if(!xGraphicFrames.hasByName(sFrameName))
                failed("Could not find graphics frame by name.");
        }
        if(!vExpectedGraphicFrames.isEmpty())
            failed("Missing expected graphics frames.");
        try
        {
            xGraphicFrames.getByName("Nonexisting graphics frame");
            failed("Got nonexisting graphics frame");
        }
        catch(com.sun.star.container.NoSuchElementException e)
        {}
        if(xGraphicFrames.hasByName("Nonexisting graphics frame"))
            failed("Has nonexisting graphics frame");

        com.sun.star.container.XIndexAccess xGraphicFramesIdx = (com.sun.star.container.XIndexAccess)UnoRuntime.queryInterface(
            com.sun.star.container.XIndexAccess.class,
            xGraphicFrames);
        if(xGraphicFramesIdx.getCount() != nGraphicFrames)
            failed("Unexpected number of graphics frames reported.");
        for(int nCurrentFrameIdx = 0; nCurrentFrameIdx < xGraphicFramesIdx.getCount(); nCurrentFrameIdx++)
        {
            try
            {
                xGraphicFramesIdx.getByIndex(nCurrentFrameIdx);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e)
            {
                failed("Could not get graphics frame by index.");
            }
        }
    }

    private void checkTextFrames(com.sun.star.text.XTextFramesSupplier xTFS)
        throws com.sun.star.lang.WrappedTargetException
    {
        Collection<String> vExpectedTextFrames = new ArrayList<String>();
        vExpectedTextFrames.add("Frame1");
        vExpectedTextFrames.add("Frame2");

        int nTextFrames = vExpectedTextFrames.size();
        com.sun.star.container.XNameAccess xTextFrames = xTFS.getTextFrames();
        for(String sFrameName : xTextFrames.getElementNames())
        {
            if(!vExpectedTextFrames.remove(sFrameName))
                failed("Unexpected frame name");
            try
            {
                xTextFrames.getByName(sFrameName);
            }
            catch(com.sun.star.container.NoSuchElementException e)
            {
                failed("Could not get text frame by name.");
            }
            if(!xTextFrames.hasByName(sFrameName))
                failed("Could not find text frame by name.");
        }
        if(!vExpectedTextFrames.isEmpty())
            failed("Missing expected text frames.");
        try
        {
            xTextFrames.getByName("Nonexisting Textframe");
            failed("Got nonexisting text frame.");
        }
        catch(com.sun.star.container.NoSuchElementException e)
        {}
        if(xTextFrames.hasByName("Nonexisting text frame"))
            failed("Has nonexisting text frame.");

        com.sun.star.container.XIndexAccess xTextFramesIdx = (com.sun.star.container.XIndexAccess)UnoRuntime.queryInterface(
            com.sun.star.container.XIndexAccess.class,
            xTextFrames);
        if(xTextFramesIdx.getCount() != nTextFrames)
            failed("Unexpected number of text frames reported.");
        for(int nCurrentFrameIdx = 0; nCurrentFrameIdx < xTextFramesIdx.getCount(); nCurrentFrameIdx++)
        {
            try
            {
                xTextFramesIdx.getByIndex(nCurrentFrameIdx);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e)
            {
                failed("Could not get text frame by index.");
            }
        }
    }
}

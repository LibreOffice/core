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


package com.sun.star.report;

import com.sun.star.awt.Size;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.graphic.XGraphicProvider;
import com.sun.star.io.IOException;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lib.uno.adapter.ByteArrayToXInputStreamAdapter;
import com.sun.star.lib.uno.adapter.InputStreamToXInputStreamAdapter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import java.awt.Dimension;

import java.io.InputStream;


/**
 * @author oj93728
 */
public class SOImageService implements ImageService
{

    private final XGraphicProvider m_xGraphicProvider;

    /**
     * Creates a new instance of SOImageService
     * @param xCompContext
     * @throws ReportExecutionException
     */
    public SOImageService(final XComponentContext xCompContext)
            throws ReportExecutionException, com.sun.star.uno.Exception
    {
        if (xCompContext == null)
        {
            throw new ReportExecutionException();
        }


        final XMultiComponentFactory m_xMCF = xCompContext.getServiceManager();
        m_xGraphicProvider = (XGraphicProvider) UnoRuntime.queryInterface(XGraphicProvider.class,
                m_xMCF.createInstanceWithContext("com.sun.star.graphic.GraphicProvider", xCompContext));

        if (m_xGraphicProvider == null)
        {
            throw new ReportExecutionException("There is no graphic-provider available.");
        }
    }

    public Dimension getImageSize(final InputStream image) throws ReportExecutionException
    {
        return getImageSize(new InputStreamToXInputStreamAdapter(image));
    }

    private Dimension getImageSize(final XInputStream image) throws ReportExecutionException
    {
        final Dimension dim = new Dimension();
        try
        {
            final PropertyValue[] value = new PropertyValue[]
            {
                new PropertyValue()
            };
            // value[0] = new PropertyValue();
            value[0].Name = "InputStream";
            value[0].Value = image;

            final XPropertySet xImage = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,
                    m_xGraphicProvider.queryGraphic(value));

            if (xImage != null)
            {
                final XPropertySetInfo xInfo = xImage.getPropertySetInfo();
                if (xInfo.hasPropertyByName("Size100thMM"))
                {
                    Size imageSize = (Size) xImage.getPropertyValue("Size100thMM");
                    dim.setSize(imageSize.Width, imageSize.Height);
                    if (dim.height == 0 && dim.width == 0)
                    {
                        imageSize = (Size) xImage.getPropertyValue("SizePixel");
                        final int dpi = java.awt.Toolkit.getDefaultToolkit().getScreenResolution();
                        final double fac = 2540 / (double) dpi;
                        dim.setSize(imageSize.Width * fac, imageSize.Height * fac);
                    }
                }
                else if (xInfo.hasPropertyByName("SizePixel"))
                {
                    final Size imageSize = (Size) xImage.getPropertyValue("SizePixel");
                    final int dpi = java.awt.Toolkit.getDefaultToolkit().getScreenResolution();
                    final double fac = 2540 / dpi;
                    dim.setSize(imageSize.Width * fac, imageSize.Height * fac);
                }
            }
        }
        catch (Exception ex)
        {
            throw new ReportExecutionException("Failed to query Image-Size", ex);
        }
        return dim;
    }

    public Dimension getImageSize(final byte[] image) throws ReportExecutionException
    {
        return getImageSize(new ByteArrayToXInputStreamAdapter(image));
    }

    private String getMimeType(final XInputStream image) throws ReportExecutionException
    {
        try
        {
            final PropertyValue[] value = new PropertyValue[]
            {
                new PropertyValue()
            };
            value[0].Name = "InputStream";
            value[0].Value = image;

            final XPropertySet xImage = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,
                    m_xGraphicProvider.queryGraphic(value));

            if (xImage != null)
            {
                final XPropertySetInfo xInfo = xImage.getPropertySetInfo();
                if (xInfo.hasPropertyByName("MimeType"))
                {
                    return (String) xImage.getPropertyValue("MimeType");
                }
            }
        }
        catch (UnknownPropertyException ex)
        {
            throw new ReportExecutionException();
        }
        catch (WrappedTargetException ex)
        {
            throw new ReportExecutionException();
        }
        catch (com.sun.star.lang.IllegalArgumentException ex)
        {
            throw new ReportExecutionException();
        }
        catch (IOException ex)
        {
            throw new ReportExecutionException();
        }
        return null;
    }

    public String getMimeType(final InputStream image) throws ReportExecutionException
    {
        return getMimeType(new InputStreamToXInputStreamAdapter(image));
    }

    public String getMimeType(final byte[] image) throws ReportExecutionException
    {
        return getMimeType(new ByteArrayToXInputStreamAdapter(image));
    }
}

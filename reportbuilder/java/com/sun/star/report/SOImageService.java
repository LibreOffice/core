/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

    public Size getImageSize(final InputStream image) throws ReportExecutionException
    {
        return getImageSize(new InputStreamToXInputStreamAdapter(image));
    }

    private Size getImageSize(final XInputStream image) throws ReportExecutionException
    {
        final Size dim = new Size();
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
                    dim.Width = imageSize.Width;
                    dim.Height = imageSize.Height;
                    if (dim.Height == 0 && dim.Width == 0)
                    {
                        imageSize = (Size) xImage.getPropertyValue("SizePixel");
                        final int dpi = java.awt.Toolkit.getDefaultToolkit().getScreenResolution();
                        final double fac = 2540 / (double) dpi;
                        dim.Width = (int) (imageSize.Width * fac);
                        dim.Height = (int) (imageSize.Height * fac);
                    }
                }
                else if (xInfo.hasPropertyByName("SizePixel"))
                {
                    final Size imageSize = (Size) xImage.getPropertyValue("SizePixel");
                    final int dpi = java.awt.Toolkit.getDefaultToolkit().getScreenResolution();
                    final double fac = 2540 / dpi;
                    dim.Width = (int) (imageSize.Width * fac);
                    dim.Height = (int) (imageSize.Height * fac);
                }
            }
        }
        catch (Exception ex)
        {
            throw new ReportExecutionException("Failed to query Image-Size", ex);
        }
        return dim;
    }

    public Size getImageSize(final byte[] image) throws ReportExecutionException
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

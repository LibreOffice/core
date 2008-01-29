/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SOImageService.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:43:19 $
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

package com.sun.star.report;

import java.awt.Dimension;
import java.io.InputStream;

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

/**
 * @author oj93728
 */
public class SOImageService implements ImageService
{
  private XGraphicProvider m_xGraphicProvider;

  /**
   * Creates a new instance of SOImageService
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
      final PropertyValue[] value = new PropertyValue[1];
      value[0] = new PropertyValue();
      value[0].Name = "InputStream";
      value[0].Value = image;

      final XPropertySet xImage = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
        m_xGraphicProvider.queryGraphic( value ) );

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
            final double fac = 2540 / dpi;
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
    catch (UnknownPropertyException ex)
    {
      throw new ReportExecutionException("Failed to query Image-Size", ex);
    }
    catch (WrappedTargetException ex)
    {
      throw new ReportExecutionException("Failed to query Image-Size", ex);
    }
    catch (com.sun.star.lang.IllegalArgumentException ex)
    {
      throw new ReportExecutionException("Failed to query Image-Size", ex);
    }
    catch (IOException ex)
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
    String mimeType = null;
    try
    {
      final PropertyValue[] value = new PropertyValue[] { new PropertyValue() };
      value[0].Name = "InputStream";
      value[0].Value = image;

      final XPropertySet xImage = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class,
          m_xGraphicProvider.queryGraphic(value));

      if (xImage != null)
      {
        final XPropertySetInfo xInfo = xImage.getPropertySetInfo();
        if (xInfo.hasPropertyByName("MimeType"))
        {
          mimeType = (String) xImage.getPropertyValue("MimeType");
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
    return mimeType;
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

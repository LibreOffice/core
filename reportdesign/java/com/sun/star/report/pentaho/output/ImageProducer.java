/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImageProducer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:34:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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


package com.sun.star.report.pentaho.output;

import java.awt.Image;
import java.awt.Dimension;
import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.sql.Blob;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Arrays;

import com.sun.star.report.InputRepository;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.ImageService;
import com.sun.star.report.ReportExecutionException;
import com.sun.star.report.pentaho.DefaultNameGenerator;
import com.keypoint.PngEncoder;
import org.jfree.io.IOUtils;
import org.jfree.layouting.input.style.values.CSSNumericType;
import org.jfree.layouting.input.style.values.CSSNumericValue;
import org.jfree.util.Log;
import org.jfree.util.WaitingImageObserver;

/**
 * This class manages the images embedded in a report.
 *
 * @author Thomas Morgner
 * @since 31.03.2007
 */
public class ImageProducer
{
  public static class OfficeImage
  {
    private CSSNumericValue width;
    private CSSNumericValue height;
    private String embeddableLink;

    public OfficeImage(final String embeddableLink, final CSSNumericValue width, final CSSNumericValue height)
    {
      this.embeddableLink = embeddableLink;
      this.width = width;
      this.height = height;
    }

    public CSSNumericValue getWidth()
    {
      return width;
    }

    public CSSNumericValue getHeight()
    {
      return height;
    }

    public String getEmbeddableLink()
    {
      return embeddableLink;
    }
  }

  private static class ByteDataImageKey
  {
    private byte[] keyData;
    private Integer hashCode;

    protected ByteDataImageKey(final byte[] keyData)
    {
      if (keyData == null)
      {
        throw new NullPointerException();
      }
      this.keyData = keyData;
    }


    public boolean equals(final Object o)
    {
      if (this == o)
      {
        return true;
      }
      if (o == null || getClass() != o.getClass())
      {
        return false;
      }

      final ByteDataImageKey key = (ByteDataImageKey) o;
      if (!Arrays.equals(keyData, key.keyData))
      {
        return false;
      }

      return true;
    }

    public int hashCode()
    {
      if (hashCode != null)
      {
        return hashCode.intValue();
      }

      final int length = Math.min(keyData.length, 512);
      int hashCode = 0;
      for (int i = 0; i < length; i++)
      {
        final byte b = keyData[i];
        hashCode = b + hashCode * 23;
      }
      this.hashCode = new Integer(hashCode);
      return hashCode;
    }
  }

  private HashMap imageCache;
  private InputRepository inputRepository;
  private OutputRepository outputRepository;
  private ImageService imageService;
  private DefaultNameGenerator nameGenerator;

  public ImageProducer(final InputRepository inputRepository,
                       final OutputRepository outputRepository,
                       final ImageService imageService)
  {
    if (inputRepository == null)
    {
      throw new NullPointerException();
    }
    if (outputRepository == null)
    {
      throw new NullPointerException();
    }
    if (imageService == null)
    {
      throw new NullPointerException();
    }

    this.inputRepository = inputRepository;
    this.outputRepository = outputRepository;
    this.imageService = imageService;
    this.imageCache = new HashMap();
    this.nameGenerator = new DefaultNameGenerator(outputRepository);
  }

  /**
   * Image-Data can be one of the following types: String, URL, URI, byte-array, blob.
   *
   * @param imageData
   * @param preserveIRI
   * @return
   */
  public OfficeImage produceImage(final Object imageData,
                                  final boolean preserveIRI)
  {

    Log.debug ("Want to produce image " + imageData);
    if (imageData instanceof String)
    {
      return produceFromString((String) imageData, preserveIRI);
    }

    if (imageData instanceof URL)
    {
      return produceFromURL((URL) imageData, preserveIRI);
    }

    if (imageData instanceof Blob)
    {
      return produceFromBlob((Blob) imageData);
    }

    if (imageData instanceof byte[])
    {
      return produceFromByteArray((byte[]) imageData);
    }

    if (imageData instanceof Image)
    {
      return produceFromImage((Image) imageData);
    }
    // not usable ..
    return null;
  }

  private OfficeImage produceFromImage(final Image image)
  {
    // quick caching ... use a weak list ...
    final WaitingImageObserver obs = new WaitingImageObserver(image);
    obs.waitImageLoaded();

    final PngEncoder encoder = new PngEncoder(image, PngEncoder.ENCODE_ALPHA, PngEncoder.FILTER_NONE, 5);
    final byte[] data = encoder.pngEncode();
    return produceFromByteArray(data);
  }

  private OfficeImage produceFromBlob(final Blob blob)
  {
    try
    {
      final InputStream inputStream = blob.getBinaryStream();
      final int length = (int) blob.length();

      final ByteArrayOutputStream bout = new ByteArrayOutputStream(length);
      try
      {
        IOUtils.getInstance().copyStreams(inputStream, bout);
      }
      finally
      {
        inputStream.close();
      }
      return produceFromByteArray(bout.toByteArray());
    }
    catch (IOException e)
    {
      Log.warn("Failed to produce image from Blob", e);
    }
    catch (SQLException e)
    {
      Log.warn("Failed to produce image from Blob", e);
    }
    return null;
  }

  private OfficeImage produceFromByteArray(final byte[] data)
  {
    final ByteDataImageKey imageKey = new ByteDataImageKey(data);
    final OfficeImage o = (OfficeImage) imageCache.get(imageKey);
    if (o != null)
    {
      return o;
    }

    try
    {
      final String mimeType = imageService.getMimeType(data);
      final Dimension dims = imageService.getImageSize(data);

      // copy the image into the local output-storage
      // todo: Implement data-fingerprinting so that we can detect the mime-type
      final String name = nameGenerator.generateName("Pictures/image", mimeType);
      final OutputStream outputStream = outputRepository.createOutputStream(name, mimeType);
      final ByteArrayInputStream bin = new ByteArrayInputStream(data);

      try
      {
        IOUtils.getInstance().copyStreams(bin, outputStream);
      }
      finally
      {
        outputStream.close();
      }

      final CSSNumericValue widthVal = CSSNumericValue.createValue(CSSNumericType.MM, dims.getWidth() / 100.0);
      final CSSNumericValue heightVal = CSSNumericValue.createValue(CSSNumericType.MM, dims.getHeight() / 100.0);
      final OfficeImage officeImage = new OfficeImage(name, widthVal, heightVal);
      imageCache.put(imageKey, officeImage);
      return officeImage;
    }
    catch (IOException e)
    {
      Log.warn("Failed to load image from local input-repository", e);
    }
    catch (ReportExecutionException e)
    {
      Log.warn("Failed to create image from local input-repository", e);
    }
    return null;
  }

  private OfficeImage produceFromString(final String source,
                                        final boolean preserveIRI)
  {

    try
    {
      final URL url = new URL(source);
      return produceFromURL(url, preserveIRI);
    }
    catch (MalformedURLException e)
    {
      // ignore .. but we had to try this ..
    }

    final OfficeImage o = (OfficeImage) imageCache.get(source);
    if (o != null)
    {
      return o;
    }

    // Next, check whether this is a local path.
    if (inputRepository.isReadable(source))
    {
      // cool, the file exists. Let's try to read it.
      try
      {
        final ByteArrayOutputStream bout = new ByteArrayOutputStream(8192);
        final InputStream inputStream = inputRepository.createInputStream(source);
        try
        {
          IOUtils.getInstance().copyStreams(inputStream, bout);
        }
        finally
        {
          inputStream.close();
        }
        final byte[] data = bout.toByteArray();
        final Dimension dims = imageService.getImageSize(data);
        final String mimeType = imageService.getMimeType(data);

        final CSSNumericValue widthVal = CSSNumericValue.createValue(CSSNumericType.MM, dims.getWidth() / 100.0);
        final CSSNumericValue heightVal = CSSNumericValue.createValue(CSSNumericType.MM, dims.getHeight() / 100.0);

        final String filename = copyToOutputRepository(mimeType, source, data);
        final OfficeImage officeImage = new OfficeImage(filename, widthVal, heightVal);
        imageCache.put(source, officeImage);
        return officeImage;
      }
      catch (IOException e)
      {
        Log.warn("Failed to load image from local input-repository", e);
      }
      catch (ReportExecutionException e)
      {
        Log.warn("Failed to create image from local input-repository", e);
      }
    }

    // Return the image as broken image instead ..
    final OfficeImage officeImage = new OfficeImage(source, null, null);
    imageCache.put(source, officeImage);
    return officeImage;
  }

  private OfficeImage produceFromURL(final URL url,
                                     final boolean preserveIRI)
  {
    final OfficeImage o = (OfficeImage) imageCache.get(url);
    if (o != null)
    {
      return o;
    }

    try
    {
      final ByteArrayOutputStream bout = new ByteArrayOutputStream(8192);
      final URLConnection urlConnection = url.openConnection();
      final InputStream inputStream = new BufferedInputStream(urlConnection.getInputStream());
      try
      {
        IOUtils.getInstance().copyStreams(inputStream, bout);
      }
      finally
      {
        inputStream.close();
      }
      final byte[] data = bout.toByteArray();

      final Dimension dims = imageService.getImageSize(data);
      final String mimeType = imageService.getMimeType(data);
      final CSSNumericValue widthVal = CSSNumericValue.createValue(CSSNumericType.MM, dims.getWidth() / 100.0);
      final CSSNumericValue heightVal = CSSNumericValue.createValue(CSSNumericType.MM, dims.getHeight() / 100.0);

      if (preserveIRI == false)
      {
        final OfficeImage retval = new OfficeImage(url.toString(), widthVal, heightVal);
        imageCache.put(url, retval);
        return retval;
      }

      final String file = url.getFile();
      final String name = copyToOutputRepository(mimeType, file, data);
      final OfficeImage officeImage = new OfficeImage(name, widthVal, heightVal);
      imageCache.put(url, officeImage);
      return officeImage;
    }
    catch (IOException e)
    {
      Log.warn("Failed to load image from local input-repository" + e);
    }
    catch (ReportExecutionException e)
    {
      Log.warn("Failed to create image from local input-repository" + e);
    }

    if (preserveIRI == false)
    {
      final OfficeImage image = new OfficeImage(url.toString(), null, null);
      imageCache.put(url, image);
      return image;
    }

    // OK, everything failed; the image is not - repeat it - not usable.
    return null;
  }

  private String copyToOutputRepository(final String urlMimeType, final String file, final byte[] data)
      throws IOException, ReportExecutionException
  {
    final String mimeType;
    if (urlMimeType == null)
    {
      mimeType = imageService.getMimeType(data);
    }
    else
    {
      mimeType = urlMimeType;
    }

    // copy the image into the local output-storage
    final String name = nameGenerator.generateName("Pictures/image", mimeType);
    final OutputStream outputStream = outputRepository.createOutputStream(name, mimeType);
    final ByteArrayInputStream bin = new ByteArrayInputStream(data);

    try
    {
      IOUtils.getInstance().copyStreams(bin, outputStream);
    }
    finally
    {
      outputStream.close();
    }
    return name;
  }

}

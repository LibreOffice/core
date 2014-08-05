/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package graphical;

import java.awt.Image;
import java.awt.image.PixelGrabber;
import java.awt.image.ImageObserver;
import java.io.File;
import java.lang.reflect.Method;

class ImageHelper
{
    Image m_aImage;
    int[] m_aPixels;
    int m_w = 0;
    int m_h = 0;
    boolean m_bGrabbed = false;

    private ImageHelper(Image _aImage)
    {
         m_aImage = _aImage;

         // grab all (consume much memory)
         m_w = getWidth();
         m_h = getHeight();
         int x = 0;
         int y = 0;
         m_aPixels = new int[m_w * m_h];
         PixelGrabber pg = new PixelGrabber(m_aImage, x, y, m_w, m_h, m_aPixels, 0, m_w);
         try
         {
             pg.grabPixels();
         }
         catch (InterruptedException e)
         {
             System.err.println("interrupted waiting for pixels!");
             return;
         }
         if ((pg.getStatus() & ImageObserver.ABORT) != 0)
         {
             System.err.println("image fetch aborted or errored");
             return;
         }
         m_bGrabbed = true;
    }
    public int getWidth() {return m_aImage.getWidth(null);}
    public int getHeight() {return m_aImage.getHeight(null);}
    // direct access to a pixel
    public int getPixel(final int x, final int y)
        {
            return m_aPixels[y * m_w + x];
        }

    public static ImageHelper createImageHelper(String _sFilename)
        throws java.io.IOException
        {
            Image aImage = null;
            File aFile = new File(_sFilename);
            Exception ex = null;
            try {
                Class<?> imageIOClass = Class.forName("javax.imageio.ImageIO");
                Method readMethod = imageIOClass.getDeclaredMethod("read", new Class[]{java.io.File.class});
                Object retValue = readMethod.invoke(imageIOClass, new Object[]{aFile});
                aImage = (Image)retValue;
            }
            catch(java.lang.ClassNotFoundException e) {
                ex = e;
            }
            catch(java.lang.NoSuchMethodException e) {
                ex = e;
            }
            catch(java.lang.IllegalAccessException e) {
                ex = e;
            }
            catch(java.lang.reflect.InvocationTargetException e) {
                ex = e;
            }

            if (ex != null) {
                // get Java version:
                String javaVersion = System.getProperty("java.version");
                throw new java.io.IOException(
                    "Cannot construct object with current Java version " +
                    javaVersion + ": " + ex.getMessage());
            }
            return new ImageHelper(aImage);
        }
}

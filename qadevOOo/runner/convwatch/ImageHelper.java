/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImageHelper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:13:55 $
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

package convwatch;

import java.awt.Image;
import java.awt.image.PixelGrabber;
import java.awt.image.ImageObserver;
import java.io.File;
//import javax.imageio.ImageIO;
import java.io.IOException;
import java.lang.reflect.Method;

class ImageHelper
{
    Image m_aImage;
    int[] m_aPixels;
    int m_w = 0;
    int m_h = 0;
    boolean m_bGrabbed = false;

    public ImageHelper(Image _aImage)
    {
         m_aImage = _aImage;

         // grab all (consume much memory)
         m_w = getWidth();
         m_h = getHeight();
         int x = 0;
         int y = 0;
         m_aPixels = new int[m_w * m_h];
         PixelGrabber pg = new PixelGrabber(m_aImage, x, y, m_w, m_h, m_aPixels, 0, m_w);
         try {
             pg.grabPixels();
         } catch (InterruptedException e) {
             System.err.println("interrupted waiting for pixels!");
             return;
         }
         if ((pg.getStatus() & ImageObserver.ABORT) != 0) {
             System.err.println("image fetch aborted or errored");
             return;
         }
         m_bGrabbed = true;
    }
    public int getWidth() {return m_aImage.getWidth(null);}
    public int getHeight() {return m_aImage.getHeight(null);}
    // direct access to a pixel
    public int getPixel(int x, int y)
        {
            return m_aPixels[y * m_w + x];
        }

    // Write down the current image to a file.
    // public void storeImage(String _sFilename)
    //    {
    //    }

    public static ImageHelper createImageHelper(String _sFilename)
        throws java.io.IOException
        {
            Image aImage = null;
            File aFile = new File(_sFilename);
            Exception ex = null;
            try {
                Class imageIOClass = Class.forName("javax.imageio.ImageIO");
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
//            aImage = ImageIO.read(aFile);
            return new ImageHelper(aImage);
        }
}

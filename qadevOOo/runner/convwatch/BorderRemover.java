/*************************************************************************
 *
 *  $RCSfile: BorderRemover.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2004-12-10 16:55:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package convwatch;

import convwatch.ImageHelper;
import java.io.File;
import java.awt.image.RenderedImage;
import java.awt.image.BufferedImage;
import java.awt.Image;
import java.lang.reflect.Method;

// -----------------------------------------------------------------------------
class Rect
{
    int x;
    int y;
    int w;
    int h;

    public Rect(int _x, int _y, int _w, int _h)
        {
            x = _x;
            y = _y;
            w = _w;
            h = _h;
        }
    public int getX() {return x;}
    public int getY() {return y;}
    public int getWidth() {return w;}
    public int getHeight() {return h;}
}

class BorderRemover
{
    ImageHelper m_aImage;

    // Helper values, filled after find Border

    // --------------------------------- test mode ---------------------------------

    // void pixelValue(int pixel)
    // {
    //     int alpha = (pixel >> 24) & 0xff;
    //     int red   = (pixel >> 16) & 0xff;
    //     int green = (pixel >>  8) & 0xff;
    //     int blue  = (pixel      ) & 0xff;
    //     int dummy = 0;
    // }

    /*
     * compares 2 colors with a given tolerance. So it's possible to check differences approximate.
     * @param _nColor1
     * @param _nColor2
     * @param _nTolerance is a percentage value how strong the colors could be differ

     */
    boolean compareColorWithTolerance(int _nColor1, int _nColor2, int _nTolerance)
        {
            // int alpha1 = (_nColor1 >> 24) & 0xff;
            int red1   = (_nColor1 >> 16) & 0xff;
            int green1 = (_nColor1 >>  8) & 0xff;
            int blue1  = (_nColor1      ) & 0xff;

            // int alpha2 = (_nColor2 >> 24) & 0xff;
            int red2   = (_nColor2 >> 16) & 0xff;
            int green2 = (_nColor2 >>  8) & 0xff;
            int blue2  = (_nColor2      ) & 0xff;

            if (_nTolerance > 100)
            {
                _nTolerance = 100;
            }

            // calculate tolerance halve
            double nTolerable = (_nTolerance * 256 / 100);
            if (nTolerable < 0)
            {
                nTolerable = 0;
            }

            // X - th < Y < X + th
            // if ((red1 - nTolerable) < red2 && red2 < (red1 + nTolerable))
            // is the same
            // abs (X - Y) < th
            if (Math.abs(red1 - red2) < nTolerable)
            {
                if (Math.abs(green1 - green2) < nTolerable)
                {
                    if (Math.abs(blue1 - blue2) < nTolerable)
                    {
                        return true;
                    }
                    else
                    {
                        // blue differ
                    }
                }
                else
                {
                    // green differ
                }
            }
            else
            {
                // red differ
            }

            return false;
        }

    /**
     * create a new image from an exist one without it's borders
     * open the file (_sFilenameFrom) as an image, check if it contains any borders and remove
     * the borders.
     */
    public boolean createNewImageWithoutBorder(String _sFilenameFrom, String _sFilenameTo)
        throws java.io.IOException
        {
            // System.out.println("load image: " + fileName);
            m_aImage = ImageHelper.createImageHelper(_sFilenameFrom);

            // System.out.println("image  width:" + String.valueOf(m_aImage.getWidth()));
            // System.out.println("image height:" + String.valueOf(m_aImage.getHeight()));

            // int nw = graphics_stuff.countNotWhitePixel(m_aImage);
            // System.out.println("not white pixels:" + String.valueOf(nw));

            // int nb = graphics_stuff.countNotBlackPixel(m_aImage);
            // System.out.println("not black pixels:" + String.valueOf(nb));

            int nBorderColor = m_aImage.getPixel(0,0);
            Rect aInnerRect = findBorder(m_aImage, nBorderColor);

            RenderedImage aImage = createImage(m_aImage, aInnerRect);

            File aWriteFile = new File(_sFilenameTo);

            Exception ex = null;
            try {
                Class imageIOClass = Class.forName("javax.imageio.ImageIO");
                Method writeMethod = imageIOClass.getDeclaredMethod("write", new Class[]{java.awt.Image.class, java.lang.String.class, java.io.File.class});
                writeMethod.invoke(imageIOClass, new Object[]{aImage, "jpg", aWriteFile});
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
//            ImageIO.write(aImage, "jpg", aWriteFile);

            return true;
        }


    /**
     * runs through the image, pixel by pixel
     * as long as found pixels like the color at (0,0) this is interpreted as border.
     * as result it fills the m_nXMin, m_nXMax, m_nYMin, m_nYMax values.
     */

    Rect findBorder(ImageHelper _aImage, int _nBorderColor)
        {
            int h = _aImage.getHeight();
            int w = _aImage.getWidth();
            int nXMin = w;
            int nXMax = 0;
            int nYMin = h;
            int nYMax = 0;

            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < nXMin; x++)
                {
                    // handlesinglepixel(x+i, y+j, pixels[j * w + i]);
                    int nCurrentColor = _aImage.getPixel(x, y);
                    if (! compareColorWithTolerance(nCurrentColor, _nBorderColor, 10))
                    {
                        // pixelValue(nCurrentColor);
                        // System.out.print("*");
                        nXMin = java.lang.Math.min(nXMin, x);
                        nYMin = java.lang.Math.min(nYMin, y);
                    }
                    // else
                    // {
                    //     System.out.print(" ");
                    // }
                }
            }
            for (int y = 0; y < h; y++)
            {
                for (int nx = w - 1; nx >= nXMax; --nx)
                {
                    int ny = h - y - 1;
                    int nCurrentColor = _aImage.getPixel(nx, ny);
                    if (! compareColorWithTolerance(nCurrentColor, _nBorderColor, 10))
                    {
                        nXMax = java.lang.Math.max(nXMax, nx);
                        nYMax = java.lang.Math.max(nYMax, ny);
                    }
                }
                // System.out.println();
            }
            // System.out.println("xmin: " + String.valueOf(nXMin));
            // System.out.println("xmax: " + String.valueOf(nXMax));
            // System.out.println("ymin: " + String.valueOf(nYMin));
            // System.out.println("ymax: " + String.valueOf(nYMax));

            Rect aRect;
            if (nXMin < nXMax && nYMin < nYMax)
            {
                int nw = nXMax - nXMin + 1;
                int nh = nYMax - nYMin + 1;

                // this is the rectangle around the image content.
                aRect = new Rect(nXMin, nYMin, nw, nh );
            }
            else
            {
                // create the smalles possible image
                aRect = new Rect(0,0,1,1);
            }


            // m_nXMin = nXMin;
            // m_nXMax = nXMax;
            // m_nYMin = nYMin;
            // m_nYMax = nYMax;
            return aRect;
        }

    RenderedImage createImage(ImageHelper _aImage, Rect _aRect) throws IllegalArgumentException
        {
// TODO: throw if w or h < 0
            int w = _aRect.getWidth();
            int h = _aRect.getHeight();

            if (w <= 0 || h <= 0)
            {
                throw new IllegalArgumentException("width or height are too small or negative.");
            }

            BufferedImage aBI = new BufferedImage(w, h, BufferedImage.TYPE_INT_RGB);

            int nXOffset = _aRect.getX();
            int nYOffset = _aRect.getY();

            // Memory Block move
            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    // aPixels[y * w + x] = m_aImage.getPixel(m_nXMin + x, m_nYMin + y);
                    aBI.setRGB(x, y, _aImage.getPixel(x + nXOffset, y + nYOffset));
                }
            }
            // java.awt.image.MemoryImageSource aSource = new java.awt.image.MemoryImageSource(w, h, aPixels, 0, w);
//             return java.awt.Component.createImage(aSource);
             // return java.awt.Toolkit.getDefaultToolkit().createImage(aSource);
             return aBI;
        }

}

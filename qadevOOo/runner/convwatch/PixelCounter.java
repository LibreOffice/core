/*************************************************************************
 *
 *  $RCSfile: PixelCounter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:22:18 $
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

// import java.awt.Image;
import convwatch.ImageHelper;
import java.io.IOException;

// -----------------------------------------------------------------------------
abstract class CountPixel
{
    int m_nCount = 0;
    public int getCount() {return m_nCount;}
    public abstract void count(int _nRGB);
}

// -----------------------------------------------------------------------------
class CountNotWhite extends CountPixel
{
    public CountNotWhite()
        {
            // System.out.println("CountWhite()");
        }

    public void count(int pixel)
        {
            int alpha = (pixel >> 24) & 0xff;
            int red   = (pixel >> 16) & 0xff;
            int green = (pixel >>  8) & 0xff;
            int blue  = (pixel      ) & 0xff;

            // System.out.println(String.valueOf(red) + ":" + String.valueOf(green) + ":" + String.valueOf(blue));
            if (red == 0xff && green == 0xff && blue == 0xff)
            {
                return;
            }
            m_nCount++;
        }
}

// -----------------------------------------------------------------------------
class CountNotBlack extends CountPixel
{
    public CountNotBlack()
        {
            // System.out.println("CountBlack()");
        }

    public void count(int pixel)
        {
            int alpha = (pixel >> 24) & 0xff;
            int red   = (pixel >> 16) & 0xff;
            int green = (pixel >>  8) & 0xff;
            int blue  = (pixel      ) & 0xff;

            if (red == 0x00 && green == 0x00 && blue == 0x00)
            {
                return;
            }
            m_nCount++;
        }
}

// -----------------------------------------------------------------------------
class graphics_stuff
{
    public int stuff()
        {
// (1) decoding
            int rgba = 0; // ...; // comes from PixelGrabber, BufferedImage.getRGB etc.
            int red = (rgba >> 16) & 0xff;
            int green = (rgba >> 8) & 0xff;
            int blue = rgba & 0xff;
            int alpha = (rgba >> 24) & 0xff;
// (2) now modify red, green, blue and alpha as you like;
//     make sure that each of the four values stays in the
//     interval 0 to 255
//            ...
// (3) and encode back to an int, e.g. to give it to MemoryImageSource or
//     BufferedImage.setRGB
                rgba = (alpha << 24) | (red << 16) | (green << 8) | blue;
                return 0;
        }

    public static void handlesinglepixel(int x, int y, int pixel)
        {
            int alpha = (pixel >> 24) & 0xff;
            int red   = (pixel >> 16) & 0xff;
            int green = (pixel >>  8) & 0xff;
            int blue  = (pixel      ) & 0xff;
            // Deal with the pixel as necessary...
        }

    public static void countPixel(ImageHelper img, int _x, int _y, int _w, int _h, CountPixel _aPixelCounter)
        {
            for (int y = 0; y < _h; y++) {
                for (int x = 0; x < _w; x++) {
                    // handlesinglepixel(x+i, y+j, pixels[j * w + i]);
                    _aPixelCounter.count(img.getPixel(x,y));
                }
            }
        }
    public static int countNotWhitePixel(ImageHelper _aImage)
        {
            int w = _aImage.getWidth();
            int h = _aImage.getHeight();

            CountPixel aCountNotWhite = new CountNotWhite();
            countPixel(_aImage, 0, 0, w, h, aCountNotWhite);
            return aCountNotWhite.getCount();
        }

    public static int countNotBlackPixel(ImageHelper _aImage)
        {
            int w = _aImage.getWidth();
            int h = _aImage.getHeight();

            CountPixel aCountNotBlack = new CountNotBlack();
            countPixel(_aImage, 0, 0, w, h, aCountNotBlack);
            return aCountNotBlack.getCount();
        }
}

// -----------------------------------------------------------------------------

public class PixelCounter {
    // private Image m_aImage;
    ImageHelper m_aImage;


    public int countNotWhitePixel(String _sFile)
        throws java.io.IOException
        {
            m_aImage = ImageHelper.createImageHelper(_sFile);
            int nw = graphics_stuff.countNotWhitePixel(m_aImage);
            return nw;
        }

    public int countNotBlackPixel(String _sFile)
        throws java.io.IOException
        {
            m_aImage = ImageHelper.createImageHelper(_sFile);
            int nw = graphics_stuff.countNotBlackPixel(m_aImage);
            return nw;
        }

    public static int countNotWhitePixelsFromImage(String _sFile)
        throws java.io.IOException
        {
            PixelCounter a = new PixelCounter();
            return a.countNotWhitePixel(_sFile);
        }

    public static int countNotBlackPixelsFromImage(String _sFile)
        throws java.io.IOException
        {
            PixelCounter a = new PixelCounter();
            return a.countNotBlackPixel(_sFile);
        }

    // -----------------------------------------------------------------------------

    public static void main(String[] args) {

        String a = StringHelper.createValueString(10, 4);
        int dummy = 1;
/*
 BorderRemover a = new BorderRemover();
        try
        {
            a.createNewImageWithoutBorder(args[0], args[1]);
        }
        catch(java.io.IOException e)
        {
            System.out.println("Exception caught.");
        }
 */
    }
}



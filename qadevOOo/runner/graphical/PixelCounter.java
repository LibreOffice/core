/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PixelCounter.java,v $
 * $Revision: 1.1.2.1 $
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

package graphical;


// -----------------------------------------------------------------------------
abstract class CountPixel
{
    protected int m_nCount = 0;
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

    public void countold(final int pixel)
        {
            // final int alpha = (pixel >> 24) & 0xff;
            final int red   = (pixel >> 16) & 0xff;
            final int green = (pixel >>  8) & 0xff;
            final int blue  = (pixel      ) & 0xff;

            // System.out.println(String.valueOf(red) + ":" + String.valueOf(green) + ":" + String.valueOf(blue));
            if (red == 0xff && green == 0xff && blue == 0xff)
            {
                return;
            }
            ++m_nCount;
        }
    public void count(final int pixel)
        {
            // final int alpha = (pixel >> 24) & 0xff;
            final int blue  = (pixel      ) & 0xff;
            if (blue != 0xff)
            {
                ++m_nCount;
                return;
            }
            final int green = (pixel >>  8) & 0xff;
            if (green != 0xff)
            {
                ++m_nCount;
                return;
            }
            final int red   = (pixel >> 16) & 0xff;
            if (red != 0xff)
            {
                ++m_nCount;
                return;
            }
        }
}

// -----------------------------------------------------------------------------
class CountNotBlack extends CountPixel
{
    public CountNotBlack()
        {
            // System.out.println("CountBlack()");
        }

    public void countold(final int pixel)
        {
            // final int alpha = (pixel >> 24) & 0xff;
            final int red   = (pixel >> 16) & 0xff;
            final int green = (pixel >>  8) & 0xff;
            final int blue  = (pixel      ) & 0xff;

            if (red == 0x00 && green == 0x00 && blue == 0x00)
            {
                return;
            }
            ++m_nCount;
        }
    public void count(final int pixel)
        {
            // final int alpha = (pixel >> 24) & 0xff;
            final int blue  = (pixel      ) & 0xff;
            if (blue != 0x00)
            {
                ++m_nCount;
                return;
            }
            final int green = (pixel >>  8) & 0xff;
            if (green != 0x00)
            {
                ++m_nCount;
                return;
            }
            final int red   = (pixel >> 16) & 0xff;
            if (red != 0x00)
            {
                ++m_nCount;
                return;
            }
        }
}

// -----------------------------------------------------------------------------
class graphics_stuff
{
//    public int stuff()
//        {
//// (1) decoding
//            int rgba = 0; // ...; // comes from PixelGrabber, BufferedImage.getRGB etc.
//            int red = (rgba >> 16) & 0xff;
//            int green = (rgba >> 8) & 0xff;
//            int blue = rgba & 0xff;
//            int alpha = (rgba >> 24) & 0xff;
//// (2) now modify red, green, blue and alpha as you like;
////     make sure that each of the four values stays in the
////     interval 0 to 255
////            ...
//// (3) and encode back to an int, e.g. to give it to MemoryImageSource or
////     BufferedImage.setRGB
//                rgba = (alpha << 24) | (red << 16) | (green << 8) | blue;
//                return 0;
//        }

//    public static void handlesinglepixel(int x, int y, int pixel)
//        {
//            int alpha = (pixel >> 24) & 0xff;
//            int red   = (pixel >> 16) & 0xff;
//            int green = (pixel >>  8) & 0xff;
//            int blue  = (pixel      ) & 0xff;
//            // Deal with the pixel as necessary...
//        }

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
            final int w = _aImage.getWidth();
            final int h = _aImage.getHeight();

            CountPixel aCountNotWhite = new CountNotWhite();
            countPixel(_aImage, 0, 0, w, h, aCountNotWhite);
            return aCountNotWhite.getCount();
        }

    public static int countNotBlackPixel(ImageHelper _aImage)
        {
            final int w = _aImage.getWidth();
            final int h = _aImage.getHeight();

            CountPixel aCountNotBlack = new CountNotBlack();
            countPixel(_aImage, 0, 0, w, h, aCountNotBlack);
            return aCountNotBlack.getCount();
        }
}

// -----------------------------------------------------------------------------

public class PixelCounter {
    // private Image m_aImage;
    // ImageHelper m_aImage;


    public int countNotWhitePixel(String _sFile)
        throws java.io.IOException
        {
            ImageHelper aImage = ImageHelper.createImageHelper(_sFile);
            final int nw = graphics_stuff.countNotWhitePixel(aImage);
            return nw;
        }

    public int countNotBlackPixel(String _sFile)
        throws java.io.IOException
        {
            ImageHelper aImage = ImageHelper.createImageHelper(_sFile);
            final int nw = graphics_stuff.countNotBlackPixel(aImage);
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

//    public static void main(String[] args) {
//
//        String a = helper.StringHelper.createValueString(10, 4);
//        int dummy = 1;
///*
// BorderRemover a = new BorderRemover();
//        try
//        {
//            a.createNewImageWithoutBorder(args[0], args[1]);
//        }
//        catch(java.io.IOException e)
//        {
//            System.out.println("Exception caught.");
//        }
// */
//    }
}



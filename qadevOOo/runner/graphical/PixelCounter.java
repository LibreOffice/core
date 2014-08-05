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



abstract class CountPixel
{
    protected int m_nCount = 0;
    public int getCount() {return m_nCount;}
    public abstract void count(int _nRGB);
}


class CountNotWhite extends CountPixel
{
    public CountNotWhite()
        {
        }

    public void countold(final int pixel)
        {
            // final int alpha = (pixel >> 24) & 0xff;
            final int red   = (pixel >> 16) & 0xff;
            final int green = (pixel >>  8) & 0xff;
            final int blue  = (pixel      ) & 0xff;

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


class CountNotBlack extends CountPixel
{
    public CountNotBlack()
        {
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


class graphics_stuff
{
    public static void countPixel(ImageHelper img, int _x, int _y, int _w, int _h, CountPixel _aPixelCounter)
        {
            for (int y = 0; y < _h; y++) {
                for (int x = 0; x < _w; x++) {
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



public class PixelCounter {

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


}



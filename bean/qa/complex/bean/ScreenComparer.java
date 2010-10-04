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
package complex.bean;


// import complexlib.ComplexTestCase;
import java.io.File;
import java.awt.Rectangle;
// import java.awt.BorderLayout;
import java.awt.image.BufferedImage;
import java.awt.image.PixelGrabber;
// import java.awt.event.*;
// import java.awt.Frame;
import javax.imageio.ImageIO;
// import javax.imageio.stream.FileImageOutputStream;



class ScreenComparer
{
    Rectangle m_rect;
    BufferedImage m_img1;
    BufferedImage m_img2;
    BufferedImage m_imgDiff;

    int m_diffColor;
    public ScreenComparer(int x, int y, int width, int height)
    {
        this(new Rectangle(x, y, width, height));
    }

    public ScreenComparer(Rectangle location)
    {
        m_rect = location;
        int red = 0xff;
        int alpha = 0xff;
        m_diffColor = (alpha << 24);
        m_diffColor = m_diffColor | (red << 16);
    }

    public ScreenComparer()
    {
        this(new Rectangle(0, 0, 0, 0));
    }

    public void reset()
    {
        m_rect = null;
        m_img1 = null;
        m_img2 = null;
        m_imgDiff = null;
    }

    public Rectangle getLocation()
    {
        return m_rect;
    }
    public void grabOne() throws Exception
    {
        grabOne(m_rect);
    }

    public void grabOne(Rectangle r) throws Exception
    {
        java.awt.Robot robot = new java.awt.Robot();
        m_img1 = robot.createScreenCapture(r);
    }

    public void grabTwo() throws Exception
    {
        grabTwo(m_rect);
    }

    public void grabTwo(Rectangle r) throws Exception
    {
        java.awt.Robot robot = new java.awt.Robot();
        m_img2 = robot.createScreenCapture(r);
    }

    public boolean compare() throws Exception
    {
        if (m_img1 == null || m_img2 == null)
        {
            throw new Exception("Only one image captured!");
        }
        boolean ret = true;
        int w1 = m_img1.getWidth();
        int h1 = m_img1.getHeight();
        int w2 = m_img2.getWidth();
        int h2 = m_img2.getHeight();

        if (w1 != w2 || h1 != h2)
        {
            System.out.println("### 1\n");
            //Different size. Create an image that holds both images.
            int w = w1 > w2 ? w1 : w2;
            int h = h1 > h2 ? h1 : h2;
            m_imgDiff = new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
            for (int y = 0; y < h; y ++)
            {
                for (int x = 0; x < w; x++)
                {
                    boolean bOutOfRange = false;
                    int pixel1 = 0;
                    int pixel2 = 0;
                    //get the pixel for m_img1
                    if (x < w1 && y < h1)
                    {
                        pixel1 = m_img1.getRGB(x, y);
                    }
                    else
                    {
                        bOutOfRange = true;
                    }

                    if (x < w2 && y < h2)
                    {
                        pixel2 = m_img2.getRGB(x, y);
                    }
                    else
                    {
                        bOutOfRange = true;
                    }

                    if (bOutOfRange || pixel1 != pixel2)
                    {
                        m_imgDiff.setRGB(x, y, m_diffColor);
                    }
                    else
                    {
                        m_imgDiff.setRGB(x, y, pixel1);
                    }

                }
            }
            return false;
        }

        //Images have same dimension
        int[] pixels1 = new int[w1 * h1];
        PixelGrabber pg = new PixelGrabber(
            m_img1.getSource(), 0, 0, w1, h1, pixels1, 0, w1);
        pg.grabPixels();

        int[] pixels2 = new int[w2 * h2];
        PixelGrabber pg2 = new PixelGrabber(
            m_img2.getSource(), 0, 0, w2, h2, pixels2, 0, w2);
        pg2.grabPixels();

        m_imgDiff = new BufferedImage(w1, h1, BufferedImage.TYPE_INT_ARGB);

        //First check if the the images differ.
        int lenAr = pixels1.length;
        int index = 0;
        for (index = 0; index < lenAr; index++)
        {
            if (pixels1[index] != pixels2[index])
            {
                break;
            }
        }

        //If the images are different, then create the diff image
        if (index < lenAr)
        {
            for (int y = 0; y < h1; y++)
            {
                for (int x = 0; x < w1; x++)
                {
                    int offset = y * w1 + x;
                    if (pixels1[offset] != pixels2[offset])
                    {
                        ret = ret && false;
                        m_imgDiff.setRGB(x, y, m_diffColor);
                    }
                    else
                    {
                        m_imgDiff.setRGB(x, y, pixels1[offset]);
                    }
                }
            }
        }
        return ret;
    }

    /** Writes Images to a location. The
     *  directory is determined by the java property OOoBean.Images
     *
     */
    public void writeImages() throws Exception
    {
        String imgLocation = System.getProperty("OOoBean.Images", "");
        File file_tmp = File.createTempFile("OOoBean", "", new File(imgLocation));
        File file1 = new File(file_tmp.getPath()+".png");
        file_tmp.delete();
        if (m_img1 != null)
        {
            ImageIO.write(m_img1, "png", file1);
            System.out.println("\nCompared images:");
            System.out.println("1. " + file1.getPath());
        }
        file1= null;
        file_tmp= null;
        file_tmp = File.createTempFile("OOoBean", "", new File(imgLocation));
        file1 = new File(file_tmp.getPath()+".png");
        file_tmp.delete();
        if (m_img2 != null)
        {
            ImageIO.write(m_img2, "png", file1);
            System.out.println("2. " + file1.getPath());
        }
        file1= null;
        file_tmp= null;
        file_tmp = File.createTempFile("OOoBean", "_diff", new File(imgLocation));
        file1 = new File(file_tmp.getPath()+".png");
        file_tmp.delete();
        if (m_imgDiff != null)
        {
            ImageIO.write(m_imgDiff, "png", file1);
            System.out.println("Diff image: " + file1.getPath() + "\n");
        }
    }

}


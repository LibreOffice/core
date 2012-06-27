/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.common;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import javax.imageio.ImageIO;


/**
 * Utilities related to graphics
 *
 */
public class GraphicsUtil {
    /**
     * Error tolerance for rectangle
     */
    static final double ERR_RANGLE_RECTANGLE = 0.0;

    /**
     * Error tolerance for ellipse
     */
    static final double ERR_RANGLE_ELLIPSE = 1;


    /**
     * Load a image file as buffered image
     * @param file
     * @return
     */
    public static BufferedImage loadImage(String file) {
        BufferedImage image = null;
        FileInputStream in = null;
        try {
            in = new FileInputStream(file);
            image = ImageIO.read(in);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (in != null)
                    in.close();
            } catch (IOException e) {
                // ignore
            }
        }
        return image;
    }

    /**
     * Store a buffered image in the given file
     *
     * @param image
     * @param imgFile
     */
    public static void storeImage(BufferedImage image, String imgFile) {
        File file = new File(imgFile);
        if (!file.getParentFile().exists())
            file.getParentFile().mkdirs();
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(file);
            ImageIO.write(image, FileUtil.getFileExtName(imgFile), fos);
        } catch (Exception e) {
            //
            e.printStackTrace();
        } finally {
            try {
                if (fos != null)
                    fos.close();
            } catch (IOException e) {
                //ignore
            }
        }

    }

    /**
     * Get a BufferedImage including the full current screen shot
     * @return
     */
    public static BufferedImage screenshot() {
        return screenshot(null, null);
    }

    /**
     * Get a BufferedImage including the area of current screen shot
     * @param area
     * @return
     */
    public static BufferedImage screenshot(Rectangle area) {
        return screenshot(null, area);
    }


    /**
     *  Store the screen shot as a image file
     * @param filename
     */
    public static BufferedImage screenShot(String filename) {
        return screenshot(filename, null);
    }

    /**
     * Store the specified area of the screen as a image file
     * @param filename
     * @param area
     */
    public static BufferedImage screenshot(String filename, Rectangle area) {
        // screen capture
        try {
            Robot robot = new Robot();
            if (area == null)
                area = new Rectangle(Toolkit.getDefaultToolkit().getScreenSize());

            BufferedImage capture = robot.createScreenCapture(area);
            if (filename != null)
                storeImage(capture, filename);
            return capture;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }


    /**
     * Find a rectangle in the screen.
     * Note: The rectangle must be filled with solid color and the color must be different from the background color
     *
     * @param rect the area in the screen to search
     * @param color the rectangle color.
     * @return The found rectangle's location and size. If no rectangle is
     *         found, return null
     */
    public static Rectangle findRectangle(Rectangle rect, int color) {
        return findRectangle(screenshot(rect), color);
    }

    /**
     * find a rectangle in an image
     * Note: The rectangle must be filled with solid color and the color must be different from the background color
     * @param src
     * @param color
     *            the rectangle color.
     * @return The found rectangle's location and size. If no rectangle is
     *         found, return null
     */
    public static Rectangle findRectangle(BufferedImage src, int color) {
        Rectangle re = new Rectangle();

        BufferedImage dst = new BufferedImage(src.getWidth(), src.getHeight(),
                BufferedImage.TYPE_INT_ARGB);

        for (int x = 0; x < dst.getWidth(); x++) {
            for (int y = 0; y < dst.getHeight(); y++) {
                dst.setRGB(x, y, 0xFFFFFFFF);
            }
        }

        Graphics g = dst.getGraphics();
        g.setColor(Color.black);

        int sx = -1, sy = 0, ex = 0, ey = 0;
        for (int x = 0; x < src.getWidth(); x++) {
            for (int y = 0; y < src.getHeight(); y++) {
                int rgbSrc = src.getRGB(x, y);
                if (rgbSrc == color) {
                    if (sx == -1) {
                        sx = x;
                        sy = y;
                    }
                    ex = x;
                    ey = y;
                }
            }
        }

        g.fillRect(sx, sy, ex - sx + 1, ey - sy + 1);
        // g.fillRect(0, 0, dst.getWidth(), dst.getHeight());
        int perimeter = 2 * (ex - sx + ey - sy);
        int errMax = (int)(perimeter * ERR_RANGLE_RECTANGLE);

        if (!(detect(src, color, dst, 0xff000000, errMax) && detect(dst, 0xff000000,
                src, color, errMax)))
            return null;
        re.setBounds(sx, sy, ex - sx, ey - sy);
        if (re.width < 2 || re.height < 2) {
            return null;
        }
        return re;
    }


    protected static boolean detect(BufferedImage src, int colorSrc,
            BufferedImage dst, int colorDst, double errMax) {
        int errCount = 0;
        for (int x = 0; x < src.getWidth(); x++) {
            for (int y = 0; y < src.getHeight(); y++) {
                int rgbSrc = src.getRGB(x, y);
                if (rgbSrc == colorSrc) {
                    int rgbDst = dst.getRGB(x, y);
                    if (!(rgbDst == colorDst)) {
                        errCount++;
                    }
                }
            } // end for y
        }// end for x
        // System.out.println(errCount);
        if (errCount <= errMax)
            return true;
        return false;
    }

}

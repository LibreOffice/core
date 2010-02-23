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
package complex;


import complexlib.ComplexTestCase;
import java.io.*;
import java.awt.Rectangle;
import java.awt.Insets;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.awt.image.PixelGrabber;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.event.*;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.Robot;
import java.awt.event.KeyEvent;
import java.awt.Button;
import javax.imageio.ImageIO;
import javax.imageio.stream.FileImageOutputStream;
import com.sun.star.comp.beans.OOoBean;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.XTextDocument;

import java.awt.*;

public class OOoBeanTest extends ComplexTestCase
{

    public String[] getTestMethodNames()
    {
        // TODO think about trigger of sub-tests from outside
        return new String[]
        {
              "test1",
              "test2",
              "test3",
              "test4",
              "test5",
              "test6",
              "test6a",
              "test7",
              "test8"
        };
    }

    /** For X-Windows we need to prolong the time between painting windows. Because
        it takes longer than on Windows.
    */
    int getSleepTime(int time)
    {
        int ret = time;
        if (isWindows() == false)
            return time * 5;
        return time;
    }

    /** If it cannot be determined if we run on Windows then we assume
        that we do not.
    */
    boolean isWindows()
    {
        boolean ret = false;
        String os = System.getProperty("os.name");
        if (os != null)
        {
            os = os.trim();
            if (os.indexOf("Win") == 0)
                ret = true;
        }
        return ret;
    }

    public String getText(OOoBean bean) throws Exception
    {
        com.sun.star.frame.XModel model = (com.sun.star.frame.XModel)bean.getDocument();
        com.sun.star.text.XTextDocument myDoc =
            (XTextDocument) UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class,
                                                      model);
        com.sun.star.text.XText xText = myDoc.getText();
        return xText.getString();
    }

    /** 1.Create a Java frame
     *  2.Add OOoBean (no document loaded yet)
     *  3.Show frame
     *  4.Load document
     */
    public void test1() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100 ,100, 500 ,400, false);
            f.setText("OOoBean test.");
            Thread.sleep(1000);
        }
        finally
        {
            if (f != null)
                f.dispose();
        }
    }

    /** Sizing, painting
     */
    public void test2() throws Exception
    {
        WriterFrame f = null;
        ScreenComparer capturer = null;
        try
        {
            f = new WriterFrame(100, 100, 500,500, false);
            if (f.checkUnoFramePosition() == false)
                failed("Sizing error: Client are of Java frame does not match the UNO window.", true);
            capturer = new ScreenComparer(100, 100, 500, 500);

            //Minimize Window and back
            f.goToStart();
            f.pageDown();
            Thread.sleep(1000);
            for (int i = 0; i < 3; i++)
            {
                capturer.reset();
                capturer.grabOne(f.getClientArea());
                f.setExtendedState(Frame.ICONIFIED);
                Thread.sleep(getSleepTime(200));
                if (f.checkUnoFramePosition() == false)
                    failed("Sizing error: Frame was iconified.", true);
                f.setExtendedState(Frame.NORMAL);
                Thread.sleep(getSleepTime(200));
                if (f.checkUnoFramePosition() == false)
                    failed("Sizing error: Frame size set back to normal after it was iconified.", true);
                capturer.grabTwo(f.getClientArea());
                if (capturer.compare() == false)
                {
                    failed("Painting error: Minimize (iconify) frame and back to normal size.", true);
                    capturer.writeImages();
                }
            }

            //Maximize Window and back to normal
            for (int i = 0; i < 3; i++)
            {
                capturer.reset();
                capturer.grabOne(f.getClientArea());
                f.setExtendedState(Frame.MAXIMIZED_BOTH);
                Thread.sleep(getSleepTime(200));
                if (f.checkUnoFramePosition() == false)
                    failed("Sizing error: Frame maximized.", true);
                f.setExtendedState(Frame.NORMAL);
                Thread.sleep(getSleepTime(200));
                if (f.checkUnoFramePosition() == false)
                    failed("Sizing error: Frame set from maximized to normal.", true);
                capturer.grabTwo(f.getClientArea());
                if (capturer.compare() == false)
                {
                    failed("Painting error: Maximize frame and back to normal size", true);
                    capturer.writeImages();
                }
            }

            //move Window top left
            capturer.reset();
            capturer.grabOne(f.getClientArea());
            Rectangle oldPosition = f.getBounds();
            f.setBounds(0, 0, oldPosition.width, oldPosition.height);
            Thread.sleep(getSleepTime(200));
            if (f.checkUnoFramePosition() == false)
                failed("Sizing error: Frame moved.", true);

            capturer.grabTwo(f.getClientArea());
            if (capturer.compare() == false)
            {
                failed("Painting error: Move frame to a different position.", true);
                capturer.writeImages();
            }

            //move Window down
            Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
            int maxY = dim.height - f.getBounds().height;

            int curY = 0;
            while (curY < maxY)
            {
                capturer.reset();
                capturer.grabOne(f.getClientArea());
                oldPosition = f.getBounds();
                f.setBounds(0, curY, oldPosition.width, oldPosition.height);
                capturer.grabTwo(f.getClientArea());
                if (capturer.compare() == false)
                {
                    failed("Painting error: Move frame to a different position.", true);
                    capturer.writeImages();
                }
                curY+= 50;
                Thread.sleep(getSleepTime(200));
            }

            //obscure the window and make it visible again

            oldPosition = f.getBounds();

            Rectangle pos = new Rectangle(oldPosition.x - 50, oldPosition.y - 50,
                                          oldPosition.width, oldPosition.height);
            Frame coverFrame = new Frame();
            coverFrame.setBounds(pos);
            capturer.reset();
            capturer.grabOne(f.getClientArea());

            for (int i = 0; i < 3; i++)
            {
                coverFrame.setVisible(true);
                Thread.sleep(getSleepTime(200));
                f.toFront();
                Thread.sleep(getSleepTime(200));
                if (f.checkUnoFramePosition() == false)
                    failed("Sizing error: Frame moved from back to front.", true);

                capturer.grabTwo(f.getClientArea());
                if (capturer.compare() == false)
                {
                    failed("Painting error: Move frame to back and to front.", true);
                    capturer.writeImages();
                }
            }

            coverFrame.dispose();
        }
        finally
        {
            if (f != null)
                f.dispose();
        }
    }

    /**
       1. Create a OOoBean
       2. Load a document
       3. Create Frame (do not show yet)
       4. Add OOoBean to Frame
       5. Show Frame
    */
    public void test3() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100, 100, 500, 300, true);
            if (f.checkUnoFramePosition() == false)
                failed("Sizing error.", true);

        }
        finally
        {
            if (f != null)
                f.dispose();
        }
    }

    /** Test repeated OOoBean.aquireSystemWindow and OOoBean.releaseSystemWindow
     * calls.
     */
    public void test4() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100, 100, 500, 300, false);
            OOoBean b = f.getBean();
            for (int i = 0; i < 100; i++)
            {
                b.releaseSystemWindow();
                b.aquireSystemWindow();
            }
            if (f.checkUnoFramePosition() == false)
                failed("Sizing error.", true);
        }
        finally
        {
            if (f != null)
                f.dispose();
            if (isWindows() == false)
                Thread.sleep(10000);
        }
    }

    /** Adding and removing the bean to a Java frame multiple times.
     * Test painting and sizing.
     */
    public void test5() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100, 100, 500, 400, false);
            f.goToStart();
            f.pageDown();
            Thread.sleep(1000);

            ScreenComparer capturer = new ScreenComparer(100,100,500,400);
            capturer.grabOne();
            for (int i = 0; i < 100; i++)
            {
                f.removeOOoBean();
                f.addOOoBean();
            }

            f.goToStart();
            f.pageDown();
            Thread.sleep(getSleepTime(200));
            capturer.grabTwo();

            if (capturer.compare() == false)
            {
                failed("Painting error: adding and removing OOoBean " +
                       "repeatedly to java.lang.Frame.", true);
                capturer.writeImages();
            }

            if (f.checkUnoFramePosition() == false)
                failed("Sizing error.", true);

        }
        finally
        {
            if (f != null)
                f.dispose();
            if (isWindows() == false)
                Thread.sleep(10000);
        }
    }


    /** Test focus  (i49454). After repeatedly adding and removing the bean to a window
     * it should still be possible to enter text in the window. This does not
     * work all the time on Windows. This is probably a timing problem. When using
     * Thread.sleep (position #1) then it should work.
     */
    public void test6() throws Exception
    {
        for (int j = 0; j < 10; j++)
        {
            final OOoBean bean = new OOoBean();
            java.awt.Frame frame = null;
            bean.setOOoCallTimeOut(10000);
            try {
                frame = new java.awt.Frame("OpenOffice.org Demo");
                frame.add(bean, BorderLayout.CENTER);
                frame.pack();
                frame.setSize(600,300);
                frame.show();
                bean.loadFromURL("private:factory/swriter", null);
                // #1
                Thread.sleep(1000);

                StringBuffer buf = new StringBuffer(1000);
                for (int i = 0; i < 1; i++)
                {
//                    Thread.sleep(1000);
                    bean.releaseSystemWindow();
                    frame.remove(bean);
//                    frame.validate();
//                    Thread.sleep(1000);
                    frame.add(bean, BorderLayout.CENTER);
                    bean.aquireSystemWindow();
//                    frame.validate();
                }

                if (isWindows() == false)
                    Thread.sleep(5000);

                Robot roby = new Robot();
                roby.keyPress(KeyEvent.VK_H);
                roby.keyRelease(KeyEvent.VK_H);
                buf.append("h");

                String s = getText(bean);
                if ( ! s.equals(buf.toString()))
                {
                    failed("Focus error: After removing and adding the bean, the" +
                           "office window does not receive keyboard input.\n" +
                           "Try typing in the window, you've got 30s!!! This " +
                           "test may not work with Linux/Solaris", true);
                    Thread.sleep(30000);
                    break;
                }
                else
                {
                    Thread.sleep(2000);
                }

            } finally {
                bean.stopOOoConnection();
                frame.dispose();
            }
        }
    }

    /** Tests focus problem just like test6, but the implementation is a little
     * different. The bean is added and removed from withing the event dispatch
     * thread. Using Thread.sleep at various points (#1, #2, #3) seems to workaround
     * the problem.
     */
    public void test6a() throws Exception
    {
        for (int j = 0; j < 50; j++)
        {
            final OOoBean bean = new OOoBean();
            final java.awt.Frame frame = new Frame("Openoffice.org");
            bean.setOOoCallTimeOut(10000);

            try {
                frame.add(bean, BorderLayout.CENTER);
                frame.pack();
                frame.setSize(600,400);
                frame.show();
                bean.loadFromURL("private:factory/swriter", null);
                frame.validate();
                // #1
                Thread.sleep(1000);
                StringBuffer buf = new StringBuffer(1000);
                int i = 0;

                for (; i < 1; i++)
                {
                EventQueue q = Toolkit.getDefaultToolkit().getSystemEventQueue();
                q.invokeAndWait( new Runnable() {
                        public void run() {
                            try {

                            bean.releaseSystemWindow();
                            frame.remove(bean);
                            frame.validate();

                            } catch (Exception e) {
                                e.printStackTrace();
                            }

                            }
                        });
                // #2
                Thread.sleep(1000);
                q.invokeAndWait( new Runnable() {
                        public void run() {
                            try {

                            frame.add(bean, BorderLayout.CENTER);
                            bean.aquireSystemWindow();
                            frame.validate();
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                            }
                        });

                // #3
                Thread.sleep(1000);
                }

                if (isWindows() == false)
                    Thread.sleep(5000);

                Robot roby = new Robot();
                roby.mouseMove(300, 200);
                roby.waitForIdle();
                roby.mousePress(InputEvent.BUTTON1_MASK);
                roby.waitForIdle();
                roby.mouseRelease(InputEvent.BUTTON1_MASK);
                roby.waitForIdle();
                roby.keyPress(KeyEvent.VK_H);
                roby.waitForIdle();
                roby.keyRelease(KeyEvent.VK_H);
                roby.waitForIdle();

                buf.append("h");
                Thread.sleep(1000);
                String s = getText(bean);
                System.out.println(" getText: " + s);
                if ( ! s.equals(buf.toString()))
                {
                    roby.mousePress(InputEvent.BUTTON1_MASK);
                    roby.waitForIdle();
                    roby.mouseRelease(InputEvent.BUTTON1_MASK);
                    roby.waitForIdle();
                    roby.keyPress(KeyEvent.VK_H);
                    roby.waitForIdle();
                    roby.keyRelease(KeyEvent.VK_H);
                    roby.waitForIdle();

                    String sH = "h";
                    Thread.sleep(1000);
                    String s2 = getText(bean);

                    if ( ! sH.equals(s2))
                    {
                        failed("Focus error: After removing and adding the bean, the" +
                               "office window does not receive keyboard input.\n" +
                               "Try typing in the window, you've got 30s!!! This " +
                               "test may not work with Linux/Solaris", true);
                        System.out.println("j: " + j + "   i: " + i);
                        Thread.sleep(30000);
                        break;
                    }
                }
                else
                {
                    //                   Thread.sleep(2000);
                }

            } finally {
                bean.stopOOoConnection();
                frame.dispose();
            }
        }
    }

    /** Repeatedly loading a document in one and the same OOoBean instance.
     */
    public void test7() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100 ,100, 500 ,400, false);
            String text = "OOoBean test.";

            for (int i = 0; i < 10; i++)
            {
                f.getBean().clear();
                f.getBean().loadFromURL("private:factory/swriter", null);
                f.setText(text);
                f.goToStart();
                f.validate();

                if (text.equals(f.getText()) == false)
                    failed("Repeated loading of a document failed.");
                Thread.sleep(1000);
            }
        }
        finally
        {
            if (f != null)
                f.dispose();
        }
    }

    /** Using multiple instances of OOoBean at the same time
     */
    public void test8() throws Exception
    {
        OOoBean bean1 = new OOoBean();
        BeanPanel bp1 = new BeanPanel(bean1);
        OOoBean bean2 = new OOoBean();
        BeanPanel bp2 = new BeanPanel(bean2);
        OOoBean bean3 = new OOoBean();
        BeanPanel bp3 = new BeanPanel(bean3);
        OOoBean bean4 = new OOoBean();
        BeanPanel bp4 = new BeanPanel(bean4);

        try
        {
            Frame f = new Frame("OOoBean example with several instances");
            f.setLayout(new GridBagLayout());
            GridBagConstraints c = new GridBagConstraints();
            c.fill = GridBagConstraints.HORIZONTAL;
            c.weightx = 0.5;

            c.insets = new Insets(0, 0, 0, 10);
            c.gridx = 0;
            c.gridy = 0;
            f.add(bp1, c);

            c.gridx = 1;
            c.insets = new Insets(0, 0, 0, 0);
            f.add(bp2, c);

            c.gridx = 0;
            c.gridy = 1;
            c.insets = new Insets(10, 0, 0, 10);
            f.add(bp3, c);

            c.gridx = 1;
            c.gridy = 1;
            c.insets = new Insets(10, 0, 0, 0);
            f.add(bp4, c);

            f.pack();
            f.setBounds(0, 0, 1000, 600);
            f.setVisible(true);
            try {
            bean1.loadFromURL("private:factory/swriter", null);
            bean2.loadFromURL("private:factory/swriter", null);
            bean3.loadFromURL("private:factory/swriter", null);
            bean4.loadFromURL("private:factory/swriter", null);
            } catch( Exception e)
            {
                e.printStackTrace();
            }
            f.validate();

            Thread.sleep(10000);
        }
        finally
        {
            bean1.stopOOoConnection();
            bean2.stopOOoConnection();
            bean3.stopOOoConnection();
            bean4.stopOOoConnection();
        }
    }

    class BeanPanel extends Panel
    {
        public BeanPanel(OOoBean b)
        {
            setLayout(new BorderLayout());
            add(b, BorderLayout.CENTER);
        }
        public Dimension getPreferredSize()
        {
            Container c = getParent();
            return new Dimension(200, 200);
        }
    }

}



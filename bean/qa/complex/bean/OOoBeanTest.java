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
package complex.bean;


// import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import java.awt.event.*;
import java.awt.event.KeyEvent;
import com.sun.star.comp.beans.OOoBean;
import com.sun.star.uno.UnoRuntime;

import java.awt.*;

// import org.junit.After;
import org.junit.AfterClass;
// import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

class PrivateLocalOfficeConnection extends com.sun.star.comp.beans.LocalOfficeConnection
{
    public PrivateLocalOfficeConnection(com.sun.star.uno.XComponentContext xContext)
    {
        super(xContext);
    }
}

public class OOoBeanTest
{

//    public String[] getTestMethodNames()
//    {
//        // TODO think about trigger of sub-tests from outside
//        return new String[]
//        {
//              "test1",
//              "test2",
//              "test3",
//              "test4",
//              "test5",
//              "test6",
//              "test6a",
//              "test7",
//              "test8"
//        };
//    }

    /** For X-Windows we need to prolong the time between painting windows. Because
        it takes longer than on Windows.
    */
    private int getSleepTime(int time)
    {
        if (!isWindows())
        {
            return time * 5;
        }
        return time;
    }

    /** If it cannot be determined if we run on Windows then we assume
        that we do not.
    */
    private boolean isWindows()
    {
        boolean ret = false;
        String os = System.getProperty("os.name");
        if (os != null)
        {
            os = os.trim();
            if (os.toLowerCase().indexOf("win") == 0)
            {
                ret = true;
            }
        }
        return ret;
    }

    private String getText(OOoBean bean) throws Exception
    {
        com.sun.star.frame.XModel model = bean.getDocument();
        com.sun.star.text.XTextDocument myDoc =
            UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class, model);
        com.sun.star.text.XText xText = myDoc.getText();
        return xText.getString();
    }

    /** 1.Create a Java frame
     *  2.Add OOoBean (no document loaded yet)
     *  3.Show frame
     *  4.Load document
     * @throws Exception
     */
    @Test public void test1() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100 ,100, 500 ,400, false, connection.getComponentContext());
            f.setText("OOoBean test.");
            Thread.sleep(1000);
        }
        finally
        {
            if (f != null)
            {
                f.dispose();
            }
        }
    }

    /** Sizing, painting
     * @throws Exception
     */
    @Test public void test2() throws Exception
    {
        WriterFrame f = null;
        ScreenComparer capturer = null;
        try
        {
            f = new WriterFrame(100, 100, 500,500, false, connection.getComponentContext());
            if (!f.checkUnoFramePosition())
            {
                fail("Sizing error: Client are of Java frame does not match the UNO window.");
            }
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
                if (!f.checkUnoFramePosition())
                {
                    fail("Sizing error: Frame was iconified.");
                }
                f.setExtendedState(Frame.NORMAL);
                Thread.sleep(getSleepTime(200));
                if (!f.checkUnoFramePosition())
                {
                    fail("Sizing error: Frame size set back to normal after it was iconified.");
                }
                capturer.grabTwo(f.getClientArea());
                if (!capturer.compare())
                {
                    fail("Painting error: Minimize (iconify) frame and back to normal size.");
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
                if (!f.checkUnoFramePosition())
                {
                    fail("Sizing error: Frame maximized.");
                }
                f.setExtendedState(Frame.NORMAL);
                Thread.sleep(getSleepTime(200));
                if (!f.checkUnoFramePosition())
                {
                    fail("Sizing error: Frame set from maximized to normal.");
                }
                capturer.grabTwo(f.getClientArea());
                if (!capturer.compare())
                {
                    fail("Painting error: Maximize frame and back to normal size");
                    capturer.writeImages();
                }
            }

            //move Window top left
            capturer.reset();
            capturer.grabOne(f.getClientArea());
            Rectangle oldPosition = f.getBounds();
            f.setBounds(0, 0, oldPosition.width, oldPosition.height);
            Thread.sleep(getSleepTime(200));
            if (!f.checkUnoFramePosition())
            {
                fail("Sizing error: Frame moved.");
            }

            capturer.grabTwo(f.getClientArea());
            if (!capturer.compare())
            {
                fail("Painting error: Move frame to a different position.");
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
                if (!capturer.compare())
                {
                    fail("Painting error: Move frame to a different position.");
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
                if (!f.checkUnoFramePosition())
                {
                    fail("Sizing error: Frame moved from back to front.");
                }

                capturer.grabTwo(f.getClientArea());
                if (!capturer.compare())
                {
                    fail("Painting error: Move frame to back and to front.");
                    capturer.writeImages();
                }
            }

            coverFrame.dispose();
        }
        finally
        {
            if (f != null)
            {
                f.dispose();
            }
        }
    }

    /**
       1. Create a OOoBean
       2. Load a document
       3. Create Frame (do not show yet)
       4. Add OOoBean to Frame
       5. Show Frame
     * @throws Exception
     */
    @Test public void test3() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100, 100, 500, 300, true, connection.getComponentContext());
            if (!f.checkUnoFramePosition())
            {
                fail("Sizing error.");
            }

        }
        finally
        {
            if (f != null)
            {
                f.dispose();
            }
        }
    }

    /** Test repeated OOoBean.aquireSystemWindow and OOoBean.releaseSystemWindow
     * calls.
     * @throws Exception
     */
    @Test public void test4() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100, 100, 500, 300, false, connection.getComponentContext());
            OOoBean b = f.getBean();
            for (int i = 0; i < 100; i++)
            {
                b.releaseSystemWindow();
                b.aquireSystemWindow();
            }
            if (!f.checkUnoFramePosition())
            {
                fail("Sizing error.");
            }
        }
        finally
        {
            if (f != null)
            {
                f.dispose();
            }
            if (!isWindows())
            {
                Thread.sleep(10000);
            }
        }
    }

    /** Adding and removing the bean to a Java frame multiple times.
     * Test painting and sizing.
     * @throws Exception
     */
    @Test public void test5() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100, 100, 500, 400, false, connection.getComponentContext());
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

            if (!capturer.compare())
            {
                fail("Painting error: adding and removing OOoBean " +
                       "repeatedly to java.lang.Frame.");
                capturer.writeImages();
            }

            if (!f.checkUnoFramePosition())
            {
                fail("Sizing error.");
            }

        }
        finally
        {
            if (f != null)
            {
                f.dispose();
            }
            if (!isWindows())
            {
                Thread.sleep(10000);
            }
        }
    }


    /** Test focus  (i49454). After repeatedly adding and removing the bean to a window
     * it should still be possible to enter text in the window. This does not
     * work all the time on Windows. This is probably a timing problem. When using
     * Thread.sleep (position #1) then it should work.
     * @throws Exception
     */
    @Test public void test6() throws Exception
    {
        for (int j = 0; j < 10; j++)
        {
            final OOoBean bean = new OOoBean(new PrivateLocalOfficeConnection(connection.getComponentContext()));
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
                    bean.releaseSystemWindow();
                    frame.remove(bean);
                    frame.add(bean, BorderLayout.CENTER);
                    bean.aquireSystemWindow();
                }

                if (!isWindows())
                {
                    Thread.sleep(5000);
                }

                Robot roby = new Robot();
                roby.keyPress(KeyEvent.VK_H);
                roby.keyRelease(KeyEvent.VK_H);
                buf.append("h");

                String s = getText(bean);
                if ( ! s.equals(buf.toString()))
                {
                    fail("Focus error: After removing and adding the bean, the" +
                           "office window does not receive keyboard input.\n" +
                           "Try typing in the window, you've got 30s!!! This " +
                           "test may not work with Linux/Solaris");
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
     * @throws Exception
     */
    @Test public void test6a() throws Exception
    {
        for (int j = 0; j < 50; j++)
        {
            final OOoBean bean = new OOoBean(new PrivateLocalOfficeConnection(connection.getComponentContext()));
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

                if (!isWindows())
                {
                    Thread.sleep(5000);
                }

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

                buf.append('h');
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
                        fail("Focus error: After removing and adding the bean, the" +
                               "office window does not receive keyboard input.\n" +
                               "Try typing in the window, you've got 30s!!! This " +
                               "test may not work with Linux/Solaris");
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
     * @throws Exception
     */
    @Test public void test7() throws Exception
    {
        WriterFrame f = null;
        try
        {
            f = new WriterFrame(100 ,100, 500 ,400, false, connection.getComponentContext());
            String text = "OOoBean test.";

            for (int i = 0; i < 10; i++)
            {
                f.getBean().clear();
                f.getBean().loadFromURL("private:factory/swriter", null);
                f.setText(text);
                f.goToStart();
                f.validate();

                if (!text.equals(f.getText()))
                {
                    fail("Repeated loading of a document failed.");
                }
                Thread.sleep(1000);
            }
        }
        finally
        {
            if (f != null)
            {
                f.dispose();
            }
        }
    }

    /** Using multiple instances of OOoBean at the same time
     * @throws Exception
     */

    @Test public void test8() throws Exception
    {
        OOoBean bean1 = new OOoBean(new PrivateLocalOfficeConnection(connection.getComponentContext()));
        BeanPanel bp1 = new BeanPanel(bean1);
        OOoBean bean2 = new OOoBean(new PrivateLocalOfficeConnection(connection.getComponentContext()));
        BeanPanel bp2 = new BeanPanel(bean2);
        OOoBean bean3 = new OOoBean(new PrivateLocalOfficeConnection(connection.getComponentContext()));
        BeanPanel bp3 = new BeanPanel(bean3);
        OOoBean bean4 = new OOoBean(new PrivateLocalOfficeConnection(connection.getComponentContext()));
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
            return new Dimension(200, 200);
        }
    }


    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();


}



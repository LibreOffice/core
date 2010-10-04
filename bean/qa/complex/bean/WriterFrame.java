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


// import com.sun.star.comp.beans.LocalOfficeConnection;
import com.sun.star.uno.XComponentContext;
import java.awt.Rectangle;
import java.awt.Insets;
import java.awt.BorderLayout;
import com.sun.star.comp.beans.OOoBean;
import com.sun.star.uno.UnoRuntime;



class WriterFrame extends java.awt.Frame
{
    com.sun.star.comp.beans.OOoBean m_bean;
    final static String m_sDocURL = "private:factory/swriter";

    /**
      @param loadBeforeVisible
          the OOoBean is added to the frame before it is displayable. Then the Java Frame does
          not have a native window peer yet.
     */
    public WriterFrame(int x, int y, int width, int height, boolean loadBeforeVisible, XComponentContext _xConn) throws Exception
    {

        try
        {
            if (loadBeforeVisible == false)
            {
                m_bean = new com.sun.star.comp.beans.OOoBean(new PrivateLocalOfficeConnection(_xConn));
                add(m_bean, BorderLayout.CENTER);
                pack();
                setBounds(x, y, width, height);
                setVisible(true);
                m_bean.loadFromURL(m_sDocURL, null);
                validate();
            }
            else
            {
                m_bean = new com.sun.star.comp.beans.OOoBean(new PrivateLocalOfficeConnection(_xConn));
                m_bean.loadFromURL(m_sDocURL, null);
                add(m_bean, BorderLayout.CENTER);
                pack();
                setBounds(x, y, width, height);
                setVisible(true);
                m_bean.aquireSystemWindow();
            }
        }
        catch (Exception e)
        {
            System.out.println("Exception caught: " + e.getMessage());
        }
    }

    public WriterFrame() throws Exception
    {
        this(0, 0, 800, 400, false, null);
    }

    public void setText(String s) throws Exception
    {
        com.sun.star.frame.XModel model = (com.sun.star.frame.XModel)m_bean.getDocument();
        com.sun.star.text.XTextDocument myDoc =
            UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class, model);
        com.sun.star.text.XText xText = myDoc.getText();
        com.sun.star.text.XTextCursor xTCursor = xText.createTextCursor();
        //inserting some Text
        xText.insertString( xTCursor, s, false );
    }

    public String getText() throws Exception
    {
        com.sun.star.frame.XModel model = (com.sun.star.frame.XModel)m_bean.getDocument();
        com.sun.star.text.XTextDocument myDoc =
            UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class, model);
        com.sun.star.text.XText xText = myDoc.getText();
        return xText.getString();
    }

    @Override
    public void dispose() {
        m_bean.stopOOoConnection();
        setVisible(false);
        super.dispose();
    }

    OOoBean getBean()
    {
        return m_bean;
    }

    /** Makes sure the document is displayed at the beginning.
     * This is important for comparing screenshots.
     */
    public void goToStart() throws Exception
    {
        com.sun.star.frame.XModel xModel = (com.sun.star.frame.XModel)m_bean.getDocument();
        com.sun.star.frame.XController xController = xModel.getCurrentController();

        com.sun.star.text.XTextViewCursorSupplier xVCSupplier =
            UnoRuntime.queryInterface(com.sun.star.text.XTextViewCursorSupplier.class, xController);

        com.sun.star.text.XTextViewCursor xTViewCursor = xVCSupplier.getViewCursor ( );
        xTViewCursor.gotoStart(false);
    }


    public void pageDown() throws Exception
    {
        com.sun.star.frame.XModel xModel = (com.sun.star.frame.XModel)m_bean.getDocument();
        com.sun.star.frame.XController xController = xModel.getCurrentController();

        com.sun.star.text.XTextViewCursorSupplier xVCSupplier =
            UnoRuntime.queryInterface(com.sun.star.text.XTextViewCursorSupplier.class, xController);
        com.sun.star.text.XTextViewCursor xTViewCursor = xVCSupplier.getViewCursor ( );
        com.sun.star.view.XScreenCursor xScreenCursor =
            UnoRuntime.queryInterface(com.sun.star.view.XScreenCursor.class, xTViewCursor);
        xScreenCursor.screenDown();
    }

    public Rectangle getClientArea()
    {

        Insets i = getInsets();
        Rectangle r = getBounds();
        Rectangle rc = new Rectangle(r.x + i.left, r.y + i.top,
                                     r.width - i.left - i.right,
                                     r.height - i.top - i.bottom);
        return rc;
    }

    public Rectangle getUnoFramePosition() throws Exception
    {
        com.sun.star.awt.XWindow win = m_bean.getFrame().getContainerWindow();
        com.sun.star.awt.Rectangle rect = win.getPosSize();
        return new Rectangle(rect.X, rect.Y, rect.Width, rect.Height);
    }

    /** After resizing the Java frame, the UNO window shall be resized
     * as well, which is checked by this method.
     */
    public boolean checkUnoFramePosition() throws Exception
    {
        Rectangle client = getClientArea();
        Rectangle unoWin = getUnoFramePosition();

        if (client.x != unoWin.x
            || client.y != unoWin.y
            || client.width != unoWin.width
            || client.height != unoWin.height)
        {
            System.out.println("\nPosition of client are of Java frame does not match the postion" +
                               "of the UNO window. These are the values of Java frame, followed by" +
                               "the UNO window: ");
            System.out.println(client);
            System.out.println(unoWin);
            System.out.println("");
            return false;
        }

        return true;
    }

    public void removeOOoBean() throws Exception
    {
        //OOoBean.releaseSystemWindow need not be called because
        //LocalOfficeWindow overrides removeNotify.
        //However because of bt4745222 which was fixed in 1.4.2_04,
        //this is very very slow. The workaround is use releaseSystemWindow
        //beforehand.
        m_bean.releaseSystemWindow();
        remove(m_bean);
    }

    public void addOOoBean() throws Exception
    {
        add(m_bean, BorderLayout.CENTER);
        m_bean.aquireSystemWindow();
        validate();
    }

}


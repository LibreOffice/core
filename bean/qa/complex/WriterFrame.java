/*************************************************************************
 *
 *  $RCSfile: WriterFrame.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2005-09-23 11:54:23 $
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
package complex;


import complexlib.ComplexTestCase;
import java.awt.Rectangle;
import java.awt.Insets;
import java.awt.BorderLayout;
import java.awt.event.*;
import java.awt.Frame;
import java.awt.Dimension;
import com.sun.star.comp.beans.OOoBean;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.XTextDocument;


class WriterFrame extends java.awt.Frame
{
    com.sun.star.comp.beans.OOoBean m_bean;
    String m_sDocURL = "private:factory/swriter";

    /**
      @param loadBeforeVisible
          the OOoBean is added to the frame before it is displayable. Then the Java Frame does
          not have a native window peer yet.
     */
    public WriterFrame(int x, int y, int width, int height, boolean loadBeforeVisible) throws Exception
    {

        if (loadBeforeVisible == false)
        {
            m_bean = new com.sun.star.comp.beans.OOoBean();
            add(m_bean, BorderLayout.CENTER);
            pack();
            setBounds(x, y, width, height);
            setVisible(true);
            m_bean.loadFromURL(m_sDocURL, null);
            validate();
        }
        else
        {
            m_bean = new com.sun.star.comp.beans.OOoBean();
            m_bean.loadFromURL(m_sDocURL, null);
            add(m_bean, BorderLayout.CENTER);
            pack();
            setBounds(x, y, width, height);
            setVisible(true);
            m_bean.aquireSystemWindow();
        }
    }

    public WriterFrame() throws Exception
    {
        this(0, 0, 800, 400, false);
    }

    public void setText(String s) throws Exception
    {
        com.sun.star.frame.XModel model = (com.sun.star.frame.XModel)m_bean.getDocument();
        com.sun.star.text.XTextDocument myDoc =
            (XTextDocument) UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class,
                                                      model);
        com.sun.star.text.XText xText = myDoc.getText();
        com.sun.star.text.XTextCursor xTCursor = xText.createTextCursor();
        //inserting some Text
        xText.insertString( xTCursor, s, false );
    }

    public String getText() throws Exception
    {
        com.sun.star.frame.XModel model = (com.sun.star.frame.XModel)m_bean.getDocument();
        com.sun.star.text.XTextDocument myDoc =
            (XTextDocument) UnoRuntime.queryInterface(com.sun.star.text.XTextDocument.class,
                                                      model);
        com.sun.star.text.XText xText = myDoc.getText();
        return xText.getString();
    }

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
            (com.sun.star.text.XTextViewCursorSupplier) UnoRuntime.queryInterface (
               com.sun.star.text.XTextViewCursorSupplier.class, xController );

        com.sun.star.text.XTextViewCursor xTViewCursor = xVCSupplier.getViewCursor ( );
        xTViewCursor.gotoStart(false);
    }


    public void pageDown() throws Exception
    {
        com.sun.star.frame.XModel xModel = (com.sun.star.frame.XModel)m_bean.getDocument();
        com.sun.star.frame.XController xController = xModel.getCurrentController();

        com.sun.star.text.XTextViewCursorSupplier xVCSupplier =
            (com.sun.star.text.XTextViewCursorSupplier) UnoRuntime.queryInterface (
               com.sun.star.text.XTextViewCursorSupplier.class, xController );
        com.sun.star.text.XTextViewCursor xTViewCursor = xVCSupplier.getViewCursor ( );
        com.sun.star.view.XScreenCursor xScreenCursor =
            (com.sun.star.view.XScreenCursor) UnoRuntime.queryInterface (
               com.sun.star.view.XScreenCursor.class, xTViewCursor );
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


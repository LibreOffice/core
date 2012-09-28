/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.XMessageBoxFactory;
import com.sun.star.awt.XVclWindowPeer;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.util.XCloseable;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;



public class MessageBox  {

    protected XComponentContext m_xContext = null;
    protected com.sun.star.lang.XMultiComponentFactory m_xMCF;

    /** Creates a new instance of MessageBox */
    public MessageBox(XComponentContext _xContext, XMultiComponentFactory _xMCF){
        m_xContext = _xContext;
        m_xMCF = _xMCF;
    }

    public static void main(String args[]) {
        XComponent xComp = null;
        try {
            XComponentContext xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            if(xContext != null )
                System.out.println("Connected to a running office ...");
            XMultiComponentFactory xMCF = xContext.getServiceManager();

            MessageBox oMessageBox = new MessageBox(xContext, xMCF);

            //load default text document to get an active frame
            xComp = oMessageBox.createDefaultTextDocument();

            XWindowPeer xWindowPeer = oMessageBox.getWindowPeerOfFrame(xComp);
            if (xWindowPeer != null) {
                XVclWindowPeer xVclWindowPeer = (XVclWindowPeer) UnoRuntime.queryInterface(XVclWindowPeer.class, xWindowPeer);
                boolean bisHighContrast = oMessageBox.isHighContrastModeActivated(xVclWindowPeer);
                oMessageBox.showErrorMessageBox(xWindowPeer, "My Sampletitle", "HighContrastMode is enabled: " + bisHighContrast);
            } else{
                System.out.println("Could not retrieve current frame");
            }

        } catch( Exception e ) {
            System.err.println( e + e.getMessage());
            e.printStackTrace();
        } finally {
            if (xComp != null) {
                try {
                    XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(XCloseable.class, xComp);
                    if (xClose != null) {
                        xClose.close(false);
                    } else {
                        xComp.dispose();
                    }
                } catch (com.sun.star.util.CloseVetoException e) {
                    System.err.println( e + e.getMessage());
                    e.printStackTrace();
                }
            }
        }

        System.exit( 0 );
    }

    // helper method to get the window peer of a document or if no
    // document is specified it tries to get the avtive frame
    // which is potentially dangerous
    public XWindowPeer getWindowPeerOfFrame(XComponent xComp) {
        try {
            XFrame xFrame = null;

            if (xComp != null) {
                XModel xModel = (XModel)UnoRuntime.queryInterface(XModel.class, xComp);
                xFrame = xModel.getCurrentController().getFrame();

            } else {
                // Note: This method is potentially dangerous and should only be used for debugging
                // purposes as it relies on the platform dependent window handler..
                Object oDesktop = m_xMCF.createInstanceWithContext("com.sun.star.frame.Desktop", m_xContext);
                XFramesSupplier xFramesSupplier = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, oDesktop);
                xFrame = xFramesSupplier.getActiveFrame();
            }

            if (xFrame != null){
                XWindow xWindow = xFrame.getContainerWindow();
                if (xWindow != null){
                    XWindowPeer xWindowPeer =  (XWindowPeer) UnoRuntime.queryInterface(XWindowPeer.class, xWindow);
                    return xWindowPeer;
                }
            }
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
        }
        return null;
    }

    XComponent createDefaultTextDocument() {

        XComponent xComp =  null;
        try {
            Object oDesktop = m_xMCF.createInstanceWithContext(
                                               "com.sun.star.frame.Desktop", m_xContext);

            // get the component laoder from the desktop to create a new
            // text document
            com.sun.star.frame.XComponentLoader xCLoader =(com.sun.star.frame.XComponentLoader)
                UnoRuntime.queryInterface(com.sun.star.frame.XComponentLoader.class,oDesktop);

            com.sun.star.beans.PropertyValue[] args = new com.sun.star.beans.PropertyValue [1];
            args[0] = new com.sun.star.beans.PropertyValue();
            args[0].Name = "Hidden";
            args[0].Value = new Boolean(true);
            String strDoc = "private:factory/swriter";

            xComp = xCLoader.loadComponentFromURL(strDoc, "_blank", 0, args);

        } catch(com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
        }
        return xComp;
    }

    /** shows an error messagebox
     *  @param _xParentWindowPeer the windowpeer of the parent window
     *  @param _sTitle the title of the messagebox
     *  @param _sMessage the message of the messagebox
     */
    public void showErrorMessageBox(XWindowPeer _xParentWindowPeer, String _sTitle, String _sMessage) {
        XComponent xComponent = null;
        try {
            Object oToolkit = m_xMCF.createInstanceWithContext("com.sun.star.awt.Toolkit", m_xContext);
            XMessageBoxFactory xMessageBoxFactory = (XMessageBoxFactory) UnoRuntime.queryInterface(XMessageBoxFactory.class, oToolkit);
            // rectangle may be empty if position is in the center of the parent peer
            Rectangle aRectangle = new Rectangle();
            XMessageBox xMessageBox = xMessageBoxFactory.createMessageBox(_xParentWindowPeer, aRectangle, "errorbox", com.sun.star.awt.MessageBoxButtons.BUTTONS_OK, _sTitle, _sMessage);
            xComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, xMessageBox);
            if (xMessageBox != null){
                short nResult = xMessageBox.execute();
            }
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace(System.err);
        } finally{
            //make sure always to dispose the component and free the memory!
            if (xComponent != null){
                xComponent.dispose();
            }
        }
    }


    /** @param _xVclWindowPeer the windowpeer of a dialog control or the dialog itself
     *  @return true if HighContrastMode is activated or false if HighContrastMode is deactivated
     */
    public boolean isHighContrastModeActivated(XVclWindowPeer _xVclWindowPeer) {
        boolean bIsActivated = false;

        try {
            if (_xVclWindowPeer != null){
                int nUIColor = AnyConverter.toInt(_xVclWindowPeer.getProperty("DisplayBackgroundColor"));
                int nRed = getRedColorShare(nUIColor);
                int nGreen = getGreenColorShare(nUIColor);
                int nBlue = getBlueColorShare(nUIColor);
                int nLuminance = (( nBlue*28 + nGreen*151 + nRed*77 ) / 256 );
                boolean bisactivated = (nLuminance <= 25);
                return bisactivated;
            } else{
                return false;
            }
        } catch (IllegalArgumentException e) {
            e.printStackTrace(System.err);
        }
        return bIsActivated;
    }

    public static int getRedColorShare(int _nColor) {
        int nRed = (int) _nColor/65536;
        int nRedModulo = _nColor % 65536;
        int nGreen = (int) (nRedModulo / 256);
        int nGreenModulo = (nRedModulo % 256);
        int nBlue = nGreenModulo;
        return nRed;
    }

    public static int getGreenColorShare(int _nColor) {
        int nRed = (int) _nColor/65536;
        int nRedModulo = _nColor % 65536;
        int nGreen = (int) (nRedModulo / 256);
        return nGreen;
    }

    public static int getBlueColorShare(int _nColor) {
        int nRed = (int) _nColor/65536;
        int nRedModulo = _nColor % 65536;
        int nGreen = (int) (nRedModulo / 256);
        int nGreenModulo = (nRedModulo % 256);
        int nBlue = nGreenModulo;
        return nBlue;
    }

}

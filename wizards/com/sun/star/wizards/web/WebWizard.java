/*
 * Start1.java
 *
 * Created on 19. September 2003, 12:09
 */

package com.sun.star.wizards.web;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Desktop;

/**
 * The last class in the WebWizard Dialog class hirarchy.
 * Has no functionality, is just nice to have it instanciated.
 * @author  rpiterman
 */
public class WebWizard extends WWD_Events{

    /**
     * @param xmsf
     */
    public WebWizard(XMultiServiceFactory xmsf) throws Exception {
        super(xmsf);
    }

    public void flash() {
        myOwnFrame.activate();
    }

    public static void main(String args[]) {

        String ConnectStr =
            "uno:socket,host=localhost,port=8111;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
        //localhost  ;Lo-1.Germany.sun.com; 10.16.65.155
        try {
            XMultiServiceFactory xmsf = Desktop.connect(ConnectStr);
            //File dir = new File("D:\\CWS\\extras6");//("C:\\Documents and Settings\\rpiterman\\My Documents");
            //getFiles(dir,xLocMSF);

            //read Configuration
            WebWizard ww = new WebWizard(xmsf);
            ww.show();
            ww.cleanup();

        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }
}
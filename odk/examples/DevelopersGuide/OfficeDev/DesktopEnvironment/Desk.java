/*************************************************************************
 *
 *  $RCSfile: Desk.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:29:42 $
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

package OfficeDev.samples.DesktopEnvironment;

// __________ Imports __________

// structs, const ...
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.DispatchResultEvent;
import com.sun.star.frame.DispatchResultState;

// interfaces
import com.sun.star.frame.XDispatchResultListener;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;

// helper
import com.sun.star.uno.UnoRuntime;

// Exceptions
import java.lang.IllegalMonitorStateException;
import java.lang.InterruptedException;

// base classes
import java.lang.String;

// __________ Implementation __________

/**
 * TODO
 *
 * @author     Andreas Schl&uuml;ns
 * @created    08.02.2002 14:05
 */
public class Desk
{
    // ____________________

    /**
     * main
     * Establish connection to a remote office and starts the demo application.
     * User can overwrite some of neccessary start options by using command line parameters.
     *
     * syntax: Desk [host=<hostname>] [port=<portnumber>] [mode={inplace|outplace}] [file=<filename>]
     *
     * @param  args  command line arguments
     *                  host        describe host on which remote office runs
     *                              default=localhost
     *                  port        describe port which is used by remote office
     *                              default=8100
     *                  mode        describe using mode of document view {inplace/outplace}
     *                              default=inplace
     *                  file        name of first file which should be open
     *                              default="private:factory/swriter" to open empty writer document
     */
    public static void main(String[] lArguments)
    {
        // Analyze command line parameters.
        String  sHost  = new String("localhost");
        String  sPort  = new String("8100");
        String  sMode  = new String("inplace");
        String  sFile  = new String("private:factory/swriter");

        for(int i=0; i<lArguments.length; ++i)
        {
            lArguments[i] = lArguments[i].toLowerCase();
            if(lArguments[i].startsWith("host=")==true)
                sHost = lArguments[i].substring(5);
            else
            if(lArguments[i].startsWith("port=")==true)
                sPort = lArguments[i].substring(5);
            else
            if(lArguments[i].startsWith("mode=")==true)
                sMode = lArguments[i].substring(5);
            else
            if(lArguments[i].startsWith("file=")==true)
                sFile = lArguments[i].substring(5);
        }

        ViewContainer.mbInplace = (sMode.compareTo("inplace")==0);

        // Connect to remote office.
        OfficeConnect.createConnection(sHost,sPort);

        // Create first document view.
        // This one will register himself at the global
        // ViewContainer. Further views will be open
        // automaticly started from this first one.
        DocumentView aView = new DocumentView();
        aView.setVisible(true);
        aView.createFrame();
        aView.load(sFile);
    }
}

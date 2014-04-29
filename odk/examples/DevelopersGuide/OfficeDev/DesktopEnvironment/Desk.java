/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// __________ Imports __________

import com.sun.star.uno.UnoRuntime;

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
     * User can overwrite some of necessary start options by using command line parameters.
     *
     * syntax: Desk [mode={inplace|outplace}] [file=<filename>]
     *
     * @param  args  command line arguments
     *                  mode        describe using mode of document view {inplace/outplace}
     *                              default=inplace
     *                  file        name of first file which should be open
     *                              default="private:factory/swriter" to open empty writer document
     */
    public static void main(String[] lArguments)
    {
        // Analyze command line parameters.
        String  sMode  = new String("inplace");
        String  sFile  = new String("private:factory/swriter");

        for(int i=0; i<lArguments.length; ++i)
        {
            lArguments[i] = lArguments[i].toLowerCase();
            if(lArguments[i].startsWith("mode=")==true)
                sMode = lArguments[i].substring(5);
            else
            if(lArguments[i].startsWith("file=")==true)
                sFile = lArguments[i].substring(5);
        }

        ViewContainer.mbInplace = (sMode.compareTo("inplace")==0);

        // Connect to remote office.
        OfficeConnect.createConnection();

        // Create first document view.
        // This one will register himself at the global
        // ViewContainer. Further views will be open
        // automatically started from this first one.
        DocumentView aView = new DocumentView();
        aView.setVisible(true);
        aView.createFrame();
        aView.load(sFile);
    }
}

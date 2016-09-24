/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// __________ Imports __________

// __________ Implementation __________

/**
 * TODO
 *
 */
public class Desk
{


    /**
     * main
     * Establish connection to a remote office and starts the demo application.
     * User can overwrite some of necessary start options by using command line parameters.
     *
     * syntax: Desk [mode={inplace|outplace}] [file=<filename>]
     *
     * @param  lArguments  command line arguments
     *                  mode        describe using mode of document view {inplace/outplace}
     *                              default=inplace
     *                  file        name of first file which should be open
     *                              default="private:factory/swriter" to open empty writer document
     */
    public static void main(String[] lArguments)
    {
        // Analyze command line parameters.
        String  sMode  = "inplace";
        String  sFile  = "private:factory/swriter";

        for(int i=0; i<lArguments.length; ++i)
        {
            lArguments[i] = lArguments[i].toLowerCase();
            if(lArguments[i].startsWith("mode="))
                sMode = lArguments[i].substring(5);
            else
            if(lArguments[i].startsWith("file="))
                sFile = lArguments[i].substring(5);
        }

        ViewContainer.mbInplace = sMode.equals("inplace");

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

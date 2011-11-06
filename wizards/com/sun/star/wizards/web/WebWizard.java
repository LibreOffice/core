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
public class WebWizard extends WWD_Events
{

    /**
     * @param xmsf
     */
    public WebWizard(XMultiServiceFactory xmsf) throws Exception
    {
        super(xmsf);
    }

    public static void main(String args[])
    {

        String ConnectStr =
                "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.ServiceManager";
        try
        {
            XMultiServiceFactory xmsf = Desktop.connect(ConnectStr);
            WebWizard ww = new WebWizard(xmsf);
            ww.show();
            ww.cleanup();

        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }
}

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
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author zxf
 */

package complex.passwordcontainer;

import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.task.XInteractionContinuation;
import com.sun.star.ucb.XInteractionSupplyAuthentication;
import com.sun.star.task.XInteractionRequest;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.MasterPasswordRequest;
import com.sun.star.uno.UnoRuntime;

public class MasterPasswdHandler extends WeakBase
        implements XInteractionHandler {
    XInteractionHandler m_xHandler;

    public MasterPasswdHandler( XInteractionHandler xHandler ) {
        m_xHandler = xHandler;
    }

    public void handle( XInteractionRequest xRequest ) {
        try {
            MasterPasswordRequest aMasterPasswordRequest;
            if( xRequest.getRequest() instanceof MasterPasswordRequest ) {
                aMasterPasswordRequest = (MasterPasswordRequest)xRequest.getRequest();
                if( aMasterPasswordRequest != null ) {
                    XInteractionContinuation xContinuations[] = xRequest.getContinuations();
                    XInteractionSupplyAuthentication xAuthentication = null;

                    for( int i = 0; i < xContinuations.length; ++i ) {
                        xAuthentication = UnoRuntime.queryInterface(XInteractionSupplyAuthentication.class, xContinuations[i]);
                        if( xAuthentication != null )
                        {
                            break;
                        }
                    }
                    if( xAuthentication.canSetPassword() )
                    {
                        xAuthentication.setPassword("abcdefghijklmnopqrstuvwxyz123456");
                    }
                    xAuthentication.select();
                }
            } else {
                m_xHandler.handle( xRequest );
            }
        } catch( Exception e ) {
            System.out.println( "MasterPasswordHandler Error: " + e );
        }
    }
}








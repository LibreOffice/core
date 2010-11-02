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








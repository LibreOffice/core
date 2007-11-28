/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WikiEditSettingDialog.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2007-11-28 11:15:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package com.sun.star.wiki;

import com.sun.star.awt.XDialog;
import com.sun.star.uno.XComponentContext;
import java.util.Hashtable;
import javax.net.ssl.SSLException;

import org.apache.commons.httpclient.*;
import org.apache.commons.httpclient.methods.*;
import org.apache.commons.httpclient.protocol.Protocol;

public class WikiEditSettingDialog extends WikiDialog
{

    private final String sOKMethod = "OK";
    private final String sCancelMethod = "Cancel";
    private final String sHelpMethod = "Help";

    String[] Methods =
    {sOKMethod, sCancelMethod, sHelpMethod};
    private Hashtable setting;
    private boolean addMode;

    public WikiEditSettingDialog( XComponentContext c, String DialogURL )
    {
        super( c, DialogURL );
        super.setMethods( Methods );
        setting = new Hashtable();
        addMode = true;
    }

    public WikiEditSettingDialog( XComponentContext c, String DialogURL, Hashtable ht )
    {
        super( c, DialogURL );
        super.setMethods( Methods );
        setting = ht;
        try
        {
            getPropSet( "UrlField" ).setPropertyValue( "Text", ht.get( "Url" ));
            getPropSet( "UsernameField" ).setPropertyValue( "Text", ht.get( "Username" ));
            getPropSet( "PasswordField" ).setPropertyValue( "Text", ht.get( "Password" ));
        }
        catch ( Exception ex )
        {
            ex.printStackTrace();
        }
        addMode = false;
    }

    public boolean callHandlerMethod( XDialog xDialog, Object EventObject, String MethodName )
    {
        if ( MethodName.equals( sOKMethod ) )
        {
            try
            {
                String sURL = ( String ) getPropSet( "UrlField" ).getPropertyValue( "Text" );
                String sUserName = ( String ) getPropSet( "UsernameField" ).getPropertyValue( "Text" );
                String sPassword = ( String ) getPropSet( "PasswordField" ).getPropertyValue( "Text" );
                String sRedirectURL = "";

                HostConfiguration aHostConfig = new HostConfiguration();
                boolean bInitHost = true;

                do
                {
                    if ( sRedirectURL.length() > 0 )
                    {
                        sURL = sRedirectURL;
                        sRedirectURL = "";
                    }

                    if ( sURL.length() > 0 )
                    {
                        URI aURI = new URI( sURL );
                        GetMethod aRequest = new GetMethod( aURI.getEscapedPathQuery() );
                        aRequest.setFollowRedirects( false );
                        Helper.ExecuteMethod( aRequest, aHostConfig, aURI, m_xContext, bInitHost );
                        bInitHost = false;

                        int nResultCode = aRequest.getStatusCode();
                        String sWebPage = null;
                        if ( nResultCode == 200 )
                            sWebPage = aRequest.getResponseBodyAsString();
                        else if ( nResultCode >= 301 && nResultCode <= 303 || nResultCode == 307 )
                            sRedirectURL = aRequest.getResponseHeader( "Location" ).getValue();

                        aRequest.releaseConnection();

                        if ( sWebPage != null && sWebPage.length() > 0 )
                        {
                            //the URL is valid
                            String sMainURL = Helper.GetMainURL( sWebPage, sURL );

                            if ( sMainURL.equals( "" ) )
                            {
                                // TODO:
                                // it's not a Wiki Page, check first whether a redirect is requested
                                // happens usually in case of https
                                sRedirectURL = Helper.GetRedirectURL( sWebPage, sURL );
                                if ( sRedirectURL.equals( "" ) )
                                {
                                    // show error
                                    ErrorDialog ed = new ErrorDialog( m_xContext, "vnd.sun.star.script:WikiEditor.Error?location=application", "This is not a Wiki Page!" );
                                    ed.show();
                                }
                            }
                            else
                            {
                                if ( ( sUserName.length() > 0 || sPassword.length() > 0 )
                                  && Helper.Login( new URI( sMainURL ), sUserName, sPassword, "sun", m_xContext ) == null )
                                {
                                    // a wrong login information is provided
                                    // show error
                                    ErrorDialog ed = new ErrorDialog( m_xContext, "vnd.sun.star.script:WikiEditor.Error?location=application", "The login information is wrong! It should either be empty or correct." );
                                    ed.show();
                                }
                                else
                                {
                                    setting.put( "Url",sMainURL );
                                    setting.put( "Username", sUserName );
                                    setting.put( "Password", sPassword );
                                    if ( addMode )
                                        Settings.getSettings( m_xContext ).addWikiCon( setting );

                                    m_bAction = true;
                                    xDialog.endExecute();
                                }
                            }
                        }
                        else if ( sRedirectURL == null || sRedirectURL.length() == 0 )
                        {
                            // URL invalid
                            // show error
                            ErrorDialog ed = new ErrorDialog( m_xContext, "vnd.sun.star.script:WikiEditor.Error?location=application", "The URl is invalid!" );
                            ed.show();
                        }
                    }
                    else
                    {
                        // URL field empty
                        // show error
                        ErrorDialog ed = new ErrorDialog( m_xContext, "vnd.sun.star.script:WikiEditor.Error?location=application", "The URL field is empty!" );
                        ed.show();

                    }
                } while ( sRedirectURL.length() > 0 );
            }
            catch ( Exception ex )
            {
                ErrorDialog ed = new ErrorDialog( m_xContext, "vnd.sun.star.script:WikiEditor.Error?location=application", "Can not access the provided URL for any reason!" );
                ed.show();

                ex.printStackTrace();
            }
            return true;
        }
        else if ( MethodName.equals( sCancelMethod ) )
        {
            xDialog.endExecute();
            return true;
        }
        else if ( MethodName.equals( sHelpMethod ) )
        {
            return true;
        }

        return false;
    }


}

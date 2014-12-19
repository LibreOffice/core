/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.wiki;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XChangesBatch;

public class Settings
{

    /* Singleton */
    private static Settings m_instance;


    private final XComponentContext m_xContext;
    private final List<Map<String, String>> m_WikiConnections = new ArrayList<Map<String, String>>();
    private final List<Map<String, Object>> m_aWikiDocs = new ArrayList<Map<String, Object>>();

    private Settings( XComponentContext ctx )
    {
        m_xContext=ctx;
        loadConfiguration();
    }


    public static synchronized Settings getSettings( XComponentContext ctx )
    {
        if ( m_instance == null )
            m_instance = new Settings( ctx );
        return m_instance;
    }


    public void addWikiCon ( Map<String, String> wikiCon )
    {
        m_WikiConnections.add( wikiCon );
    }


    private String getWikiConUrlByNumber( int num )
    {
        String url = "";
        if ( num >=0 && num < m_WikiConnections.size() )
        {
            Map<String,String> ht = m_WikiConnections.get( num );
            url = ht.get( "Url" );
        }
        return url;
    }


    public void addWikiDoc ( Map<String, Object> aWikiDoc )
    {
        String sURL = ( String ) aWikiDoc.get( "CompleteUrl" );
        Map<String,Object> aEntry = getDocByCompleteUrl( sURL );

        if ( aEntry != null )
        {
            // add doc to the end, even if it has been added before
            m_aWikiDocs.remove( aEntry );
        }
        else if ( m_aWikiDocs.size() > 10 )
        {
            // if the number of elements has reached maximum the oldest element should be removed
            m_aWikiDocs.remove( 0 );
        }

        m_aWikiDocs.add( aWikiDoc );
    }


    public Object[] getWikiDocList( int serverid )
    {
        String wikiserverurl = getWikiConUrlByNumber( serverid );
        List<String> theDocs = new ArrayList<String>();
        String [] docs = new String[0];
        for ( int i=0; i<m_aWikiDocs.size(); i++ )
        {
            Map<String,Object> ht = m_aWikiDocs.get( i );
            String docurl = ( String ) ht.get( "Url" );
            if ( docurl.equals( wikiserverurl ) )
            {
                theDocs.add( (String ) ht.get( "Doc" ) );
            }
        }
        return theDocs.toArray( docs );
    }

    public void setLastUsedWikiServer( int l )
    {
    }

    public String[] getWikiURLs()
    {
        String [] WikiList = new String [m_WikiConnections.size()];
        for ( int i=0; i<m_WikiConnections.size(); i++ )
        {
            Map<String,String> ht = m_WikiConnections.get( i );
            WikiList[i] = ht.get( "Url" );
        }
        return WikiList;
    }


    public Map<String, String> getSettingByUrl( String sUrl )
    {
        Map<String, String> ht = null;
        for( int i=0;i<m_WikiConnections.size();i++ )
        {
            Map<String, String> h1 = m_WikiConnections.get( i );
            String u1 = h1.get( "Url" );
            if ( u1.equals( sUrl ) )
            {
                ht = h1;
                try
                {
                    String sUserName = ht.get( "Username" );
                    String aPassword = ht.get( "Password" );
                    if ( sUserName != null && sUserName.length() > 0 && ( aPassword == null || aPassword.length() == 0 ) )
                    {
                        String[] pPasswords = Helper.GetPasswordsForURLAndUser( m_xContext, sUrl, sUserName );
                        if ( pPasswords != null && pPasswords.length > 0 )
                            ht.put( "Password", pPasswords[0] );
                    }
                }
                catch( Exception e )
                {
                    e.printStackTrace();
                }

                break;
            }
        }
        return ht;
    }

    private Map<String,Object> getDocByCompleteUrl( String curl )
    {
        Map<String,Object> ht = null;
        for( int i=0;i<m_aWikiDocs.size();i++ )
        {
            Map<String,Object> h1 = m_aWikiDocs.get( i );
            String u1 = ( String ) h1.get( "CompleteUrl" );
            if ( u1.equals( curl ) )
            {
                ht = h1;
            }
        }
        return ht;
    }


    public void removeSettingByUrl( String sUrl )
    {
        for( int i=0;i<m_WikiConnections.size();i++ )
        {
            Map<String,String> h1 = m_WikiConnections.get( i );
            String u1 = h1.get( "Url" );
            if ( u1.equals( sUrl ) )
            {
                m_WikiConnections.remove( i );
            }
        }
    }


    public void storeConfiguration()
    {
        try
        {
            {
                // remove stored connection information
                XNameContainer xContainer = Helper.GetConfigNameContainer(m_xContext, "org.openoffice.Office.Custom.WikiExtension/ConnectionList");
                String[] pNames = xContainer.getElementNames();
                for (String pName : pNames)
                {
                    xContainer.removeByName(pName);
                }
                // store all connections
                XSingleServiceFactory xConnectionFactory = UnoRuntime.queryInterface(XSingleServiceFactory.class, xContainer);
                for (Map<String, String> ht : m_WikiConnections)
                {
                    Object oNewConnection = xConnectionFactory.createInstance();
                    XNameReplace xNewConn = UnoRuntime.queryInterface(XNameReplace.class, oNewConnection);
                    if (xNewConn != null)
                    {
                        xNewConn.replaceByName("UserName", ht.get("Username"));
                    }
                    xContainer.insertByName(ht.get("Url"), xNewConn);
                }
                // commit changes
                XChangesBatch xBatch = UnoRuntime.queryInterface(XChangesBatch.class, xContainer);
                xBatch.commitChanges();
            }

            {
                // remove stored connection information
                XNameContainer xContainer = Helper.GetConfigNameContainer(m_xContext, "org.openoffice.Office.Custom.WikiExtension/RecentDocs");
                String[] pNames = xContainer.getElementNames();
                for (String pName : pNames)
                {
                    xContainer.removeByName(pName);
                }
                // store all Docs
                XSingleServiceFactory xDocListFactory = UnoRuntime.queryInterface(XSingleServiceFactory.class, xContainer);
                int i = 0;
                for (Map<String, Object> ht : m_aWikiDocs)
                {
                    Object oNewDoc = xDocListFactory.createInstance();
                    XNameReplace xNewDoc = UnoRuntime.queryInterface(XNameReplace.class, oNewDoc);
                    for (Map.Entry<String, Object> entry : ht.entrySet())
                    {
                        xNewDoc.replaceByName(entry.getKey(), entry.getValue());
                    }
                    xContainer.insertByName("d" + i++, xNewDoc);
                }
                // commit changes
                XChangesBatch xBatch = UnoRuntime.queryInterface(XChangesBatch.class, xContainer);
                xBatch.commitChanges();
            }

        }
        catch ( Exception ex )
        {
            ex.printStackTrace();
        }
    }

    public void loadConfiguration()
    {
        m_WikiConnections.clear();
        try
        {
            // get configuration service
            // connect to configmanager
            XNameAccess xAccess = Helper.GetConfigNameAccess( m_xContext, "org.openoffice.Office.Custom.WikiExtension" );

            if ( xAccess != null )
            {
                Object oList = xAccess.getByName( "ConnectionList" );
                XNameAccess xConnectionList = UnoRuntime.queryInterface( XNameAccess.class, oList );
                String [] allCons = xConnectionList.getElementNames();
                for (String aConnection : allCons)
                {
                    Map<String, String> ht = new HashMap<String, String>();
                    ht.put("Url", aConnection);
                    ht.put( "Username", "" );
                    ht.put( "Password", "" );
                    try
                    {
                        XPropertySet xProps = UnoRuntime.queryInterface(XPropertySet.class, xConnectionList.getByName(aConnection));
                        if ( xProps != null )
                        {
                            String aUsername = AnyConverter.toString( xProps.getPropertyValue( "UserName" ) );
                            if ( aUsername != null && aUsername.length() > 0 )
                                ht.put( "Username", aUsername );
                        }
                    }
                    catch( Exception e )
                    {
                        e.printStackTrace();
                    }
                    addWikiCon( ht );
                }

                Object oDocs = xAccess.getByName( "RecentDocs" );
                XNameAccess xRecentDocs = UnoRuntime.queryInterface( XNameAccess.class, oDocs );
                String [] allDocs = xRecentDocs.getElementNames();
                for (String aDocument : allDocs)
                {
                    Object oDoc = xRecentDocs.getByName(aDocument);
                    XNameAccess xDoc = UnoRuntime.queryInterface( XNameAccess.class, oDoc );
                    Map<String, Object> ht = new HashMap<String, Object>();
                    ht.put( "Url", xDoc.getByName( "Url" ) );
                    ht.put( "CompleteUrl", xDoc.getByName( "CompleteUrl" ) );
                    ht.put( "Doc", xDoc.getByName( "Doc" ) );
                    addWikiDoc( ht );
                }
            }
        }
        catch ( Exception ex )
        {
            ex.printStackTrace();
        }
    }
}

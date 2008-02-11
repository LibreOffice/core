/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Settings.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mav $ $Date: 2008-02-11 08:35:34 $
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

import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.task.UrlRecord;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XChangesBatch;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

public class Settings
{

    private XComponentContext m_xContext;
    private int lastUsedWikiServer = 0;


    /* Singelton */
    private static Settings m_instance;


    private Vector m_WikiConnections = new Vector();
    private Vector m_aWikiDocs = new Vector();

    private Settings( XComponentContext ctx )
    {
        m_xContext=ctx;
        loadConfiguration();
    }


    public static synchronized Settings getSettings( XComponentContext ctx )
    {
        if ( m_instance == null )
            m_instance = new Settings( ctx );
        // m_instance.loadSettings();
        return m_instance;
    }


    public void addWikiCon ( Hashtable wikiCon )
    {
        m_WikiConnections.add( wikiCon );
    }


    public Vector getWikiCons()
    {
        return m_WikiConnections;
    }

    public String getWikiConUrlByNumber( int num )
    {
        String url = "";
        if ( num >=0 && num < m_WikiConnections.size() )
        {
            Hashtable ht = ( Hashtable ) m_WikiConnections.get( num );
            url = ( String ) ht.get( "Url" );
        }
        return url;
    }


    public void addWikiDoc ( Hashtable aWikiDoc )
    {
        String sURL = ( String ) aWikiDoc.get( "CompleteUrl" );
        Hashtable aEntry = getDocByCompleteUrl( sURL );

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


    public Vector getWikiDocs()
    {
        return m_aWikiDocs;
    }

    public Object[] getWikiDocList( int serverid, int num )
    {
        String wikiserverurl = getWikiConUrlByNumber( serverid );
        Vector theDocs = new Vector();
        String [] docs = new String[0];
        for ( int i=0; i<m_aWikiDocs.size(); i++ )
        {
            Hashtable ht = ( Hashtable ) m_aWikiDocs.get( i );
            String docurl = ( String ) ht.get( "Url" );
            if ( docurl.equals( wikiserverurl ) )
            {
                theDocs.add( (String ) ht.get( "Doc" ) );
            }
        }
        return theDocs.toArray( docs );
    }

    public int getLastUsedWikiServer()
    {
        return lastUsedWikiServer;
    }

    public void setLastUsedWikiServer( int l )
    {
        lastUsedWikiServer = l;
    }

    public String[] getWikiURLs()
    {
        String [] WikiList = new String [m_WikiConnections.size()];
        for ( int i=0; i<m_WikiConnections.size(); i++ )
        {
            Hashtable ht = ( Hashtable ) m_WikiConnections.get( i );
            WikiList[i] = ( String ) ht.get( "Url" );
        }
        return WikiList;
    }


    public Hashtable getSettingByUrl( String sUrl )
    {
        Hashtable ht = null;
        for( int i=0;i<m_WikiConnections.size();i++ )
        {
            Hashtable h1 = ( Hashtable ) m_WikiConnections.get( i );
            String u1 = ( String ) h1.get( "Url" );
            if ( u1.equals( sUrl ) )
            {
                ht = h1;
            }
        }
        return ht;
    }

    public Hashtable getDocByCompleteUrl( String curl )
    {
        Hashtable ht = null;
        for( int i=0;i<m_aWikiDocs.size();i++ )
        {
            Hashtable h1 = ( Hashtable ) m_aWikiDocs.get( i );
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
        Hashtable ht = null;
        for( int i=0;i<m_WikiConnections.size();i++ )
        {
            Hashtable h1 = ( Hashtable ) m_WikiConnections.get( i );
            String u1 = ( String ) h1.get( "Url" );
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
            // remove stored connection information
            XNameContainer xContainer = Helper.GetConfigNameContainer( m_xContext, "org.openoffice.Office.Custom.WikiExtension/ConnectionList" );
            String[] pNames = xContainer.getElementNames();
            for( int i=0; i<pNames.length; i++ )
            {
                xContainer.removeByName( pNames[i] );
            }

            // store all connections
            XSingleServiceFactory xConnectionFactory = ( XSingleServiceFactory ) UnoRuntime.queryInterface( XSingleServiceFactory.class, xContainer );
            for ( int i=0; i< m_WikiConnections.size(); i++ )
            {
                Object oNewConnection = xConnectionFactory.createInstance();
                Hashtable ht = ( Hashtable ) m_WikiConnections.get( i );
                xContainer.insertByName( (String)ht.get( "Url" ), oNewConnection );
            }
            // commit changes
            XChangesBatch xBatch = ( XChangesBatch ) UnoRuntime.queryInterface( XChangesBatch.class, xContainer );
            xBatch.commitChanges();

            // remove stored connection information
            XNameContainer xContainer2 = Helper.GetConfigNameContainer( m_xContext, "org.openoffice.Office.Custom.WikiExtension/RecentDocs" );
            String[] pNames2 = xContainer2.getElementNames();
            for( int i=0; i<pNames2.length; i++ )
            {
                xContainer2.removeByName( pNames2[i] );
            }
            // store all Docs
            XSingleServiceFactory xDocListFactory = ( XSingleServiceFactory ) UnoRuntime.queryInterface( XSingleServiceFactory.class, xContainer2 );
            for ( int i=0; i< m_aWikiDocs.size(); i++ )
            {
                Hashtable ht = ( Hashtable ) m_aWikiDocs.get( i );

                Object oNewDoc = xDocListFactory.createInstance();
                XNameReplace xNewDoc = ( XNameReplace ) UnoRuntime.queryInterface( XNameReplace.class, oNewDoc );

                Enumeration e = ht.keys();
                while ( e.hasMoreElements() )
                {
                    String key = ( String ) e.nextElement();
                    xNewDoc.replaceByName( key, ht.get( key ) );
                }

                xContainer2.insertByName( "d" + i, xNewDoc );
            }
            // commit changes
            XChangesBatch xBatch2 = ( XChangesBatch ) UnoRuntime.queryInterface( XChangesBatch.class, xContainer2 );
            xBatch2.commitChanges();

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
                XNameAccess xConnectionList = ( XNameAccess ) UnoRuntime.queryInterface( XNameAccess.class, oList );
                String [] allCons = xConnectionList.getElementNames();
                for ( int i=0; i<allCons.length; i++ )
                {
                    Hashtable ht = new Hashtable();
                    ht.put( "Url", allCons[i] );
                    ht.put( "Username", "" );
                    ht.put( "Password", "" );

                    //TODO/LATER: how to handle more than one user?
                    // for now use the first one
                    UrlRecord aRecord = Helper.GetUsersForURL( m_xContext, allCons[i] );
                    if ( aRecord != null && aRecord.UserList != null && aRecord.UserList.length > 0 )
                    {
                        ht.put( "Username", aRecord.UserList[0] );
                        if ( aRecord.UserList[0].Passwords != null && aRecord.UserList[0].Passwords.length > 0 )
                            ht.put( "Password", aRecord.UserList[0].Passwords[0] );
                    }
                    addWikiCon( ht );
                }

                Object oDocs = xAccess.getByName( "RecentDocs" );
                XNameAccess xRecentDocs = ( XNameAccess ) UnoRuntime.queryInterface( XNameAccess.class, oDocs );
                String [] allDocs = xRecentDocs.getElementNames();
                for ( int i=0; i<allDocs.length; i++ )
                {
                    Object oDoc = xRecentDocs.getByName( allDocs[i] );
                    XNameAccess xDoc = ( XNameAccess ) UnoRuntime.queryInterface( XNameAccess.class, oDoc );
                    Hashtable ht = new Hashtable();
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

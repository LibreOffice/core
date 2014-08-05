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
package org.openoffice.test.tools;

/**************************************************************************/

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyState;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

/**************************************************************************/
/** provides a small wrapper around a document view
*/
public class OfficeDocumentView
{
    private     XMultiServiceFactory    m_orb;
    private     XController             m_controller;
    private     OfficeDocument          m_document;

    /* ------------------------------------------------------------------ */
    final public XController getController()
    {
        return m_controller;
    }

    /* ------------------------------------------------------------------ */
    final protected OfficeDocument getDocument()
    {
        return m_document;
    }

    /* ------------------------------------------------------------------ */
    public OfficeDocumentView( XMultiServiceFactory orb, OfficeDocument document, XController controller )
    {
        m_orb = orb;
        m_document = document;
        m_controller = controller;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves a dispatcher for the given URL, obtained at the current view of the document
        @param aURL
            a one-element array. The first element must contain a valid
            <member scope="com.sun.star.util">URL::Complete</member> value. Upon return, the URL is correctly
            parsed.
        @return
            the dispatcher for the URL in question
    */
    public XDispatch getDispatcher( URL[] aURL ) throws com.sun.star.uno.Exception
    {
        XDispatch xReturn = null;

        // go get the dispatch provider of it's frame
        XDispatchProvider xProvider = UnoRuntime.queryInterface( XDispatchProvider.class, m_controller.getFrame() );
        if ( null != xProvider )
        {
            // need an URLTransformer
            XURLTransformer xTransformer = UnoRuntime.queryInterface( XURLTransformer.class,
                m_orb.createInstance( "com.sun.star.util.URLTransformer" ) );
            xTransformer.parseStrict( aURL );

            xReturn = xProvider.queryDispatch( aURL[0], new String( ), 0 );
        }
        return xReturn;
    }


    /* ------------------------------------------------------------------ */
    /** retrieves a dispatcher for the given URL, obtained at the current view of the document
    */
    public XDispatch getDispatcher( String url ) throws com.sun.star.uno.Exception
    {
        URL[] aURL = new URL[] { new URL() };
        aURL[0].Complete = url;
        return getDispatcher( aURL );
    }

    /* ------------------------------------------------------------------ */
    /** dispatches the given URL into the view, if there's a dispatcher for it

        @return
            <TRUE/> if the URL was successfully dispatched
    */
    public boolean dispatch( String i_url ) throws com.sun.star.uno.Exception
    {
        return dispatch( i_url, new PropertyValue[0] );
    }

    /* ------------------------------------------------------------------ */
    public boolean dispatch( final String i_url, final PropertyValue[] i_arguments ) throws com.sun.star.uno.Exception
    {
        URL[] completeURL = new URL[] { new URL() };
        completeURL[0].Complete = i_url;
        XDispatch dispatcher = getDispatcher( completeURL );
        if ( dispatcher == null )
            return false;

        dispatcher.dispatch( completeURL[0], i_arguments );
        return true;
    }

    /* ------------------------------------------------------------------ */
    public boolean dispatch( final String i_url, final NamedValue[] i_arguments ) throws com.sun.star.uno.Exception
    {
        final PropertyValue[] dispatchArgs = new PropertyValue[ i_arguments.length ];
        for ( int i=0; i<i_arguments.length; ++i )
        {
            dispatchArgs[i] = new PropertyValue( i_arguments[i].Name, -1, i_arguments[i].Value, PropertyState.DIRECT_VALUE );
        }
        return dispatch( i_url, dispatchArgs );
    }
}


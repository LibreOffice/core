/*************************************************************************
 *
 *  $RCSfile: ProviderBrowseNode.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:56:06 $
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

package com.sun.star.script.framework.browse;

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.lib.uno.helper.PropertySet;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.ucb.XSimpleFileAccess;

import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.script.XInvocation;

import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;

import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.framework.log.*;
import com.sun.star.script.framework.container.*;
import com.sun.star.script.framework.browse.DialogFactory;

import java.io.*;
import java.util.*;
import javax.swing.JOptionPane;

public class ProviderBrowseNode extends PropertySet
    implements XBrowseNode, XInvocation
{
    protected ScriptProvider provider;
    protected Collection browsenodes;
    protected String name;
    protected ParcelContainer container;
    protected Parcel parcel;
    protected XComponentContext m_xCtx;

    public boolean deletable = true;
    public boolean creatable = true;
    public boolean editable = false;

    public ProviderBrowseNode( ScriptProvider provider, ParcelContainer container, XComponentContext xCtx ) {
        LogUtils.DEBUG("*** ProviderBrowseNode ctor");
        this.container = container;
        this.name = this.container.getLanguage();
        this.provider = provider;
        this.m_xCtx = xCtx;

        registerProperty("Deletable", new Type(boolean.class),
            (short)0, "deletable");
        registerProperty("Creatable", new Type(boolean.class),
            (short)0, "creatable");
        registerProperty("Editable", new Type(boolean.class),
            (short)0, "editable");
        XSimpleFileAccess xSFA = null;
        XMultiComponentFactory xFac = m_xCtx.getServiceManager();
        try
        {
            xSFA = ( XSimpleFileAccess)
                UnoRuntime.queryInterface( XSimpleFileAccess.class,
                    xFac.createInstanceWithContext(
                        "com.sun.star.ucb.SimpleFileAccess",
                        xCtx ) );
            if (  container.isUnoPkg() || xSFA.isReadOnly( container.getParcelContainerDir() ) )
            {
                deletable = false;
                creatable = false;
            }
        }
        // TODO propage errors
        catch( com.sun.star.uno.Exception e )
        {
                LogUtils.DEBUG("Caught exception in creation of ProviderBrowseNode ");
                LogUtils.DEBUG( LogUtils.getTrace(e));

        }
    }

    public String getName() {
        LogUtils.DEBUG("*** ProviderBrowseNode getName");
        return name;
    }

    public XBrowseNode[] getChildNodes() {
        LogUtils.DEBUG("***** ProviderBrowseNode.getChildNodes()");
        if ( hasChildNodes() )
        {
            // needs initialisation?
            LogUtils.DEBUG("** ProviderBrowseNode.getChildNodes(), container is " + container );
            String[] parcels = container.getElementNames();
            browsenodes = new ArrayList( parcels.length );
            for ( int index = 0; index < parcels.length; index++ )
            {
                try
                {
                    XBrowseNode node  = new ParcelBrowseNode( provider, container, parcels[ index ] );
                    browsenodes.add( node );
                }
                catch ( Exception e )
                {
                    LogUtils.DEBUG("*** Failed to create parcel node for " + parcels[ index ] );
                    LogUtils.DEBUG( e.toString() );
                }
            }
            ParcelContainer[] packageContainers = container.getChildContainers();
            LogUtils.DEBUG("**** For container named " + container.getName() + " with root path " + container.getParcelContainerDir() + " has " + packageContainers.length + " child containers " );

            for ( int i = 0; i < packageContainers.length; i++ )
            {
                XBrowseNode node = new PkgProviderBrowseNode( provider, packageContainers[ i ], m_xCtx );
                browsenodes.add( node );
            }
        }
        else
        {
            LogUtils.DEBUG("*** No container available");
            return new XBrowseNode[0];
        }
        return ( XBrowseNode[] )browsenodes.toArray( new XBrowseNode[0] );
    }

    public boolean hasChildNodes() {
        LogUtils.DEBUG("***** ProviderBrowseNode.hasChildNodes() ");
        LogUtils.DEBUG("***** ProviderBrowseNode.hasChildNodes() name " + container.getName() );
        LogUtils.DEBUG("***** ProviderBrowseNode.hasChildNodes() path " + container.getParcelContainerDir() );
        if ( container == null ||
             ( !container.hasElements() && !(container.getChildContainers().length == 0 ) )  )
        {
           LogUtils.DEBUG("** ProviderBrowseNode hasChildNodes() returning FALSE for has children");
            return true;
        }
        LogUtils.DEBUG("** ProviderBrowseNode returning TRUE for has children");
        return true;
    }

    public short getType() {
        return BrowseNodeTypes.CONTAINER;
    }

    public String toString()
    {
        return getName();
    }

    // implementation of XInvocation interface
    public XIntrospectionAccess getIntrospection() {
        return null;
    }

    public Object invoke(String aFunctionName, Object[] aParams,
                         short[][] aOutParamIndex, Object[][] aOutParam)
        throws com.sun.star.lang.IllegalArgumentException,
               com.sun.star.script.CannotConvertException,
               com.sun.star.reflection.InvocationTargetException
    {
        // Initialise the out paramters - not used but prevents error in
        // UNO bridge
        aOutParamIndex[0] = new short[0];
        aOutParam[0] = new Object[0];

        Any result = new Any(new Type(Boolean.class), Boolean.TRUE);

        if (aFunctionName.equals("Creatable"))
        {
            try
            {
                String name;

                if (aParams == null || aParams.length < 1 ||
                    AnyConverter.isString(aParams[0]) == false)
                {
                    String prompt = "Enter name for new Parcel";
                    String title = "Create Parcel";

                    // try to get a DialogFactory instance, if it fails
                    // just use a Swing JOptionPane to prompt for the name
                    try
                    {
                        DialogFactory dialogFactory =
                            DialogFactory.getDialogFactory();

                        name = dialogFactory.showInputDialog(title, prompt);
                    }
                    catch (Exception e)
                    {
                        name = JOptionPane.showInputDialog(null, prompt, title,
                            JOptionPane.QUESTION_MESSAGE);
                    }
                }
                else {
                    name = (String) AnyConverter.toString(aParams[0]);
                }

                if (name == null || name.equals(""))
                {
                    result =  new Any(new Type(Boolean.class), Boolean.FALSE);
                }
                else
                {

                    Object newParcel  = container.createParcel( name );
                    LogUtils.DEBUG("Parcel created " + name + " " + newParcel );
                    if ( newParcel == null )
                    {
                        result =  new Any(new Type(Boolean.class), Boolean.FALSE);
                    }
                    else
                    {
                        ParcelBrowseNode parcel = new ParcelBrowseNode( provider, container, name );
                        LogUtils.DEBUG("created parcel node ");
                        if ( browsenodes == null )
                        {
                            browsenodes = new ArrayList( 5 );
                        }

                        browsenodes.add(parcel);


                        result = new Any(new Type(XBrowseNode.class), parcel);
                    }
                }
            }
            catch (Exception e)
            {
        LogUtils.DEBUG("ProviderBrowseNode[create] failed with: " + e );
                LogUtils.DEBUG( LogUtils.getTrace( e ) );
                result = new Any(new Type(Boolean.class), Boolean.FALSE);

                // throw new com.sun.star.reflection.InvocationTargetException(
                //     "Error creating script: " + e.getMessage());
            }
        }
        else {
            throw new com.sun.star.lang.IllegalArgumentException(
                "Function " + aFunctionName + " not supported.");
        }

        return result;
    }

    public void setValue(String aPropertyName, Object aValue)
        throws com.sun.star.beans.UnknownPropertyException,
               com.sun.star.script.CannotConvertException,
               com.sun.star.reflection.InvocationTargetException
    {
    }

    public Object getValue(String aPropertyName)
        throws com.sun.star.beans.UnknownPropertyException
    {
        return null;
    }

    public boolean hasMethod(String aName) {
        return false;
    }

    public boolean hasProperty(String aName) {
        return false;
    }
}

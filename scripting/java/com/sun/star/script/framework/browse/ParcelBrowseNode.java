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

package com.sun.star.script.framework.browse;

import com.sun.star.beans.XIntrospectionAccess;

import com.sun.star.lib.uno.helper.PropertySet;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;


import com.sun.star.lang.XMultiComponentFactory;


import com.sun.star.script.XInvocation;

import com.sun.star.ucb.XSimpleFileAccess;

import com.sun.star.script.browse.XBrowseNode;
import com.sun.star.script.browse.BrowseNodeTypes;

import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.framework.container.ScriptEntry;
import com.sun.star.script.framework.container.Parcel;
import com.sun.star.script.framework.container.ParcelContainer;
import com.sun.star.script.framework.browse.DialogFactory;

import java.util.*;
import javax.swing.JOptionPane;

public class ParcelBrowseNode extends PropertySet
    implements XBrowseNode, XInvocation
{
    private ScriptProvider provider;
    //private RootBrowseNode parent;
    private Collection<XBrowseNode> browsenodes;
    private String name;
    private ParcelContainer container;
    private Parcel parcel;
    public boolean deletable = true;
    public boolean editable  = false;
    public boolean creatable = false;
    public boolean renamable = true;

    public ParcelBrowseNode( ScriptProvider provider, ParcelContainer container, String parcelName ) {
        this.provider = provider;
        this.name = parcelName;
        this.container = container;

        // TODO decide whether exception is propagated out or not
        try
        {
            this.parcel = (Parcel)this.container.getByName( parcelName );
        }
        catch ( Exception e )
        {

            LogUtils.DEBUG("** Exception: " + e );
            LogUtils.DEBUG(" ** Failed to get parcel named " +
                           parcelName + " from container" );
        }
        registerProperty("Deletable", new Type(boolean.class),
            (short)0, "deletable");
        registerProperty("Editable", new Type(boolean.class),
            (short)0, "editable");
        registerProperty("Creatable", new Type(boolean.class),
            (short)0, "creatable");
        registerProperty("Renamable", new Type(boolean.class),
            (short)0, "renamable");
        if (provider.hasScriptEditor() == true)
        {
            this.creatable = true;
        }

        String parcelDirUrl = parcel.getPathToParcel();
        XComponentContext xCtx = provider.getScriptingContext().getComponentContext();
        XMultiComponentFactory xFac = xCtx.getServiceManager();
        try
        {
            XSimpleFileAccess xSFA = UnoRuntime.queryInterface( XSimpleFileAccess.class,
                xFac.createInstanceWithContext(
                    "com.sun.star.ucb.SimpleFileAccess",
                    xCtx ) );
            if ( xSFA != null && ( xSFA.isReadOnly( parcelDirUrl ) ||
                container.isUnoPkg() ) )
            {
                deletable = false;
                editable  = false;
                creatable = false;
                renamable = false;
            }
        }
        catch ( com.sun.star.uno.Exception e )
        {
            // TODO propagate potential errors
            // Pthrow new com.sun.star.uno.RuntimeException( e.toString() );
            LogUtils.DEBUG( "Caught exception creating ParcelBrowseNode " + e );
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
        }

    }

    public String getName() {
        return parcel.getName();
    }

    public XBrowseNode[] getChildNodes() {
        try
        {

            if ( hasChildNodes() )
            {
                String[] names = parcel.getElementNames();
                browsenodes = new ArrayList<XBrowseNode>( names.length );

                for ( int index = 0; index < names.length; index++ )
                {
                    browsenodes.add( new ScriptBrowseNode( provider, parcel, names[ index ] ));
                }
            }
            else
            {
                LogUtils.DEBUG("ParcelBrowseNode.getChildeNodes no children " );
                return new XBrowseNode[0];
            }
        }
        catch ( Exception e )
        {
            LogUtils.DEBUG("Failed to getChildeNodes, exception: " + e );
            LogUtils.DEBUG( LogUtils.getTrace( e ) );
            return new XBrowseNode[0];
        }
        return browsenodes.toArray(new XBrowseNode[browsenodes.size()]);
    }

    public boolean hasChildNodes() {
        if ( container != null && container.hasByName( getName() ) && parcel != null )
        {
            return parcel.hasElements();
        }

        return false;
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
        LogUtils.DEBUG("ParcelBrowseNode invoke for " + aFunctionName );
        // Initialise the out parameters - not used but prevents error in
        // UNO bridge
        aOutParamIndex[0] = new short[0];
        aOutParam[0] = new Object[0];

        Any result = new Any(new Type(Boolean.class), Boolean.TRUE);

        if (aFunctionName.equals("Creatable"))
        {
            try
            {
                String newName;

                if (aParams == null || aParams.length < 1 ||
                    AnyConverter.isString(aParams[0]) == false)
                {
                    String prompt = "Enter name for new Script";
                    String title = "Create Script";

                    // try to get a DialogFactory instance, if it fails
                    // just use a Swing JOptionPane to prompt for the name
                    try
                    {
                        DialogFactory dialogFactory =
                            DialogFactory.getDialogFactory();

                        newName = dialogFactory.showInputDialog(title, prompt);
                    }
                    catch (Exception e)
                    {
                        newName = JOptionPane.showInputDialog(null, prompt, title,
                            JOptionPane.QUESTION_MESSAGE);
                    }
                }
                else {
                    newName = AnyConverter.toString(aParams[0]);
                }

                if ( newName == null || newName.equals(""))
                {
                    result =  new Any(new Type(Boolean.class), Boolean.FALSE);
                }
                else
                {
                    String source = new String(provider.getScriptEditor().getTemplate().getBytes());
                    String languageName = newName + "." + provider.getScriptEditor().getExtension();
                    String language = container.getLanguage();

                    ScriptEntry entry = new ScriptEntry( language, languageName, languageName, "", new HashMap<String,String>() );

                    Parcel parcel = (Parcel)container.getByName( getName() );
                    ScriptMetaData data = new ScriptMetaData( parcel, entry, source );
                    parcel.insertByName( languageName, data );

                    ScriptBrowseNode sbn = new ScriptBrowseNode( provider, parcel, languageName );

                    if(browsenodes==null)
                    {
                            LogUtils.DEBUG("browsenodes null!!");
                            browsenodes = new ArrayList<XBrowseNode>(4);
                    }
                    browsenodes.add(sbn);

                    result = new Any(new Type(XBrowseNode.class), sbn);
                }
            }
            catch (Exception e)
            {
        LogUtils.DEBUG("ParcelBrowseNode[create] failed with: " + e );
                LogUtils.DEBUG( LogUtils.getTrace( e ) );
                result = new Any(new Type(Boolean.class), Boolean.FALSE);

                // throw new com.sun.star.reflection.InvocationTargetException(
                //     "Error creating script: " + e.getMessage());
            }
        }
        else if (aFunctionName.equals("Deletable"))
        {
            try
            {
                if ( container.deleteParcel(getName()) )
                {
                    result = new Any(new Type(Boolean.class), Boolean.TRUE);
                }
                else
                {
                    result = new Any(new Type(Boolean.class), Boolean.FALSE);
                }
            }
            catch (Exception e)
            {
                result =  new Any(new Type(Boolean.class), Boolean.FALSE);

                // throw new com.sun.star.reflection.InvocationTargetException(
                //     "Error deleting parcel: " + e.getMessage());
            }
        }
        else if (aFunctionName.equals("Renamable"))
        {
            String newName = null;
            try
            {

                if (aParams == null || aParams.length < 1 ||
                    AnyConverter.isString(aParams[0]) == false)
                {
                    String prompt = "Enter new name for Library";
                    String title = "Rename Library";

                    // try to get a DialogFactory instance, if it fails
                    // just use a Swing JOptionPane to prompt for the name
                    try
                    {
                        DialogFactory dialogFactory =
                            DialogFactory.getDialogFactory();

                        newName = dialogFactory.showInputDialog(title, prompt);
                    }
                    catch (Exception e)
                    {
                        newName = JOptionPane.showInputDialog(null, prompt, title,
                            JOptionPane.QUESTION_MESSAGE);
                    }
                }
                else {
                    newName = AnyConverter.toString(aParams[0]);
                }
                container.renameParcel( getName(), newName );
                Parcel p = (Parcel)container.getByName( newName );
                if(browsenodes == null )
                {
                    getChildNodes();
                }
                ScriptBrowseNode[] childNodes = browsenodes.toArray(new ScriptBrowseNode[browsenodes.size()]);

                for ( int index = 0; index < childNodes.length; index++ )
                {
                    childNodes[ index ].updateURI( p );
                }
                result = new Any(new Type(XBrowseNode.class), this);
            }
            catch (Exception e)
            {
                result =  new Any(new Type(Boolean.class), Boolean.FALSE);

                // throw new com.sun.star.reflection.InvocationTargetException(
                //     "Error renaming parcel: " + e.getMessage());
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

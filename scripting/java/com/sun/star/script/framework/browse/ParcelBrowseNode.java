/*************************************************************************
 *
 *  $RCSfile: ParcelBrowseNode.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:55:37 $
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

import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.beans.PropertyAttribute;

import com.sun.star.lib.uno.helper.PropertySet;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;


import com.sun.star.lang.XMultiComponentFactory;


import com.sun.star.script.XInvocation;

import com.sun.star.ucb.XSimpleFileAccess;

import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;

import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.container.ScriptMetaData;
import com.sun.star.script.framework.container.ScriptEntry;
import com.sun.star.script.framework.container.Parcel;
import com.sun.star.script.framework.container.ParcelContainer;
import com.sun.star.script.framework.browse.DialogFactory;

import java.io.*;
import java.util.*;
import javax.swing.JOptionPane;

public class ParcelBrowseNode extends PropertySet
    implements XBrowseNode, XInvocation
{
    private ScriptProvider provider;
    //private RootBrowseNode parent;
    private Collection browsenodes;
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
            XSimpleFileAccess xSFA = ( XSimpleFileAccess)
                UnoRuntime.queryInterface( XSimpleFileAccess.class,
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
                browsenodes = new ArrayList( names.length );

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
        return (XBrowseNode[])browsenodes.toArray(new XBrowseNode[0]);
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
        // Initialise the out paramters - not used but prevents error in
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
                    newName = (String) AnyConverter.toString(aParams[0]);
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

                    ScriptEntry entry = new ScriptEntry( language, languageName, languageName, "", new HashMap() );

                    Parcel parcel = (Parcel)container.getByName( getName() );
                    ScriptMetaData data = new ScriptMetaData( parcel, entry, source );
                    parcel.insertByName( languageName, data );

                    ScriptBrowseNode sbn = new ScriptBrowseNode( provider, parcel, languageName );

                    if(browsenodes==null)
                    {
                            LogUtils.DEBUG("browsenodes null!!");
                            browsenodes = new ArrayList(4);
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
                    newName = (String) AnyConverter.toString(aParams[0]);
                }
                container.renameParcel( getName(), newName );
                Parcel p = (Parcel)container.getByName( newName );
                if(browsenodes == null )
                {
                    getChildNodes();
                }
                ScriptBrowseNode[] childNodes = (ScriptBrowseNode[])browsenodes.toArray(new ScriptBrowseNode[0]);

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

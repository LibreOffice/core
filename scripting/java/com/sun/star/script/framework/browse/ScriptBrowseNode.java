/*************************************************************************
 *
 *  $RCSfile: ScriptBrowseNode.java,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-19 08:20:19 $
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

import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;
import drafts.com.sun.star.script.provider.XScriptContext;

import com.sun.star.beans.PropertyAttribute;
import com.sun.star.lib.uno.helper.PropertySet;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;

import com.sun.star.beans.XIntrospectionAccess;
import com.sun.star.script.XInvocation;

import java.io.File;
import java.util.*;
import javax.swing.JOptionPane;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.ScriptProvider;
import com.sun.star.script.framework.browse.DialogFactory;
import com.sun.star.script.framework.container.*;

public class ScriptBrowseNode extends PropertySet
    implements XBrowseNode, XInvocation
{
    private ScriptProvider provider;

    private Parcel parent;
    private String name;
    public String uri;
    public String description;
    public boolean editable  = false;
    public boolean deletable = false;
    public boolean renamable = false;

    public ScriptBrowseNode( ScriptProvider provider, Parcel parent,
        String name )
    {
        this.provider = provider;
        this.name = name;
        this.parent = parent;
        ScriptMetaData data = null;
        try
        {
            data = (ScriptMetaData)parent.getByName( name );
        }

        // TODO fix exception types to be caught here, should we rethrow?
        catch (  Exception e )
        {
            LogUtils.DEBUG("** caught exception getting script data for " + name + " ->" + e.toString() );
        }

        uri = data.getShortFormScriptURL();
        description = data.getDescription();

        if (provider.hasScriptEditor() == true)
        {
            this.editable  = true;
            this.deletable = true;
            this.renamable = true;
        }

        registerProperty("Deletable", new Type(boolean.class),
            (short)0, "deletable");
        registerProperty("Editable", new Type(boolean.class),
            (short)0, "editable");
        registerProperty("Renamable", new Type(boolean.class),
            (short)0, "renamable");
        registerProperty("URI", new Type(String.class),
            (short)0, "uri");
        registerProperty("DESCRIPTION", new Type(String.class),
            (short)0, "description");
    }


    public String getName() {
        return name;
    }

    public XBrowseNode[] getChildNodes() {
        return new XBrowseNode[0];
    }

    public boolean hasChildNodes() {
        return false;
    }

    public short getType() {
        return BrowseNodeTypes.SCRIPT;
    }

    public String toString() {
        return getName();
    }

    public void updateURI( Parcel p ) {
        parent = p;
        ScriptMetaData data = null;
        try
        {
            data = (ScriptMetaData)parent.getByName( name );
        }

        // TODO fix exception types to be caught here, should we rethrow?
        catch (  Exception e )
        {
            LogUtils.DEBUG("** caught exception getting script data for " + name + " ->" + e.toString() );
        }
        uri = data.getShortFormScriptURL();
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

        if (aFunctionName.equals("Editable"))
        {
            if (!editable)
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Script not editable");
            }


            XScriptContext ctxt =  provider.getScriptingContext();
            ScriptMetaData data = null;
            try
            {
                data = (ScriptMetaData)parent.getByName( name );
            }
            // TODO fix exception types to be caught here
            catch (  Exception e )
            {
                LogUtils.DEBUG("** caught exception getting script data for " + name + " ->" + e.toString() );
                throw new  com.sun.star.lang.IllegalArgumentException( "no script data for " + name );
            }

            provider.getScriptEditor().edit(ctxt, data);
        }
        else if (aFunctionName.equals("Deletable"))
        {
            if (!deletable)
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Script not editable");
            }


            try
            {
                boolean goAhead = true;

                /* prompting in svx/source/dialogs/scriptdlg.cxx
                String prompt = "Do you really want to delete this Script?";
                String title = "Delete Script";

                // try to get a DialogFactory instance, if it fails
                // just use a Swing JOptionPane to prompt for the name
                try
                {
                    DialogFactory dialogFactory =
                        DialogFactory.getDialogFactory();

                    goAhead = dialogFactory.showConfirmDialog(title, prompt);
                }
                catch (Exception e)
                {
                    int reply = JOptionPane.showConfirmDialog(
                        null, prompt, title, JOptionPane.YES_NO_OPTION);

                    if (reply == JOptionPane.YES_OPTION)
                    {
                        goAhead = true;
                    }
                    else
                    {
                        goAhead = false;
                    }
                } */

                if (goAhead == true)
                {
                    parent.removeByName( name );
                    result = new Any(new Type(Boolean.class), Boolean.TRUE);
                }
                else
                {
                    result = new Any(new Type(Boolean.class), Boolean.FALSE);
                }
            }
            // TODO Exception handling TBD
            catch (Exception e)
            {
                LogUtils.DEBUG("** caught exception removing " + name + " ->" + e.toString() );
                result = new Any(new Type(Boolean.class), Boolean.FALSE);

                // throw new com.sun.star.reflection.InvocationTargetException(
                //     "Error deleting script: " + e.getMessage());
            }
        }
        else if (aFunctionName.equals("Renamable"))
        {
            if (!renamable)
            {
                throw new com.sun.star.reflection.InvocationTargetException(
                    "Script not renamable");
            }


            try
            {
                boolean goAhead = true;

                if (goAhead == true)
                {
                    String newName = (String) AnyConverter.toString(aParams[0]);
                    ScriptMetaData oldData = (ScriptMetaData)parent.getByName( name );
                    String oldSource = oldData.getSource();
                    LogUtils.DEBUG("remove old script");
                    parent.removeByName( name );
                    LogUtils.DEBUG("now create renamed script");
                    String languageName = newName + "." + provider.getScriptEditor().getExtension();
                    String language = provider.getName();

                    ScriptEntry entry = new ScriptEntry( language, languageName, languageName, "", new HashMap() );

                    ScriptMetaData data = new ScriptMetaData( parent, entry, oldSource );
                    parent.insertByName( languageName, data );
                    uri = data.getShortFormScriptURL();
                    name = languageName;
                    result = new Any(new Type(XBrowseNode.class), this);
                }
                else
                {
                    result = new Any(new Type(Boolean.class), Boolean.FALSE);
                }
            }
            // TODO Exception handling TBD
            catch (Exception e)
            {
                LogUtils.DEBUG("** caught exception removing " + name + " ->" + e.toString() );
                result = new Any(new Type(Boolean.class), Boolean.FALSE);

                // throw new com.sun.star.reflection.InvocationTargetException(
                //     "Error deleting script: " + e.getMessage());
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

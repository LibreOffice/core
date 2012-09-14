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

package com.sun.star.wizards.common;

import java.util.ArrayList;
import java.util.List;

import com.sun.star.beans.Property;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.GlobalTransferCommandArgument;
import com.sun.star.ucb.NameClash;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.ucb.TransferCommandOperation;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XContentAccess;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XDynamicResultSet;
import com.sun.star.uno.UnoRuntime;

/**
 * @author rpiterman
 * This class is used to copy the content of a folder to
 * another folder.
 * There is an incosistency with argument order.
 * It should be always: dir,filename.
 */
public class UCB
{

    private Object ucb;
    private FileAccess fa;
    
    public UCB(XMultiServiceFactory xmsf) throws Exception
    {
        ucb = xmsf.createInstanceWithArguments(                
            "com.sun.star.ucb.UniversalContentBroker", new Object[0] );
        fa = new FileAccess(xmsf);
    }
    
    public void deleteDirContent(String dir) 
        throws Exception
    {
        if (!fa.exists(dir,true))
        {
          return;
        }
        List<String> l = listFiles(dir,null);
        for (int i = 0; i<l.size(); i++)
        {
            delete(FileAccess.connectURLs(dir ,l.get(i)));
    }
    }
    
    public void delete(String filename) throws Exception
    {
        //System.out.println("UCB.delete(" + filename);
        executeCommand( getContent(filename),"delete",Boolean.TRUE);
    }
    
    public void copy(String sourceDir, String targetDir) throws Exception
    {
        copy(sourceDir,targetDir,(Verifier)null);
    }
    
    public void copy(String sourceDir, String targetDir, Verifier verifier) throws Exception
    {
        List<String> files = listFiles(sourceDir,verifier);
        for (int i = 0; i<files.size(); i++)
        {
          copy(sourceDir, files.get(i), targetDir);
        }
        
    }
    
    public void copy(String sourceDir, String filename, String targetDir, String targetName) throws Exception
    {
        if (!fa.exists(targetDir,true))
        {
          fa.fileAccess.createFolder(targetDir);
        }
        //System.out.println("UCB.copy(" + sourceDir + ", " + filename +  ", " + targetDir+ ", " + targetName);
        executeCommand(ucb, "globalTransfer", copyArg(sourceDir,filename, targetDir,targetName));
    }
    
    /**
     * @deprecated
     * @param sourceDir
     * @param filename
     * @param targetDir
     * @throws Exception
     */
    public void copy(String sourceDir, String filename, String targetDir) throws Exception
    {
        copy(sourceDir,filename, targetDir, PropertyNames.EMPTY_STRING);
    }

    /**
     * target name can be PropertyNames.EMPTY_STRING, in which case the name stays lige the source name
     * @param sourceDir
     * @param sourceFilename
     * @param targetDir
     * @param targetFilename
     * @return
     */
    public GlobalTransferCommandArgument copyArg(String sourceDir, String sourceFilename, String targetDir, String targetFilename)
    {
        
        GlobalTransferCommandArgument aArg = new GlobalTransferCommandArgument();
        aArg.Operation = TransferCommandOperation.COPY;
        aArg.SourceURL = fa.getURL(sourceDir,sourceFilename);
        aArg.TargetURL = targetDir;
        aArg.NewTitle = targetFilename;
        // fail, if object with same name exists in target folder
        aArg.NameClash = NameClash.OVERWRITE;
        return aArg;
    }
    
    public Object executeCommand(Object xContent, String aCommandName, Object aArgument)
        throws com.sun.star.ucb.CommandAbortedException, 
            com.sun.star.uno.Exception 
    {
        XCommandProcessor xCmdProcessor = UnoRuntime.queryInterface(
            XCommandProcessor.class, xContent);
        Command aCommand  = new Command();
        aCommand.Name     = aCommandName;
        aCommand.Handle   = -1; // not available
        aCommand.Argument = aArgument;
        return xCmdProcessor.execute(aCommand, 0, null);
    }

    public List<String> listFiles(String path, Verifier verifier) throws Exception
    {
        Object xContent = getContent(path);

        OpenCommandArgument2 aArg = new OpenCommandArgument2();
        aArg.Mode = OpenMode.ALL;        
        aArg.Priority = 32768;        

        // Fill info for the properties wanted.
        aArg.Properties = new Property[] {new Property()};
        
        aArg.Properties[0].Name = PropertyNames.PROPERTY_TITLE;
        aArg.Properties[0].Handle = -1;
        
        XDynamicResultSet xSet;

        xSet = UnoRuntime.queryInterface(
          XDynamicResultSet.class,executeCommand(xContent, "open", aArg));

        XResultSet xResultSet = xSet.getStaticResultSet();
        
        List<String> files = new ArrayList<String>();

        if (xResultSet.first())
        {
            // obtain XContentAccess interface for child content access and XRow for properties
            XContentAccess xContentAccess = UnoRuntime.queryInterface(
                XContentAccess.class, xResultSet);
            XRow xRow = UnoRuntime.queryInterface(XRow.class, xResultSet);
            do
            {
                // Obtain URL of child.
                String aId = xContentAccess.queryContentIdentifierString();
                // First column: Title (column numbers are 1-based!)
                String aTitle = xRow.getString(1);
                if (aTitle.length() == 0 && xRow.wasNull())
                {
                    //ignore
                }
                else 
                {
                    files.add(aTitle);
                }
            }
            while (xResultSet.next()); // next child
        }
        
        if (verifier != null)
        {
            for (int i = 0; i<files.size(); i++) 
            {
                if (!verifier.verify(files.get(i)))
                {
                    files.remove(i--);
                }
            }
        }
        
        return files;
    }
    
    public Object getContentProperty(Object content, String propName, Class<?> type)
        throws Exception 
    {
        Property[] pv = new Property[1];
        pv[0] = new Property();
        pv[0].Name = propName;
        pv[0].Handle = -1;
    
        Object row = executeCommand(content,"getPropertyValues",pv);
        XRow xrow = UnoRuntime.queryInterface(XRow.class,row);
        if (type.equals(String.class))
        {
           return xrow.getString(1);
        }
        else if (type.equals(Boolean.class))
        {
            return xrow.getBoolean(1) ? Boolean.TRUE : Boolean.FALSE;
        }
        else if (type.equals(Integer.class))
        {
            return new Integer(xrow.getInt(1));
        }
        else if (type.equals(Short.class))
        {
            return new Short(xrow.getShort(1));
        }
        else
        {
            return null;
        }
        
    }
    
    public Object getContent(String path) throws Exception
    {
        //System.out.println("Getting Content for : " + path);
        XContentIdentifier id = UnoRuntime.queryInterface(XContentIdentifierFactory.class, ucb).createContentIdentifier(path);
          
        return UnoRuntime.queryInterface(
          XContentProvider.class,ucb).queryContent(id);
    }
    
    public static interface Verifier
    {

        public boolean verify(Object object); 
    }
}

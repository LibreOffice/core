/*************************************************************************
 *
 *  $RCSfile: ConfigHelper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 17:32:10 $
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
 **********************************************************************_*/

package com.sun.star.filter.config.tools.utils;

//_______________________________________________
// imports

import java.lang.*;
import java.util.*;
import java.io.*;

//_______________________________________________
// definition

/** can be used to analyze command line parameters
 *  and merge it together with might existing config
 *  files. That provides the possibility to overwrite
 *  config values via command line parameter.
 *
 *
 */
public class ConfigHelper extends java.util.Properties
{
    //___________________________________________
    // member

    /** indicates an empty command line. */
    private boolean m_bEmpty = true;

    //___________________________________________
    // ctor

    //-------------------------------------------
    /** initialize a new helper with the list of
     *  command line parameters and bind this new instance
     *  to a property file on disk.
     *
     *  @param  sPropFile
     *          name of the property file.
     *          If its set to null or an empty value
     *          it will be ignored.
     *
     *  @param  lCommandLineArgs
     *          the list of original command line arguments.
     *
     *  @throws [Exception]
     *          in case the command line contains an unknown
     *          schema for specifiying parameters or the
     *          specified property file does not exists
     *          or seem to be corrupted.
     */
    public ConfigHelper(java.lang.String   sPropFile       ,
                        java.lang.String[] lCommandLineArgs)
        throws java.lang.Exception
    {
        // first load prop file, so its values can be overwritten
        // by command line args later
        // Do it only, if a valid file name was given.
        // But in case this file name is wrong, throw an exception.
        // So the outside code can react!
        if (
            (sPropFile          != null) &&
            (sPropFile.length() >  0   )
           )
        {
            java.lang.ClassLoader aLoader = getClass().getClassLoader();
            java.io.InputStream   aStream = aLoader.getResourceAsStream(sPropFile);
            if (aStream == null)
                aStream = new java.io.FileInputStream(sPropFile);
            load(aStream);
        }

        int count = 0;
        if (lCommandLineArgs != null)
            count = lCommandLineArgs.length;
        m_bEmpty = (count < 1);

        for (int arg=0; arg<count; ++arg)
        {
            // is it a named-value argument?
            // Note: We ignores double "=" signs! => search from left to right
            int len = lCommandLineArgs[arg].length();
            int pos = lCommandLineArgs[arg].indexOf('=');
            if (pos != -1)
            {
                java.lang.String sArg   = lCommandLineArgs[arg].substring(0,pos);
                java.lang.String sValue = lCommandLineArgs[arg].substring(pos+1);
                setProperty(sArg, sValue);
                continue;
            }

            // is it a boolean argument?
            // Note: Because "--" and "-" will be interpreted as the same
            // we search from right to left!
            pos = lCommandLineArgs[arg].lastIndexOf('-');
            if (pos == -1)
                pos = lCommandLineArgs[arg].lastIndexOf('/');
            if (pos != -1)
            {
                java.lang.String sArg = lCommandLineArgs[arg].substring(pos+1);
                setProperty(sArg, java.lang.String.valueOf(true));
                continue;
            }

            // There is an unknown format used by this argument ...
            throw new MalformedCommandLineException("Invalid command line detected. The argument \""+lCommandLineArgs[arg]+"\" use an unsupported format.");
        }
    }

    //-------------------------------------------
    /** indicates if the given command line includes
     *  a help request.
     *
     *  @return True if there was an explicit help request.
     */
    public synchronized boolean isHelp()
    {
        return (
                (containsKey("help")) ||
                (containsKey("?")   ) ||
                (containsKey("h")   )
               );
    }

    //-------------------------------------------
    /** indicates if the gioven command line was empty.
     *
     *  @return True if there was an empty command line.
     */
    public synchronized boolean isEmpty()
    {
        return m_bEmpty;
    }

    //-------------------------------------------
    /** returns the value of sProp as boolean value.
     *
     *  @param  sProp
     *          the name of the parameter.
     *
     *  @return The boolean value of the requested property.
     *
     *  @throw  [NoSuchElementException]
     *          if the requested property does not exists.
     */
    public synchronized boolean getBoolean(java.lang.String sProp)
        throws java.util.NoSuchElementException
    {
        java.lang.String sValue = getProperty(sProp);
        if (sValue == null)
            throw new java.util.NoSuchElementException("The requested config value \""+sProp+"\" does not exists!");
        return new java.lang.Boolean(sValue).booleanValue();
    }

    //-------------------------------------------
    /** returns the value of sProp as int value.
     *
     *  @param  sProp
     *          the name of the parameter.
     *
     *  @return The int value of the requested property.
     *
     *  @throw  [NoSuchElementException]
     *          if the requested property does not exists.
     */
    public synchronized int getInt(java.lang.String sProp)
        throws java.util.NoSuchElementException
    {
        java.lang.String sValue = getProperty(sProp);
        if (sValue == null)
            throw new java.util.NoSuchElementException("The requested config value \""+sProp+"\" does not exists!");
        return new java.lang.Integer(sValue).intValue();
    }

    public synchronized int getInt(java.lang.String sProp   ,
                                   int              nDefault)
    {
        java.lang.String sDefault = java.lang.String.valueOf(nDefault);
        java.lang.String sValue   = getProperty(sProp, sDefault);
        return new java.lang.Integer(sValue).intValue();
    }

    //-------------------------------------------
    /** returns the value of sProp as string value.
     *
     *  @param  sProp
     *          the name of the parameter.
     *
     *  @return The string value of the requested property.
     *
     *  @throw  [NoSuchElementException]
     *          if the requested property does not exists.
     */
    public synchronized java.lang.String getString(java.lang.String sProp)
        throws java.util.NoSuchElementException
    {
        java.lang.String sValue = getProperty(sProp);
        if (sValue == null)
            throw new java.util.NoSuchElementException("The requested config value \""+sProp+"\" does not exists!");
        return sValue;
    }

    //-------------------------------------------
    /** returns the value of sProp as string list value!
     *
     *  @descr  The delimiter must be well known and
     *          it must be clear if trailing/leading
     *          whitespaces must be ignored or not.
     *
     *  @param  sProp
     *          the name of the parameter.
     *
     *  @param  sDelim
     *          the delimiter, which must be used to split
     *          the config string value into an array.
     *
     *  @param  bTrim
     *          if its set to true, trailing and leading whitespace
     *          characters will be ommited.
     *
     *  @param  bDecode
     *          if its set to TRUE all liste items will be
     *          interpreted as "<xxx>" and converted to <xxx>!
     *
     *  @return The string list value of the requested property.
     *
     *  @throw  [NoSuchElementException]
     *          if the requested property does not exists.
     */
    public synchronized java.util.Vector getStringList(java.lang.String sProp     ,
                                                       java.lang.String sDelimiter,
                                                       boolean          bTrim     ,
                                                       boolean          bDecode   )
        throws java.util.NoSuchElementException
    {
        java.lang.String sValue = getProperty(sProp);
        if (sValue == null)
            throw new java.util.NoSuchElementException("The requested config value \""+sProp+"\" does not exists!");

        java.util.Vector lValue = new java.util.Vector();
        try
        {
            java.util.StringTokenizer lTokens = new java.util.StringTokenizer(sValue, sDelimiter);
            while(lTokens.hasMoreTokens())
            {
                java.lang.String sToken = lTokens.nextToken();
                // remove trailing/leading whitespaces
                if (bTrim)
                    sToken = sToken.trim();
                // remove ""
                if (
                    (bDecode                                      ) &&
                    (sToken.indexOf("\"")     == 0                ) &&
                    (sToken.lastIndexOf("\"") == sToken.length()-1)
                   )
                {
                    sToken = sToken.substring(1, sToken.length()-1);
                }
                lValue.add(sToken);
            }
        }
        catch(java.lang.Throwable ex)
            { lValue.clear(); }

        return lValue;
    }
}

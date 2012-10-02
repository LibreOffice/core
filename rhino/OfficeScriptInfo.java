/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


package org.mozilla.javascript.tools.debugger;
import java.net.URL;
import java.util.Hashtable;
import org.mozilla.javascript.Scriptable;

public class OfficeScriptInfo
{
    private Hashtable loadedSFScripts = new Hashtable();

    public void addScript( URL url, Scriptable scope, Runnable closeCallback )
    {
        addScript( url.toString(), url, scope, closeCallback );
    }

    public void addScript( String key, URL url, Scriptable scope, Runnable closeCallback )
    {
        SFScriptInfo si = (SFScriptInfo)loadedSFScripts.get( key );
        if ( si == null )
        {
            si = new SFScriptInfo();
            si.url = url;
            si.scope = scope;
            si.closeCallback = closeCallback;
            loadedSFScripts.put( key, si );
        }
    }

    public void deleteScript( String key )
    {
        SFScriptInfo info = (SFScriptInfo)loadedSFScripts.remove( key );
        if ( info != null )
        {
            if ( info.closeCallback != null )
            {
                System.out.println("** In removeSFScriptInfo  have callback for " + key );
                info.closeCallback.run(); // really need to do this in seperate thread????
            }
        }
    }

    public Scriptable getScriptScope( String key )
    {
        Scriptable result = null;
        SFScriptInfo info = (SFScriptInfo)loadedSFScripts.get( key );
        if ( info != null )
        {
            result = info.scope;
        }
        return result;
    }

    public URL getScriptUrl( String key )
    {
        URL result = null;
        SFScriptInfo info = (SFScriptInfo)loadedSFScripts.get( key );
        if ( info != null )
        {
            result = info.url;
        }
        return result;
    }
    public boolean hasScript( String key )
    {
        boolean result = true;
        SFScriptInfo info = (SFScriptInfo)loadedSFScripts.get( key );
        if ( info == null )
        {
            result = false;
        }
        return result;
    }

    public void setScriptRunning( String key, boolean running )
    {
        SFScriptInfo info = (SFScriptInfo)loadedSFScripts.get( key );
        if ( info != null )
        {
            info.isExecuting = running;
        }
    }

    public boolean isScriptRunning( String key )
    {
        boolean result = false;
        SFScriptInfo info = (SFScriptInfo)loadedSFScripts.get( key );
        if ( info != null )
        {
            result = info.isExecuting;
        }
        return result;
    }

    class SFScriptInfo
    {
        Scriptable scope;
        boolean isExecuting;
        URL url;
        Runnable closeCallback;
    }
}

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

package org.mozilla.javascript.tools.debugger;
import java.net.URL;
import java.util.HashMap;
import org.mozilla.javascript.Scriptable;

public class OfficeScriptInfo
{
    private HashMap<String, SFScriptInfo> loadedSFScripts = new HashMap<String, SFScriptInfo>();

    public void addScript( URL url, Scriptable scope, Runnable closeCallback )
    {
        addScript( url.toString(), url, scope, closeCallback );
    }

    public void addScript( String key, URL url, Scriptable scope, Runnable closeCallback )
    {
        SFScriptInfo si = loadedSFScripts.get( key );
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
        SFScriptInfo info = loadedSFScripts.remove( key );
        if ( info != null )
        {
            if ( info.closeCallback != null )
            {
                System.out.println("** In removeSFScriptInfo  have callback for " + key );
                info.closeCallback.run(); // really need to do this in separate thread????
            }
        }
    }

    public Scriptable getScriptScope( String key )
    {
        Scriptable result = null;
        SFScriptInfo info = loadedSFScripts.get( key );
        if ( info != null )
        {
            result = info.scope;
        }
        return result;
    }

    public URL getScriptUrl( String key )
    {
        URL result = null;
        SFScriptInfo info = loadedSFScripts.get( key );
        if ( info != null )
        {
            result = info.url;
        }
        return result;
    }
    public boolean hasScript( String key )
    {
        boolean result = true;
        SFScriptInfo info = loadedSFScripts.get( key );
        if ( info == null )
        {
            result = false;
        }
        return result;
    }

    public void setScriptRunning( String key, boolean running )
    {
        SFScriptInfo info = loadedSFScripts.get( key );
        if ( info != null )
        {
            info.isExecuting = running;
        }
    }

    public boolean isScriptRunning( String key )
    {
        boolean result = false;
        SFScriptInfo info = loadedSFScripts.get( key );
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

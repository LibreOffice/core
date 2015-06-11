/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package org.apache.openoffice.ooxml.framework.part;

/** Operations around part names.
 */
public class PartName
    implements Comparable<PartName>
{
    public PartName (final String sPath)
    {
        if ( ! (sPath.isEmpty() || sPath.startsWith("/")))
        {
            assert(sPath.isEmpty() || sPath.startsWith("/"));
        }
        assert(sPath.indexOf('\\') == -1);

        msPath = sPath;
    }




    public PartName (
        final String sPath,
        final PartName aParentName,
        final String sMode)
    {
        switch(sMode)
        {
            case "External":
                msPath = sPath;
                break;

            case "Internal":
                msPath = Cleanup(aParentName.GetPathname() + "/" + sPath);
                break;

            default:
                throw new RuntimeException();
        }
    }




    public PartName getRelationshipsPartName ()
    {
        return new PartName(GetPathname() + "/_rels/" + GetBasename() + ".rels");
    }




    private String GetPathname ()
    {
        if (msPath.isEmpty())
            return "";
        else
        {
            final int nPathnameEnd = msPath.lastIndexOf('/');
            assert(nPathnameEnd>=0);
            return msPath.substring(0, nPathnameEnd);
        }
    }




    public String GetBasename ()
    {
        if (msPath.isEmpty())
            return "";
        else
        {
            final int nBasenameStart = msPath.lastIndexOf('/');
            assert(nBasenameStart>=0);
            return msPath.substring(nBasenameStart+1);
        }
    }




    public String GetExtension ()
    {
        final int nExtensionStart = msPath.lastIndexOf('.');
        if (nExtensionStart < 0)
            return null;
        else
            return msPath.substring(nExtensionStart+1);
    }




    public String GetFullname()
    {
        return msPath;
    }




    @Override
    public int compareTo (final PartName aOther)
    {
        return msPath.compareTo(aOther.msPath);
    }




    private String Cleanup (final String sName)
    {
        return sName.replaceAll("/[^/]+/\\.\\./", "/");
    }




    @Override
    public String toString ()
    {
        return msPath;
    }




    private final String msPath;
}

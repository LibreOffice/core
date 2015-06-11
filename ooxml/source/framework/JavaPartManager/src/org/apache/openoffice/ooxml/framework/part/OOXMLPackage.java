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

import java.io.File;
import java.io.InputStream;

public class OOXMLPackage
    extends Package
{
    public static OOXMLPackage Create (final File aOOXMLFile)
    {
        return new OOXMLPackage(
            aOOXMLFile.getAbsolutePath(),
            new PartManager(aOOXMLFile));
    }




    private OOXMLPackage (final String sPath, final PartManager aPartManager)
    {
        super(sPath, aPartManager);
    }




    /** Return a list of stream names.
     *  Note that that list is not necessarily identical to the list of part
     *  names.  It can contain entries that are not parts.
     */
    public String[] listStreamNames ()
    {
        return maPartManager.listStreamNames();
    }




    /** Return an InputStream object for the specified stream.
     */
    public InputStream getStream (final String sStreamName)
    {
        return maPartManager.getStreamForStreamName(sStreamName);
    }




    public Part getPart (final PartName aPartName)
    {
        return new Part (
            maPartManager.getContentTypes().getTypeForPartName(aPartName),
            maPartManager,
            aPartName);
    }
}

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

import org.apache.openoffice.ooxml.framework.part.parser.ParserFactory;
import org.apache.openoffice.ooxml.parser.Log;

public class PartManagerPrototype
{
    public static void main (final String ... aArgumentList)
    {
        if (aArgumentList.length != 3)
        {
            System.err.printf("usage: PartManagerPrototype <ooxml-file-name> <parser-table-filename> <log-filename>");
            System.exit(1);
        }

        final long nStartTime = System.currentTimeMillis();

        Log.Dbg = new Log(aArgumentList[2]);
        ParserFactory.SetParserTableFilename(aArgumentList[1]);

        final File aOOXMLFile = new File(aArgumentList[0]);
        final Part aPart = OOXMLPackage.Create(aOOXMLFile).getOfficeDocumentPart().getPartById("rId1");

        final long nEndTime = System.currentTimeMillis();

        System.out.printf("got content type %s for %s in %fs\n",
            aPart.getContentType(),
            aPart.getPartName(),
            (nEndTime-nStartTime)/1000.0);
    }
}

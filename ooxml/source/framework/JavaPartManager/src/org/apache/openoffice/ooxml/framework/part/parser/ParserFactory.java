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

package org.apache.openoffice.ooxml.framework.part.parser;

import java.io.File;
import java.io.InputStream;
import java.util.Vector;

import org.apache.openoffice.ooxml.framework.part.ContentType;
import org.apache.openoffice.ooxml.parser.Parser;
import org.apache.openoffice.ooxml.parser.StateMachine;

public class ParserFactory
{
    public static Parser getParser (
        final ContentType eType,
        final InputStream aStream,
        final Vector<String> aErrorsAndWarnings)
    {
        switch(eType)
        {
            case Relationships:
                return new RelationshipParser(aStream, msParserTableFilename, aErrorsAndWarnings);

            case ContentTypes:
                return new ContentTypesParser(aStream, msParserTableFilename, aErrorsAndWarnings);

            default:
                return new Parser(
                    new StateMachine(new File(msParserTableFilename), aErrorsAndWarnings),
                    aStream);
        }
    }




    public static void SetParserTableFilename (final String sFilename)
    {
        assert(new File(sFilename).exists());
        msParserTableFilename = sFilename;
    }




    private static String msParserTableFilename = null;
}

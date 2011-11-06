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



package com.sun.star.help;

import java.io.File;
import java.io.Reader;
import java.io.FileInputStream;
import java.io.InputStreamReader;
//import java.io.FileReader;
import java.io.StringReader;

import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;

/** Lucene Document for help files */
public class HelpFileDocument
{
    /** Creates reader for UTF-8 files
    */
    private static Reader getReaderForFile( File aFile )
        throws java.io.FileNotFoundException, java.io.UnsupportedEncodingException {
        Reader aReader;
        if( aFile != null ) {
            FileInputStream fis = new FileInputStream( aFile );
            aReader = new InputStreamReader( fis, "UTF-8" );
        }
        else {
            aReader = new StringReader( "" );
        }
        return aReader;
    }

    /** Makes a document for a File.
    */
    public static Document Document( String aModule, File aCaptionFile, File aContentFile )
        throws java.io.FileNotFoundException, java.io.UnsupportedEncodingException {
        Document doc = new Document();

        // Add the path of the file as a field named "path".  Use a field that is
        // indexed (i.e. searchable), but don't tokenize the field into words.
        File aFile = aCaptionFile != null ? aCaptionFile : aContentFile;
        if( aFile != null )
        {
            String aPath = "#HLP#" + aModule + "/" + aFile.getName();
            doc.add(new Field("path", aPath, Field.Store.YES, Field.Index.UN_TOKENIZED));
        }

        // Add the caption of the file to a field named "caption".  Specify a Reader,
        // so that the text of the file is tokenized and indexed, but not stored.
        doc.add( new Field( "caption", getReaderForFile( aCaptionFile ) ) );

        // Add the contents of the file to a field named "content".  Specify a Reader,
        // so that the text of the file is tokenized and indexed, but not stored.
        doc.add( new Field( "content", getReaderForFile( aContentFile ) ) );

        // return the document
        return doc;
    }

    private HelpFileDocument() {}
}

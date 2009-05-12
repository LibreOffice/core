/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: HelpFileDocument.java,v $
 * $Revision: 1.2 $
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

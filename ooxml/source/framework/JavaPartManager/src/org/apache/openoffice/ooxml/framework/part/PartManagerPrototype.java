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

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

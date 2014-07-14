package org.apache.openoffice.ooxml.framework.part.parser;

import java.io.File;
import java.io.InputStream;
import java.util.Vector;

import org.apache.openoffice.ooxml.parser.Parser;
import org.apache.openoffice.ooxml.parser.StateMachine;

public class RelationshipParser
    extends Parser
{
    public RelationshipParser (
        final InputStream aIn,
        final String sParserTableFilename,
        final Vector<String> aErrorsAndWarnings)
    {
        super(CreateStateMachine(sParserTableFilename, aErrorsAndWarnings), aIn);
    }




    private static StateMachine CreateStateMachine (
        final String sParserTableFilename,
        final Vector<String> aErrorsAndWarnings)
    {
        return new StateMachine(new File(sParserTableFilename), aErrorsAndWarnings);
    }
}

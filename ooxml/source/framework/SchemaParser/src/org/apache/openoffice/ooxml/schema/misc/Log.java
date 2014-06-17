package org.apache.openoffice.ooxml.schema.misc;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;

/** Make output with indentation easier.
 */
public class Log
{
    public Log (final File aFile)
    {
        PrintStream aOut = null;
        try
        {
            aOut = new PrintStream(new FileOutputStream(aFile));
        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        }
        maOut = aOut;
        mbIsActive = maOut!=null;
        msIndentation = "";
    }




    public void AddComment (
        final String sFormat,
        final Object ... aArgumentList)
    {
        if (mbIsActive)
        {
            maOut.print(msIndentation);
            maOut.print("// ");
            maOut.printf(sFormat, aArgumentList);
            maOut.print("\n");
        }
    }




    public void StartBlock ()
    {
        if (mbIsActive)
            msIndentation += "    ";
    }




    public void EndBlock ()
    {
        if (mbIsActive)
            msIndentation = msIndentation.substring(4);
    }




    public void printf (
        final String sFormat, final Object ... aArgumentList)
    {
        if (mbIsActive)
        {
            final String sMessage = String.format(sFormat, aArgumentList);
            maOut.print(msIndentation);
            maOut.print(sMessage);
        }
    }




    public void println (
        final String sMessage)
    {
        if (mbIsActive)
        {
            maOut.print(msIndentation);
            maOut.print(sMessage);
            maOut.print("\n");
        }
    }




    public void Close()
    {
        if (mbIsActive)
            maOut.close();
    }




    private final PrintStream maOut;
    private final boolean mbIsActive;
    private String msIndentation;
}

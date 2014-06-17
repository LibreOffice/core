package org.apache.openoffice.ooxml.parser;

import java.io.InputStream;

import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

import org.apache.openoffice.ooxml.parser.action.ActionManager;
import org.apache.openoffice.ooxml.parser.attribute.AttributeProvider;

/** This is the actual parser (where OOXMLParser is the front end that handles
 *  parameters given to the main method).
 */
public class Parser
{
    public Parser (
        final StateMachine aMachine,
        final InputStream aIn)
    {
        maMachine = aMachine;
        maReader = GetStreamReader(aIn, "input");
        mnElementCount = 0;
    }




    public void Parse ()
    {
        try
        {
            final AttributeProvider aAttributeProvider = new AttributeProvider(maReader);
            while (maReader.hasNext())
            {
                final int nCode = maReader.next();
                switch(nCode)
                {
                    case XMLStreamReader.START_ELEMENT:
                        ++mnElementCount;
                        if (maMachine.IsInSkipState())
                        {
                            if (Log.Dbg != null)
                                Log.Dbg.printf("is skip state -> starting to skip\n");
                            Skip();
                        }
                        else if ( ! maMachine.ProcessStartElement(
                            maReader.getNamespaceURI(),
                            maReader.getLocalName(),
                            maReader.getLocation(),
                            aAttributeProvider))
                        {
                            if (Log.Dbg != null)
                                Log.Dbg.printf("starting to skip to recover from error\n");
                            Skip();
                        }
                        break;

                    case XMLStreamReader.END_ELEMENT:
                        maMachine.ProcessEndElement(
                            maReader.getNamespaceURI(),
                            maReader.getLocalName(),
                            maReader.getLocation());
                        break;

                    case XMLStreamReader.CHARACTERS:
                        maMachine.ProcessCharacters(
                            maReader.getText(),
                            maReader.getLocation());
                        break;

                    case XMLStreamReader.END_DOCUMENT:
                        break;

                    default:
                        Log.Err.printf("can't handle XML event of type %d\n", nCode);
                }
            }

            maReader.close();
        }
        catch (final XMLStreamException aException)
        {
            aException.printStackTrace();
        }
    }




    public int GetElementCount ()
    {
        return mnElementCount;
    }




    private void Skip ()
    {
        if (Log.Dbg != null)
        {
            Log.Dbg.printf("starting to skip on %s at L%dC%d\n",
                maReader.getLocalName(),
                maReader.getLocation().getLineNumber(),
                maReader.getLocation().getColumnNumber());
            Log.Dbg.IncreaseIndentation();
        }

        // We are called when processing a start element.  This means that we are
        // already at relative depth 1.
        int nRelativeDepth = 1;
        try
        {
            while (maReader.hasNext())
            {
                final int nCode = maReader.next();
                switch (nCode)
                {
                    case XMLStreamReader.START_ELEMENT:
                        ++nRelativeDepth;
                        ++mnElementCount;
                        if (Log.Dbg != null)
                        {
                            Log.Dbg.printf("skipping start element %s\n", maReader.getLocalName());
                            Log.Dbg.IncreaseIndentation();
                        }
                        break;

                    case XMLStreamReader.END_ELEMENT:
                        --nRelativeDepth;
                        if (Log.Dbg != null)
                            Log.Dbg.DecreaseIndentation();
                        if (nRelativeDepth <= 0)
                        {
                            if (Log.Dbg != null)
                                Log.Dbg.printf("leaving skip mode on %s\n", maReader.getLocalName());
                            return;
                        }
                        break;

                    case XMLStreamReader.END_DOCUMENT:
                        throw new RuntimeException("saw end of document while skipping elements\n");

                    case XMLStreamReader.CHARACTERS:
                        SkipText(maReader.getText());
                        break;

                    default:
                        if (Log.Dbg != null)
                            Log.Dbg.printf("%s\n",  nCode);
                        break;
                }
            }
        }
        catch (final XMLStreamException aException)
        {
            aException.printStackTrace();
        }
    }




    private void SkipText (final String sText)
    {
        if (Log.Dbg != null)
            Log.Dbg.printf("skipping text [%s]\n", sText.replace("\n", "\\n"));
    }




    private XMLStreamReader GetStreamReader (
        final InputStream aIn,
        final String sDescription)
    {
        if (aIn == null)
            return null;

        try
        {
            final XMLInputFactory aFactory = (XMLInputFactory)XMLInputFactory.newInstance();
            aFactory.setProperty(XMLInputFactory.IS_REPLACING_ENTITY_REFERENCES, false);
            aFactory.setProperty(XMLInputFactory.IS_SUPPORTING_EXTERNAL_ENTITIES, false);
            aFactory.setProperty(XMLInputFactory.IS_COALESCING, false);

            return (XMLStreamReader)aFactory.createXMLStreamReader(
                sDescription,
                aIn);
        }
        catch (final Exception aException)
        {
            aException.printStackTrace();
            return null;
        }
    }




    public ActionManager GetActionManager()
    {
        return maMachine.GetActionManager();
    }




    private final XMLStreamReader maReader;
    private final StateMachine maMachine;
    private int mnElementCount;
}

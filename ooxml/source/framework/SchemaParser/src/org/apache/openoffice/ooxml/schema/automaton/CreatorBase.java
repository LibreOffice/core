package org.apache.openoffice.ooxml.schema.automaton;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.iterator.AttributeIterator;
import org.apache.openoffice.ooxml.schema.iterator.DereferencingNodeIterator;
import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

/** Base class of the creator classes for DFAs and NFAs.
 */
public class CreatorBase
{
    CreatorBase (
        final SchemaBase aSchemaBase,
        final File aLogFile)
    {
        maSchemaBase = aSchemaBase;
        maStateContainer = new StateContainer();

        PrintStream aLog = null;
        try
        {
            aLog = new PrintStream(new FileOutputStream(aLogFile));
        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        }
        maLog = aLog;
        msLogIndentation = "";
    }




    protected Vector<Attribute> CollectAttributes (final INode aRoot)
    {
        final Vector<Attribute> aAttributes = new Vector<>();
        for (final INode aNode : new DereferencingNodeIterator(aRoot, maSchemaBase, true))
            for (final Attribute aAttribute : new AttributeIterator(aNode, maSchemaBase))
                aAttributes.add(aAttribute);
        return aAttributes;
    }




    protected void AddSkipTransition (
        final State aState,
        final SkipData aSkipData)
    {
        aState.AddSkipData(aSkipData);

        if (maLog != null)
        {
            maLog.printf("%sskip state %s\n",
                msLogIndentation,
                aState.GetFullname());
        }
    }




    protected void ProcessAttributes (final INode aNode)
    {
        for (final Attribute aAttribute : new AttributeIterator(aNode, maSchemaBase))
        {
            maLog.printf("%sattribute %s\n",
                msLogIndentation,
                aAttribute.GetName().GetDisplayName());
            maAttributes.add(aAttribute);
        }
    }




    protected void AddComment (
        final String sFormat,
        final Object ... aArgumentList)
    {
        if (maLog != null)
        {
            maLog.print(msLogIndentation);
            maLog.print("// ");
            maLog.printf(sFormat, aArgumentList);
            maLog.print("\n");
        }
    }




    protected void StartBlock ()
    {
        if (maLog != null)
            msLogIndentation += "    ";
    }




    protected void EndBlock ()
    {
        if (maLog != null)
            msLogIndentation = msLogIndentation.substring(4);
    }




    protected final SchemaBase maSchemaBase;
    protected final StateContainer maStateContainer;
    protected final PrintStream maLog;
    protected String msLogIndentation;
    protected Vector<Attribute> maAttributes;
}

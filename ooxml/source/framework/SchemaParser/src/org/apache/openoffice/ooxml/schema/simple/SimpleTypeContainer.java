package org.apache.openoffice.ooxml.schema.simple;

import java.io.File;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;

public class SimpleTypeContainer
{
    public static SimpleTypeContainer Create (
        final SchemaBase aSchemaBase,
        final File aLogFile)
    {
        final SimpleTypeContainer aContainer = new SimpleTypeContainer(aLogFile);
        for (final SimpleType aType : aSchemaBase.SimpleTypes.GetSorted())
        {
            aContainer.ProcessSimpleType(aType, aSchemaBase);
        }
        return aContainer;
    }




    public int GetSimpleTypeCount ()
    {
        return maSimpleTypes.size();
    }




    public Iterable<Entry<String,SimpleTypeDescriptor>> GetSimpleTypes ()
    {
        return maSimpleTypes.entrySet();
    }




    public Iterable<Entry<String,SimpleTypeDescriptor>> GetSimpleTypesSorted ()
    {
        final Map<String,SimpleTypeDescriptor> aSortedSimpleTypes = new TreeMap<>();
        aSortedSimpleTypes.putAll(maSimpleTypes);
        return aSortedSimpleTypes.entrySet();
    }




    private SimpleTypeContainer (final File aLogFile)
    {
        maSimpleTypes = new TreeMap<>();
        maLog = new Log(aLogFile);
    }




    private void ProcessSimpleType (
        final SimpleType aType,
        final SchemaBase aSchemaBase)
    {
        maSimpleTypes.put(
            aType.GetName().GetStateName(),
            SimpleTypeDescriptorFactory.CreateSimpleTypeDescriptor(
                aType,
                aSchemaBase,
                maLog));
    }




    private final Map<String,SimpleTypeDescriptor> maSimpleTypes;
    private final Log maLog;
}

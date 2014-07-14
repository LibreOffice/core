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

package org.apache.openoffice.ooxml.schema.generator;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.util.Map.Entry;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleTypeReference;

public class LogGenerator
{
    public static void Write (
            final File aOutputFile,
            final SchemaBase aSchemaBase,
            final Iterable<Schema> aTopLevelSchemas)
    {
        final long nStartTime = System.currentTimeMillis();

        try
        {
            final LogGenerator aGenerator = new LogGenerator(
                new PrintStream(aOutputFile),
                aSchemaBase);

            aGenerator.WriteNamespaces(aSchemaBase);
            aGenerator.WriteTopLevelElements(aTopLevelSchemas);
            aGenerator.WriteComplexTypes(aSchemaBase);
            aGenerator.WriteGroups(aSchemaBase);
            aGenerator.WriteSimpleTypes(aSchemaBase);
            aGenerator.WriteAttributeGroups(aSchemaBase);
            aGenerator.WriteAttributes(aSchemaBase);
        }
        catch (final FileNotFoundException aException)
        {
            aException.printStackTrace();
        }

        final long nEndTime = System.currentTimeMillis();
        System.out.printf("wrote log output to '%s' in %fs\n",
            aOutputFile.toString(),
            (nEndTime-nStartTime)/1000.0f);
    }




    private LogGenerator (
        final PrintStream aOut,
        final SchemaBase aSchemaBase)
    {
        maSchemaBase = aSchemaBase;
        maOut = aOut;
    }




    private void WriteComment (final String sFormat, final Object ... aArgumentList)
    {
        maOut.printf("// "+sFormat+"\n", aArgumentList);
    }




    private void WriteNamespaces (final SchemaBase aSchema)
    {
        // Write namespace definitions.
        WriteComment("%d Namespaces.", aSchema.Namespaces.GetCount());
        for (final Entry<String,String> aEntry : aSchema.Namespaces.GetSorted())
        {
            maOut.printf("    %s -> %s\n",
                aEntry.getValue()==null ? "<no-prefix>" : aEntry.getValue(),
                    aEntry.getKey());
        }
    }



    private void WriteTopLevelElements (final Iterable<Schema> aTopLevelSchemas)
    {
        // Write top level elements.
        WriteComment("Top-level elements.");
        for (final Schema aSchema : aTopLevelSchemas)
        {
            WriteComment(" Schema %s.", aSchema.GetShortName());
            for (final Element aElement : aSchema.TopLevelElements.GetSorted())
                maOut.printf("    \"%s\" -> %s\n",
                        aElement.GetElementName().GetDisplayName(),
                        aElement.GetTypeName().GetDisplayName());
        }
    }




    private void WriteComplexTypes (final SchemaBase aSchema)
    {
        WriteComment(" %d Complex Types.", aSchema.ComplexTypes.GetCount());
        for (final ComplexType aType : aSchema.ComplexTypes.GetSorted())
        {
            WriteType("    ", aType, true);
        }
    }




    private void WriteSimpleTypes (final SchemaBase aSchema)
    {
        WriteComment(" %d Simple Types.", aSchema.SimpleTypes.GetCount());
        for (final SimpleType aType : aSchema.SimpleTypes.GetSorted())
        {
            WriteType("    ", aType, true);
        }
    }




    private void WriteGroups (final SchemaBase aSchema)
    {
        WriteComment(" %d Groups.", aSchema.Groups.GetCount());
        for (final Node aType : aSchema.Groups.GetSorted())
        {
            WriteType("    ", aType, true);
        }
    }




    private void WriteAttributeGroups (final SchemaBase aSchema)
    {
        WriteComment(" %d Attribute Groups.", aSchema.AttributeGroups.GetCount());
        for (final Node aType : aSchema.AttributeGroups.GetSorted())
        {
            WriteType("    ", aType, true);
        }
    }




    private void WriteAttributes (final SchemaBase aSchema)
    {
        WriteComment(" %d Attributes.", aSchema.Attributes.GetCount());
        for (final Node aType : aSchema.Attributes.GetSorted())
        {
            WriteType("    ", aType, true);
        }
    }




    private void WriteType (
        final String sIndentation,
        final INode aType,
        final boolean bIsTopLevel)
    {
        maOut.printf("%s%s", sIndentation, aType.toString());

        if (bIsTopLevel)
        {
            final Node aNode = (Node)aType;
            maOut.printf(" defined at %s",
                aNode.GetLocation());
        }
        if ( ! HasChild(aType))
        {
            maOut.printf(" {}\n");
        }
        else
        {
            maOut.printf(" {\n");

            // Write attributes.
            switch(aType.GetNodeType())
            {
                case ComplexType:
                    for (final INode aAttribute : ((ComplexType)aType).GetAttributes())
                        WriteAttribute(sIndentation+"    ", aAttribute);
                    break;

                case SimpleTypeReference:
                    WriteType(
                            sIndentation+"    ",
                            ((SimpleTypeReference)aType).GetReferencedSimpleType(maSchemaBase),
                            false);
                    break;

                default:
                    break;
            }


            // Write child types.
            for (final INode aChild : aType.GetChildren())
                WriteType(sIndentation+"    ", aChild, false);

            maOut.printf("%s}\n", sIndentation);
        }
    }




    private void WriteAttribute (
        final String sIndentation,
        final INode aAttribute)
    {
        switch(aAttribute.GetNodeType())
        {
            case Attribute:
                maOut.printf(
                    "%sattribute %s of type %s\n",
                    sIndentation,
                    ((Attribute)aAttribute).GetName().GetDisplayName(),
                    ((Attribute)aAttribute).GetTypeName().GetDisplayName());
                break;

            case AttributeGroup:
                maOut.printf(
                    "%sattribute group %s {\n",
                    sIndentation,
                    ((AttributeGroup)aAttribute).GetName().GetDisplayName());
                for (final INode aChildAttribute : ((AttributeGroup)aAttribute).GetChildren())
                    WriteAttribute(sIndentation+"    ", aChildAttribute);
                maOut.printf("%s}\n", sIndentation);
                break;
            case AttributeGroupReference:
                maOut.printf(
                    "%sreference to attribute group %s {\n",
                    sIndentation,
                    ((AttributeGroupReference)aAttribute).GetReferencedName().GetDisplayName());
                WriteAttribute(sIndentation+"    ", ((AttributeGroupReference)aAttribute).GetReferencedAttributeGroup(maSchemaBase));
                maOut.printf("%s}\n", sIndentation);
                break;

            case AttributeReference:
                maOut.printf(
                    "%sreference to attribute %s {\n",
                    sIndentation,
                    ((AttributeReference)aAttribute).GetReferencedName().GetDisplayName());
                WriteAttribute(sIndentation+"    ", ((AttributeReference)aAttribute).GetReferencedAttribute(maSchemaBase));
                maOut.printf("%s}\n", sIndentation);
                break;
            default:
                throw new RuntimeException();
        }
    }




    private boolean HasChild (final INode aType)
    {
        if (aType.GetChildren().iterator().hasNext())
            return true;

        switch (aType.GetNodeType())
        {
            case ComplexType:
                return ((ComplexType)aType).GetAttributes().iterator().hasNext();

            case SimpleTypeReference:
                return true;

            default:
                return false;
        }
    }




    private final SchemaBase maSchemaBase;
    private final PrintStream maOut;
}

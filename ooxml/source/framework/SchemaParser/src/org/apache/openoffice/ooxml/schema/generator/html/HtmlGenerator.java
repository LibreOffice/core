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

package org.apache.openoffice.ooxml.schema.generator.html;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.complex.All;
import org.apache.openoffice.ooxml.schema.model.complex.Any;
import org.apache.openoffice.ooxml.schema.model.complex.Choice;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexContent;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexTypeReference;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.complex.ElementReference;
import org.apache.openoffice.ooxml.schema.model.complex.Extension;
import org.apache.openoffice.ooxml.schema.model.complex.Group;
import org.apache.openoffice.ooxml.schema.model.complex.GroupReference;
import org.apache.openoffice.ooxml.schema.model.complex.OccurrenceIndicator;
import org.apache.openoffice.ooxml.schema.model.complex.Sequence;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltIn;
import org.apache.openoffice.ooxml.schema.model.simple.List;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleContent;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleTypeReference;
import org.apache.openoffice.ooxml.schema.model.simple.Union;

/** Create a single HTML page that shows information about all
 *  complex and simple types.
 */
public class HtmlGenerator
    implements INodeVisitor
{
    public HtmlGenerator(
        final SchemaBase aSchemaBase,
        final Map<String, Schema> aTopLevelSchemas,
        final File aOutputFile)
    {
        maSchemaBase = aSchemaBase;
        maTopLevelSchemas = aTopLevelSchemas;
        msIndentation = "";
        msSingleIndentation = "  ";
        msSpace = " ";
        try
        {
            maOut = new PrintStream(new FileOutputStream(aOutputFile));
        } catch (FileNotFoundException aException)
        {
            aException.printStackTrace();
            throw new RuntimeException(aException);
        }
    }




    /** Read a template HTML file, expand its $... references and write the resulting content.
     */
    public void Generate ()
    {
        CopyFile("linking-template.html", true);
        maOut.close();
    }




    private void CopyFile (
        final String sBasename,
        final boolean bIsTemplate)
    {
        try
        {
            final BufferedReader aIn = new BufferedReader(
                new InputStreamReader(
                    new FileInputStream(
                        new File(
                            new File("bin/org/apache/openoffice/ooxml/schema/generator/html"),
                            sBasename))));

            final Pattern aReferencePattern = Pattern.compile("^(.*?)\\$([^\\$]+)\\$(.*)$");
            while (true)
            {
                final String sLine = aIn.readLine();
                if (sLine == null)
                    break;

                if (bIsTemplate)
                {
                    final Matcher aMatcher = aReferencePattern.matcher(sLine);
                    if (aMatcher.matches())
                    {
                        maOut.println(aMatcher.group(1));
                        switch(aMatcher.group(2))
                        {
                            case "CSS":
                                CopyFile("display.css", false);
                                break;
                            case "CODE":
                                CopyFile("code.js", false);
                                break;
                            case "DATA":
                                WriteJsonData();
                                break;
                        }
                        maOut.printf("%s\n", aMatcher.group(3));
                    }
                    else
                        maOut.printf("%s\n", sLine);
                }
                else
                    maOut.printf("%s\n", sLine);
            }
            aIn.close();
        }
        catch (final Exception e)
        {
            e.printStackTrace();
        }
    }




    private void WriteJsonData ()
    {
        maOut.printf("Data={\n");

        WriteTopLevelNodes(maSchemaBase.ComplexTypes.GetSorted());
        WriteTopLevelNodes(maSchemaBase.SimpleTypes.GetSorted());
        WriteTopLevelNodes(maSchemaBase.Groups.GetSorted());
        WriteTopLevelNodes(maSchemaBase.AttributeGroups.GetSorted());

        maOut.printf("}\n");
    }




    private void WriteTopLevelNodes (final Iterable<? extends INode> aNodes)
    {
        for (final INode aNode : aNodes)
        {
            maOut.printf("    \"%s\" : {\n", aNode.GetName().GetDisplayName());

            final String sSavedIndentation = msIndentation;
            msIndentation += msSingleIndentation + msSingleIndentation;
            aNode.AcceptVisitor(this);
            msIndentation = sSavedIndentation;

            maOut.printf("    }, \n");
        }
    }




    @Override
    public void Visit (final All aNode)
    {
        WritePair("type", "all");
        WriteLocation(aNode);
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final Any aNode)
    {
        WritePair("type", "any");
        WriteLocation(aNode);
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final ComplexContent aNode)
    {
        WritePair("type", "complex-content");
        WriteLocation(aNode);
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final ComplexType aNode)
    {
        WritePair("type", "complex-type");
        WritePair("name", aNode.GetName().GetDisplayName());
        WriteLocation(aNode);
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final ComplexTypeReference aReference)
    {
        WritePair("type", "complex-type-reference");
        WriteLocation(aReference);
        WritePair("referenced-complex-type", aReference.GetReferencedTypeName().GetDisplayName());
        WriteAttributes(aReference);
    }




    @Override
    public void Visit (final Choice aNode)
    {
        WritePair("type", "choice");
        WriteLocation(aNode);
        WriteChildren(aNode);
        WriteAttributes(aNode);
    }




    @Override
    public void Visit (final Element aNode)
    {
        WritePair("type", "element");
        WriteLocation(aNode);
        WritePair("tag", aNode.GetElementName().GetDisplayName());
        WritePair("result-type", aNode.GetTypeName().GetDisplayName());
        WriteAttributes(aNode);
    }




    @Override
    public void Visit (final ElementReference aReference)
    {
        WritePair("type", "element-type-reference");
        WriteLocation(aReference);
        WritePair("referenced-element", aReference.GetReferencedElementName().GetDisplayName());
        WriteAttributes(aReference);
    }




    @Override
    public void Visit (final Extension aNode)
    {
        WritePair("type", "extension");
        WriteLocation(aNode);
        WritePair("base-type", aNode.GetBaseTypeName().GetDisplayName());
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final Group aNode)
    {
        WritePair("type", "group");
        WriteLocation(aNode);
        WritePair("name", aNode.GetName().GetDisplayName());
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final GroupReference aReference)
    {
        WritePair("type", "group-reference");
        WriteLocation(aReference);
        WritePair("referenced-group", aReference.GetReferencedGroupName().GetDisplayName());
        WriteAttributes(aReference);
    }




    @Override
    public void Visit (final OccurrenceIndicator aNode)
    {
        WritePair("type", "occurrence");
        WriteLocation(aNode);
        WritePair("minimum", aNode.GetDisplayMinimum());
        WritePair("maximum", aNode.GetDisplayMaximum());
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final Sequence aNode)
    {
        WritePair("type", "sequence");
        WriteLocation(aNode);
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final BuiltIn aNode)
    {
        WritePair("type", "builtin");
        WriteLocation(aNode);
        WritePair("builtin-type", aNode.GetBuiltInType().GetQualifiedName().GetDisplayName());
        WriteAttributes(aNode);
    }




    @Override
    public void Visit (final List aNode)
    {
        WritePair("type", "list");
        WriteLocation(aNode);
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final Restriction aNode)
    {
        WritePair("type", "restriction");
        WriteLocation(aNode);
        WritePair("base-type", aNode.GetBaseType().GetDisplayName());
        WriteAttributes(aNode);

        if (aNode.HasFeature(Restriction.EnumerationBit))
            WritePair("enumeration", Join(aNode.GetEnumeration(), ";"));
        if (aNode.HasFeature(Restriction.PatternBit))
            WritePair("pattern", QuoteString(aNode.GetPattern()));
        if (aNode.HasFeature(Restriction.MinExclusiveBit))
            WritePair("exclusive-minimum", aNode.GetMinExclusive());
        if (aNode.HasFeature(Restriction.MinInclusiveBit))
            WritePair("inclusive-minimum", aNode.GetMinInclusive());
        if (aNode.HasFeature(Restriction.MaxInclusiveBit))
            WritePair("inclusive-maximum", aNode.GetMaxInclusive());
        if (aNode.HasFeature(Restriction.MaxInclusiveBit))
            WritePair("inclusive-maximum", aNode.GetMaxInclusive());
        if (aNode.HasFeature(Restriction.LengthBit))
            WritePair("length", Integer.toString(aNode.GetLength()));
        if (aNode.HasFeature(Restriction.MinLengthBit))
            WritePair("minimum-length", Integer.toString(aNode.GetMinimumLength()));
        if (aNode.HasFeature(Restriction.MaxLengthBit))
            WritePair("maximum-length", Integer.toString(aNode.GetMaximumLength()));
        assert(aNode.GetChildCount() == 0);
    }




    @Override
    public void Visit (final SimpleContent aNode)
    {
        WritePair("type", "simple-content");
        WriteLocation(aNode);
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final SimpleType aNode)
    {
        WritePair("type", "simple-type");
        WriteLocation(aNode);
        WritePair("name", aNode.GetName().GetDisplayName());
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final SimpleTypeReference aReference)
    {
        WritePair("type", "simple-type-reference");
        WriteLocation(aReference);
        WritePair("referenced-simple-type", aReference.GetReferencedTypeName().GetDisplayName());
        WriteAttributes(aReference);
    }




    @Override
    public void Visit (final Union aNode)
    {
        WritePair("type", "union");
        WriteLocation(aNode);
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final AttributeGroup aNode)
    {
        WritePair("type", "attribute-group");
        WriteLocation(aNode);
        WritePair("name", aNode.GetName().GetDisplayName());
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final AttributeReference aReference)
    {
        WritePair("type", "attribute-reference");
        WriteLocation(aReference);
        WritePair("referenced-attribute", aReference.GetReferencedName().GetDisplayName());
        WriteAttributes(aReference);
    }




    @Override
    public void Visit (final Attribute aNode)
    {
        WritePair("type", "attribute");
        WriteLocation(aNode);
        WritePair("name", aNode.GetName().GetDisplayName());
        WritePair("default-value", aNode.GetDefault());
        WritePair("value-type", aNode.GetTypeName().GetDisplayName());
        WritePair("use", aNode.GetUse().toString());
        WriteAttributes(aNode);
        WriteChildren(aNode);
    }




    @Override
    public void Visit (final AttributeGroupReference aReference)
    {
        WritePair("type", "attribute-group-reference");
        WriteLocation(aReference);
        WritePair("referenced-attribute-group", aReference.GetReferencedName().GetDisplayName());
        WriteAttributes(aReference);
    }




    private void WriteChildren (
        final INode aParent)
    {
        if (aParent.GetChildCount() > 0)
        {
            maOut.printf("%s\"children\" : [\n", msIndentation);
            int nIndex = 0;
            for (final INode aChild : aParent.GetChildren())
            {
                maOut.printf("%s%s{\n", msIndentation, msSingleIndentation, nIndex++);

                final String sSavedIndentation = msIndentation;
                msIndentation += msSingleIndentation + msSingleIndentation;
                aChild.AcceptVisitor(this);
                msIndentation = sSavedIndentation;

                maOut.printf("%s%s},\n", msIndentation, msSingleIndentation);
            }

            maOut.printf("%s]\n", msIndentation);
        }
    }




    private void WriteAttributes (
        final INode aParent)
    {
        if (aParent.GetAttributeCount() > 0)
        {
            maOut.printf("%s\"attributes\" : [\n", msIndentation);
            int nIndex = 0;
            for (final INode aAttribute: aParent.GetAttributes())
            {

                maOut.printf("%s%s{\n", msIndentation, msSingleIndentation, nIndex++);

                final String sSavedIndentation = msIndentation;
                msIndentation += msSingleIndentation + msSingleIndentation;
                aAttribute.AcceptVisitor(this);
                msIndentation = sSavedIndentation;

                maOut.printf("%s%s},\n", msIndentation, msSingleIndentation);
            }

            maOut.printf("%s],\n", msIndentation);
        }
    }




    private void WriteLocation (
        final INode aNode)
    {
        if (aNode.GetLocation() == null)
            return;

        WritePair("location", aNode.GetLocation().toString());
    }




    private void WritePair (
        final String sKey,
        final String sValue)
    {
        maOut.printf("%s\"%s\"%s:%s\"%s\"%s\n", msIndentation, sKey, msSpace, msSpace, sValue, ",");
    }




    private String Join (final Collection<String> aValues, final String sSeparator)
    {
        final Iterator<String> aIterator = aValues.iterator();
        if ( ! aIterator.hasNext())
            return "";

        final StringBuffer aBuffer = new StringBuffer(aIterator.next());
        while (aIterator.hasNext())
        {
            aBuffer.append(sSeparator);
            aBuffer.append(aIterator.next());
        }
        return aBuffer.toString();
    }




    private String QuoteString (final String sValue)
    {
        return sValue.replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
    }




    private final SchemaBase maSchemaBase;
    private final Map<String, Schema> maTopLevelSchemas;
    private final PrintStream maOut;
    private String msIndentation;
    private final String msSingleIndentation;
    private final String msSpace;
}

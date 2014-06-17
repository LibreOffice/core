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

package org.apache.openoffice.ooxml.schema.parser;

import java.io.File;
import java.io.FileInputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.complex.All;
import org.apache.openoffice.ooxml.schema.model.complex.Any;
import org.apache.openoffice.ooxml.schema.model.complex.Choice;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexContent;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.complex.ElementReference;
import org.apache.openoffice.ooxml.schema.model.complex.Extension;
import org.apache.openoffice.ooxml.schema.model.complex.Group;
import org.apache.openoffice.ooxml.schema.model.complex.GroupReference;
import org.apache.openoffice.ooxml.schema.model.complex.OccurrenceIndicator;
import org.apache.openoffice.ooxml.schema.model.complex.Sequence;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.List;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleContent;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleTypeReference;
import org.apache.openoffice.ooxml.schema.model.simple.Union;


/** Parser for single schema file.
 *  Imports and includes of other schema files are stored and can be retrieved
 *  by calling GetImportedSchemas().
 *
 *  Typical usage:
 *  1) Create SchemaParser for top-level schema file.
 *  2) Call Parse().
 *  3) Repeat the same recursively for all imported schemas (as returned by
 *     GetImportedSchemas()).
 *
 *  All top level types (complex types, simple types, elements, etc.) are
 *  stored in the given Schema object.
 */
public class SchemaParser
{
    public SchemaParser (
        final File aSchemaFile,
        final Schema aSchema,
        final SchemaBase aSchemaBase)
    {
        maSchema = aSchema;
        maSchemaBase = aSchemaBase;
        maReader = GetStreamReader(aSchemaFile);
        msBasename = aSchemaFile.getName();
        maDirectory = aSchemaFile.getParentFile();
        maLocalNamespaceMap = new HashMap<>();
        maImportedSchemas = new Vector<>();
        maLastLocation = null;
    }




    /** Parse the schema file.
     *  @return
     *      Return false if there is any error.
     * @throws XMLStreamException
     */
    public void Parse ()
        throws XMLStreamException
    {
        if (maReader == null)
            return;

        while (maReader.hasNext())
        {
            final int nCode = maReader.next();
            switch (nCode)
            {
                case XMLStreamReader.START_ELEMENT:
                    if (maReader.getLocalName().equals("schema"))
                    {
                        ProcessSchemaTag();
                        ParseSchema();

                        maLastLocation = maReader.getLocation();
                    }
                    else
                    {
                        throw CreateErrorException("expecting top level element to be 'schema'");
                    }
                    break;

                case XMLStreamReader.END_DOCUMENT:
                    return;

                default:
                    throw CreateErrorException("unexpected XML event %d", nCode);
            }
        }
    }




    public Iterable<File> GetImportedSchemaFilenames ()
    {
        return maImportedSchemas;
    }




    public int GetLineCount ()
    {
        if (maLastLocation != null)
            return maLastLocation.getLineNumber();
        else
            return 0;
    }




    public int GetByteCount ()
    {
        if (maLastLocation != null)
            return maLastLocation.getCharacterOffset();
        else
            return 0;
    }




    /** Process the namespace definitions in the outer 'schema' element.
     */
    private void ProcessSchemaTag ()
    {
        GetOptionalAttributeValue("id", null);
        meAttributeFormDefault = FormDefault.valueOf(
            GetOptionalAttributeValue("attributeFormDefault", "unqualified"));
        meElementFormDefault = FormDefault.valueOf(
            GetOptionalAttributeValue("elementFormDefault", "unqualified"));
        GetOptionalAttributeValue("blockDefault", null);//=(#all|list of (extension|restriction|substitution))
        GetOptionalAttributeValue("finalDefault", null);//=(#all|list of (extension|restriction|list|union))
        msTargetNamespace = GetOptionalAttributeValue("targetNamespace", null);
        GetOptionalAttributeValue("version", null);

        for (int nIndex=0; nIndex<maReader.getNamespaceCount(); ++nIndex)
        {
            final String sPrefix = maReader.getNamespacePrefix(nIndex);
            final String sURI = maReader.getNamespaceURI(nIndex);
            maLocalNamespaceMap.put(sPrefix, sURI);
            maSchemaBase.Namespaces.ProvideNamespace(sURI, sPrefix);
        }

        maLocalNamespaceMap.put(null, msTargetNamespace);
        maSchemaBase.Namespaces.ProvideNamespace(msTargetNamespace, null);
    }




    private void ParseSchema ()
        throws XMLStreamException
    {
        while (true)
        {
            switch (Next())
            {
                case XMLStreamReader.START_ELEMENT:
                    ProcessTopLevelStartElement();
                    break;

                case XMLStreamReader.END_ELEMENT:
                    return;

                default:
                    throw CreateErrorException("unexpected event (expteced START_ELEMENT): %d", maReader.getEventType());
            }
        }
    }




    private void ProcessTopLevelStartElement ()
        throws XMLStreamException
    {
        assert(GetAttributeValue("minOccurs") == null);
        assert(GetAttributeValue("maxOccurs") == null);

        switch (maReader.getLocalName())
        {
            case "attribute":
                maSchemaBase.Attributes.Add(ParseAttribute());
                break;

            case "attributeGroup":
                maSchemaBase.AttributeGroups.Add(ParseAttributeGroup());
                break;

            case "complexType":
                maSchemaBase.ComplexTypes.Add(ParseComplexType());
                break;

            case "element":
                final Element aElement = ParseElement(null);
                if (maSchema != null)
                    maSchema.TopLevelElements.Add(aElement);
                maSchemaBase.TopLevelElements.Add(aElement);
                break;

            case "group":
                maSchemaBase.Groups.Add(ParseGroup(null));
                break;

            case "import":
                ParseImport();
                break;

            case "include":
                ParseInclude();
                break;

            case "simpleType":
                maSchemaBase.SimpleTypes.Add(ParseSimpleType(null));
                break;

            default:
                throw CreateErrorException("unexpected top level element %s", maReader.getLocalName());
        }
    }




    private void ProcessStartElement (final Node aParent)
        throws XMLStreamException
    {
        final String sMinimumOccurrence = GetOptionalAttributeValue("minOccurs", "1");
        final String sMaximumOccurrence = GetOptionalAttributeValue("maxOccurs", "1");

        final Node aLocalParent;
        if ( ! (sMinimumOccurrence.equals("1") && sMaximumOccurrence.equals("1")))
        {
            // Occurrence does not only have default values (min=max=1).
            // Have to create an intermediate node for the occurrence indicator.
            final OccurrenceIndicator aIndicator = new OccurrenceIndicator(
                aParent,
                sMinimumOccurrence,
                sMaximumOccurrence,
                GetLocation());
            aParent.AddChild(aIndicator);
            aLocalParent = aIndicator;
        }
        else
            aLocalParent = aParent;

        switch (maReader.getLocalName())
        {
            case "all":
                aLocalParent.AddChild(ParseAll(aLocalParent));
                break;

            case "any":
                aLocalParent.AddChild(ParseAny(aLocalParent));
                break;

            case "attribute":
                aLocalParent.AddAttribute(ParseAttributeOrReference());
                break;

            case "attributeGroup":
                aLocalParent.AddAttribute(ParseAttributeGroupOrReference());
                break;

            case "choice":
                aLocalParent.AddChild(ParseChoice(aLocalParent));
                break;

            case "complexContent":
                aLocalParent.AddChild(ParseComplexContent(aLocalParent));
                break;

            case "element":
                aLocalParent.AddChild(ParseElementOrReference(aLocalParent));
                break;

            case "extension":
                aLocalParent.AddChild(ParseExtension(aLocalParent));
                break;

            case "group":
                aLocalParent.AddChild(ParseGroupOrReference(aLocalParent));
                break;

            case "list":
                aLocalParent.AddChild(ParseList(aLocalParent));
                break;

            case "sequence":
                aLocalParent.AddChild(ParseSequence(aLocalParent));
                break;

            case "simpleContent":
                aLocalParent.AddChild(ParseSimpleContent(aLocalParent));
                break;

            default:
                throw CreateErrorException("unsupported content type %s", maReader.getLocalName());
        }
    }




    private void IgnoreAnnotation ()
        throws XMLStreamException
    {
        IgnoreContent();
    }




    private void IgnoreContent ()
        throws XMLStreamException
    {
        while(true)
        {
            switch(maReader.next())
            {
                case XMLStreamReader.START_ELEMENT:
                    IgnoreContent();
                    break;

                case XMLStreamReader.END_ELEMENT:
                    return;

                case XMLStreamReader.COMMENT:
                case XMLStreamReader.CHARACTERS:
                    break;

                default:
                    throw CreateErrorException(
                        "unexpected XML event %d while ignoring content",
                        maReader.getEventType());
            }
        }
    }




    private All ParseAll (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs"));

        final All aAll = new All(aParent, GetLocation());
        ParseContent(aAll);
        return aAll;
    }




    private Any ParseAny (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs", "namespace", "processContents"));

        final Any aAny = new Any(
            aParent,
            GetLocation(),
            GetOptionalAttributeValue("processContents", "strict"),
            GetOptionalAttributeValue("namespace", "##any"));
        ExpectEndElement("ParseAny");
        return aAny;
    }




    private AttributeGroup ParseAttributeGroup ()
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("name"));

        final AttributeGroup aGroup = new AttributeGroup(GetOptionalQualifiedName("name"), GetLocation());

        while (true)
        {
            switch (Next())
            {
                case XMLStreamReader.START_ELEMENT:
                    if ( ! maReader.getLocalName().equals("attribute"))
                        throw CreateErrorException(
                            "attributeGroup expects element 'attribute' but got %s",
                            maReader.getLocalName());
                    else
                        aGroup.AddAttribute(ParseAttributeOrReference());
                    break;

                case XMLStreamReader.END_ELEMENT:
                    return aGroup;

                default:
                    throw CreateErrorException(
                        "unexpected event when parsing attributeGroup: %d",
                        maReader.getEventType());
            }
        }
    }




    private INode ParseAttributeGroupOrReference ()
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("ref"));

        final INode aGroup;
        if (GetAttributeValue("schemaLocation") != null)
        {
            aGroup = ParseAttributeGroup();
        }
        else
        {
            aGroup = new AttributeGroupReference(
                CreateQualifiedName(
                    GetAttributeValue("ref")),
                GetLocation());
            ExpectEndElement("attribute group or reference");
        }
        return aGroup;
    }




    private INode ParseAttributeOrReference ()
        throws XMLStreamException
    {
        final INode aAttribute;
        if (GetAttributeValue("name") != null)
        {
            aAttribute = ParseAttribute();
        }
        else
        {
            assert(HasOnlyAttributes("default", "fixed", "ref", "use"));

            aAttribute = new AttributeReference(
                GetQualifiedName("ref"),
                GetOptionalAttributeValue("use", "optional"),
                GetOptionalAttributeValue("default", null),
                GetOptionalAttributeValue("fixed", null),
                meAttributeFormDefault,
                GetLocation());
            ExpectEndElement("attribute reference");
        }
        return aAttribute;
    }




    private Attribute ParseAttribute ()
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("default", "fixed", "name", "type", "use"));

        final Attribute aAttribute = new Attribute(
            GetQualifiedName("name"),
            GetQualifiedName("type"),
            GetOptionalAttributeValue("use", "optional"),
            GetOptionalAttributeValue("default", null),
            GetOptionalAttributeValue("fixed", null),
            meAttributeFormDefault,
            GetLocation());
        ExpectEndElement("attribute");

        return aAttribute;
    }



    private Choice ParseChoice (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs"));

        final Choice aChoice = new Choice(aParent, GetLocation());
        ParseContent(aChoice);
        return aChoice;
    }




    private ComplexContent ParseComplexContent (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs"));

        final ComplexContent aNode = new ComplexContent(aParent, GetLocation());
        ParseContent(aNode);
        return aNode;
    }




    private ComplexType ParseComplexType ()
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("mixed", "name"));

        final ComplexType aComplexType = new ComplexType(
            null,
            GetQualifiedName("name"),
            GetLocation());

        ParseContent(aComplexType);

        return aComplexType;
    }




    private Element ParseElement (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs", "name", "type"));

        final Element aElement = new Element(
            aParent,
            GetQualifiedName("name"),
            GetQualifiedName("type"),
            GetLocation());

        ExpectEndElement("element");

        return aElement;
    }




    private Element ParseElementOrReference (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs", "name", "ref", "type"));

        final Element aElement;
        final String sName = GetOptionalAttributeValue("name", null);
        if (sName != null)
        {
            aElement = ParseElement(aParent);
        }
        else
        {
            final String sElementReference = GetOptionalAttributeValue("ref",  null);
            if (sElementReference != null)
            {
                ExpectEndElement("element reference");
                aElement = new ElementReference(
                    aParent,
                    CreateQualifiedName(sElementReference),
                    GetLocation());
            }
            else
            {
                throw CreateErrorException("element has no name and no ref");
            }
        }
        return aElement;
    }




    private Extension ParseExtension (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("base", "minOccurs", "maxOccurs"));

        final Extension aNode = new Extension(
            aParent,
            CreateQualifiedName(GetAttributeValue("base")),
            GetLocation());
        ParseContent(aNode);
        return aNode;
    }




    private Group ParseGroup (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("name"));

        final Group aGroup = new Group(
            aParent,
            GetQualifiedName("name"),
            GetLocation());
        ParseContent(aGroup);
        return aGroup;
    }




    private Node ParseGroupOrReference (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs", "name", "ref"));

        final Node aGroup;
        final String sName = GetOptionalAttributeValue("name", null);
        if (sName != null)
        {
            aGroup = ParseGroup(aParent);
        }
        else
        {
            final String sGroupReference = GetOptionalAttributeValue("ref",  null);
            if (sGroupReference != null)
            {
                ExpectEndElement("group reference");
                aGroup = new GroupReference(
                    aParent,
                    CreateQualifiedName(sGroupReference),
                    GetLocation());
            }
            else
            {
                throw CreateErrorException("group has no name and no ref");
            }
        }
        return aGroup;
    }




    private Restriction ParseRestriction (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("base", "value"));

        final String sBaseType = GetAttributeValue("base");
        final Restriction aRestriction = new Restriction(
            aParent,
            CreateQualifiedName(sBaseType),
            GetLocation());

        while (true)
        {
            switch (Next())
            {
                case XMLStreamReader.START_ELEMENT:
                    final String sValue = GetAttributeValue("value");
                    switch (maReader.getLocalName())
                    {
                        case "enumeration":
                            aRestriction.AddEnumeration(sValue);
                            break;

                        case "minInclusive":
                            aRestriction.SetMinInclusive(sValue);
                            break;

                        case "minExclusive":
                            aRestriction.SetMinExclusive(sValue);
                            break;

                        case "maxInclusive":
                            aRestriction.SetMaxInclusive(sValue);
                            break;

                        case "maxExclusive":
                            aRestriction.SetMaxExclusive(sValue);
                            break;

                        case "length":
                            aRestriction.SetLength(sValue);
                            break;

                        case "minLength":
                            aRestriction.SetMinLength(sValue);
                            break;

                        case "maxLength":
                            aRestriction.SetMaxLength(sValue);
                            break;

                        case "pattern":
                            aRestriction.SetPattern(sValue);
                            break;

                        default:
                            throw CreateErrorException("unsupported restriction type "+maReader.getLocalName());
                    }
                    ExpectEndElement("restriction");
                    break;

                case XMLStreamReader.END_ELEMENT:
                    return aRestriction;

                default:
                    throw CreateErrorException("unexpected XML event while parsing restrictions: %d", maReader.getEventType());
            }
        }
    }




    private List ParseList (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("itemType"));

        final List aList = new List(
            aParent,
            GetQualifiedName("itemType"),
            GetLocation());
        ExpectEndElement("list");
        return aList;
    }




    private Sequence ParseSequence (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs", "name"));

        final Sequence aSequence = new Sequence(
            aParent,
            GetOptionalQualifiedName("name"),
            GetLocation());
        ParseContent(aSequence);
        return aSequence;
    }




    private SimpleContent ParseSimpleContent (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("minOccurs", "maxOccurs"));

        final SimpleContent aSimpleContent = new SimpleContent(
            aParent,
            GetLocation());
        ParseContent(aSimpleContent);
        return aSimpleContent;
    }




    private SimpleType ParseSimpleType (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("final", "name"));

        final SimpleType aType = new SimpleType(
            aParent,
            GetQualifiedName("name"),
            GetLocation());
        final String sFinalValue = GetOptionalAttributeValue("final", null);
        if (sFinalValue != null)
            aType.SetFinal(sFinalValue.split("\\s+"));

        while (true)
        {
            switch (Next())
            {
                case XMLStreamReader.START_ELEMENT:
                    switch(maReader.getLocalName())
                    {
                        case "list":
                            aType.AddChild(ParseList(aType));
                            break;

                        case "restriction":
                            aType.AddChild(ParseRestriction(aType));
                            break;

                        case "union":
                            aType.AddChild(ParseUnion(aType));
                            break;

                        default:
                            throw CreateErrorException("unsupported simple type part %s", maReader.getLocalName());
                    }
                    break;

                case XMLStreamReader.END_ELEMENT:
                    return aType;

                default:
                    throw CreateErrorException("unexpected XML event in ParseSimpleType: %s", maReader.getEventType());
            }
        }
    }




    private Union ParseUnion (final Node aParent)
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("memberTypes"));

        final Union aUnion = new Union(
            aParent,
            GetLocation());
        final String[] aMemberTypes = GetAttributeValue("memberTypes").split("\\s+");
        for (int nIndex=0; nIndex<aMemberTypes.length; ++nIndex)
            aUnion.AddChild(
                new SimpleTypeReference(
                    aUnion,
                    CreateQualifiedName(aMemberTypes[nIndex]),
                    GetLocation()));

        ParseContent(aUnion);

        return aUnion;
    }


    private void ParseContent (final Node aParent)
        throws XMLStreamException
    {
        while(true)
        {
            switch(Next())
            {
                case XMLStreamReader.START_ELEMENT:
                    ProcessStartElement(aParent);
                    break;

                case XMLStreamReader.END_ELEMENT:
                    return;

                default:
                    throw CreateErrorException(
                        "unexpected XML event %d while parsing content of %s",
                        maReader.getEventType(),
                        aParent.toString());
            }
        }
    }




    private void ParseImport ()
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("id", "namespace", "schemaLocation"));

        final String sFilename = GetOptionalAttributeValue("schemaLocation", null);
        if (sFilename == null)
        {
            final String sNamespaceName = GetAttributeValue("namespace");
            if (sNamespaceName.equals(XmlNamespace.URI))
            {
                XmlNamespace.Apply(maSchemaBase);
                maLocalNamespaceMap.put(XmlNamespace.Prefix, XmlNamespace.URI);
            }
            else
                throw CreateErrorException("invalid import");
        }
        else
        {
            maImportedSchemas.add(new File(maDirectory, sFilename));
        }

        ExpectEndElement("import");
    }




    private void ParseInclude ()
        throws XMLStreamException
    {
        assert(HasOnlyAttributes("id", "schemaLocation"));

        final String sFilename = GetOptionalAttributeValue("schemaLocation", null);
        if (sFilename == null)
        {
            throw CreateErrorException("invalid include");
        }
        else
        {
            maImportedSchemas.add(new File(maDirectory, sFilename));
        }

        ExpectEndElement("include");
    }




    private void ExpectEndElement (final String sCaller)
        throws XMLStreamException
    {
        final int nNextEvent = Next();
        if (nNextEvent != XMLStreamReader.END_ELEMENT)
            throw CreateErrorException("expected END_ELEMENT of %s but got %s",
                sCaller,
                nNextEvent);
    }




    /** Return the next relevant token from the XML stream.
     *  Ignores comments.
     *  @return
     *      Returns the event code of the next relevant XML event or END_DOCUMENT when the end of the file has been reached.
     */
    private int Next ()
        throws XMLStreamException
    {
        while (maReader.hasNext())
        {
            switch (maReader.next())
            {
                case XMLStreamReader.COMMENT:
                    // Ignore comments.
                    break;

                case XMLStreamReader.CHARACTERS:
                    // Ignore whitespace.
                    if (maReader.getText().matches("^\\s+$"))
                        break;
                    else
                    {
                        // Character events are not expected in schema files
                        // and therefore not supported.
                        // Alternatively, they could easily be ignored.
                        throw CreateErrorException("unexpected CHARACTERS event with text [%s]", maReader.getText());
                    }

                case XMLStreamReader.START_ELEMENT:
                    switch (maReader.getLocalName())
                    {
                        case "annotation":
                            IgnoreAnnotation();
                            break;
                        case "unique":
                            // Not supported.
                            IgnoreContent();
                            break;
                        default:
                            return XMLStreamReader.START_ELEMENT;
                    }
                    break;

                default:
                    return maReader.getEventType();
            }
        }
        return XMLStreamReader.END_DOCUMENT;
    }




    private String GetAttributeValue (final String sAttributeLocalName)
    {
        return maReader.getAttributeValue(null, sAttributeLocalName);
    }




    private String GetOptionalAttributeValue (
        final String sAttributeLocalName,
        final String sDefaultValue)
    {
        final String sValue = maReader.getAttributeValue(null, sAttributeLocalName);
        if (sValue == null)
            return sDefaultValue;
        else
            return sValue;
    }




    /** Read the specified attribute and return its value as QualifiedName object.
     */
    private QualifiedName GetQualifiedName (final String sAttributeLocalName)
    {
        final String sName = maReader.getAttributeValue(null, sAttributeLocalName);
        if (sName == null)
            throw CreateErrorException(
                "did not find a qualified name as value of attribute '%s' at %s:%s",
                sAttributeLocalName,
                msBasename,
                maReader.getLocation());
        else
            return CreateQualifiedName(sName);
    }




    private QualifiedName GetOptionalQualifiedName (final String sAttributeLocalName)
    {
        final String sName = maReader.getAttributeValue(null, sAttributeLocalName);
        if (sName == null)
            return null;
        else
            return CreateQualifiedName(sName);
    }




    /** Create a QualifiedName object from the given string.
     * @param sName
     *     May or may not contain a namespace prefix (separated from the name
     *     by a colon).
     */
    private QualifiedName CreateQualifiedName (final String sName)
    {
        final String[] aParts = sName.split(":");
        final String sNamespaceURL;
        final String sLocalPart;
        switch (aParts.length)
        {
            case 1:
                // sName only consists of a local part.
                // Use the target namespace as namespace.
                sNamespaceURL = msTargetNamespace;
                sLocalPart = aParts[0];
                break;

            case 2:
                // sName is of the form prefix:local.
                sNamespaceURL = maLocalNamespaceMap.get(aParts[0]);
                sLocalPart = aParts[1];
                break;

            default:
                throw new RuntimeException(
                    "the string '"+sName+"' can not be transformed into a qualified name");
        }
        if (sNamespaceURL == null)
            throw CreateErrorException("can not determine namespace for '%s'", sName);
        // Transform the local namespace prefix into a global namespace prefix
        // (different schema files can use different prefixes for the same
        // namespace URI).
        final String sGlobalNamespacePrefix = maSchemaBase.Namespaces.GetNamespacePrefix(sNamespaceURL);
        return new QualifiedName(
            sNamespaceURL,
            sGlobalNamespacePrefix,
            sLocalPart);
    }




    /** Create an XMLStreamReader for the given file.
     *  Returns null when there is an error.
     */
    private static XMLStreamReader GetStreamReader (final File aSchemaFile)
    {
        final XMLInputFactory aFactory = (XMLInputFactory)XMLInputFactory.newInstance();
        aFactory.setProperty(XMLInputFactory.IS_REPLACING_ENTITY_REFERENCES, false);
        aFactory.setProperty(XMLInputFactory.IS_SUPPORTING_EXTERNAL_ENTITIES, false);
        aFactory.setProperty(XMLInputFactory.IS_COALESCING, false);

        try
        {
            return aFactory.createXMLStreamReader(
                aSchemaFile.getName(),
                new FileInputStream(aSchemaFile));
        }
        catch (Exception aException)
        {
            aException.printStackTrace();
            return null;
        }
    }




    private RuntimeException CreateErrorException (
        final String sFormat,
        final Object ... aArgumentList)
    {
        return new RuntimeException(String.format(sFormat, aArgumentList)
            + String.format(" in %s at L%dC%dP%d",
                msBasename,
                maReader.getLocation().getLineNumber(),
                maReader.getLocation().getColumnNumber(),
                maReader.getLocation().getCharacterOffset()));
    }




    /** This predicate is only used for debugging to assert
     *  that no unsupported attributes are present.
     *  If there where then the parser has to be extended.
     */
    private boolean HasOnlyAttributes (final String ... aAttributeNameList)
    {
        for (int nIndex=0,nCount=maReader.getAttributeCount(); nIndex<nCount; ++nIndex)
        {
            final String sAttributeName = maReader.getAttributeLocalName(nIndex);
            boolean bFound = false;
            for (final String sName : aAttributeNameList)
                if (sAttributeName.equals(sName))
                {
                    bFound = true;
                    break;
                }
            if ( ! bFound)
            {
                // Attribute name was not found in the given list.
                System.err.printf("attribute '%s' is not handled\n", sAttributeName);
                return false;
            }
        }
        return true;
    }




    private Location GetLocation ()
    {
        final javax.xml.stream.Location aLocation = maReader.getLocation();
        return new Location(
            msBasename,
            aLocation.getLineNumber(),
            aLocation.getColumnNumber(),
            aLocation.getCharacterOffset());
    }




    private final Schema maSchema;
    private final SchemaBase maSchemaBase;
    private final XMLStreamReader maReader;
    private final String msBasename;
    private final File maDirectory;
    private String msTargetNamespace;
    /// Map of namespace prefix to URI, local to the currently read schema file.
    private final Map<String,String> maLocalNamespaceMap;
    /// The names of other schema files referenced by import elements.
    private final Vector<File> maImportedSchemas;
    /// Some values for tracking the number of read bytes and lines.
    private javax.xml.stream.Location maLastLocation;
    private FormDefault meAttributeFormDefault;
    private FormDefault meElementFormDefault;
}

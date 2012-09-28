/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package complex.writer;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.NoSupportException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.Pair;
import com.sun.star.beans.StringPair;
import com.sun.star.container.XNamed;
import com.sun.star.container.XChild;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XContentEnumerationAccess;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.frame.XStorable;
import com.sun.star.util.XCloseable;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XWordCursor;
import com.sun.star.text.XSentenceCursor;
import com.sun.star.text.XParagraphCursor;
import com.sun.star.text.XFootnote;
import com.sun.star.text.XTextField;
import com.sun.star.text.XBookmarksSupplier;
import com.sun.star.text.XTextSectionsSupplier;
import com.sun.star.text.XDocumentIndexesSupplier;
import com.sun.star.text.TextContentAnchorType;
import static com.sun.star.text.TextContentAnchorType.*;
import static com.sun.star.text.ControlCharacter.*;
import com.sun.star.rdf.XMetadatable;
import com.sun.star.rdf.Statement;
import com.sun.star.rdf.XDocumentRepository;
import com.sun.star.rdf.XRepositorySupplier;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Stack;


class TreeNodeEnum
{
    private Object[] m_Elements;
    private int m_Pos;
    TreeNodeEnum(Object[] Elements) { m_Elements = Elements; m_Pos = 0; }
    boolean hasNext() { return m_Pos < m_Elements.length; }
    TreeNode next() { return (TreeNode) m_Elements[m_Pos++]; }
}

/** base class for tree nodes. only instance: root of tree. */
class TreeNode
{
    private List<TreeNode> m_Children;
    String getType() { return "__ROOT__"; }
    boolean hasContent() { return false; }
    boolean isNesting() { return false; }
    TreeNode dup() { throw new RuntimeException("TreeNode.dup"); }
    TreeNodeEnum createEnumeration() {
        return new TreeNodeEnum(m_Children.toArray());
    }
    TreeNode() { m_Children = new ArrayList<TreeNode>(); }
    TreeNode appendChild(TreeNode child)
    { m_Children.add(child); return this; }
    public String toString() {
        return "<" + getType() + ">";
    }
}

abstract class MarkNode extends TreeNode
{
    boolean m_isPoint;
    boolean m_isStart = false;
    String m_Name;
    boolean isPoint() { return m_isPoint; }
    boolean isStart() { return m_isStart; }
    String getName() { return m_Name; }
    MarkNode(String name) { m_Name = name; m_isPoint = true; }
    boolean equals(MarkNode other) {
        return (other.m_Name.equals(m_Name)) && (other.m_isPoint == m_isPoint)
            && (other.m_isStart == m_isStart);
    }
    public String toString() {
        return super.toString() + "\tname: " + m_Name +
            "\tisPoint: " + m_isPoint + "\tisStart: " + m_isStart;
    }
}

class BookmarkNode extends MarkNode
{
    private StringPair m_XmlId;
    StringPair getXmlId() { return m_XmlId; }
    BookmarkNode dup() { return new BookmarkNode(getName(), getXmlId()); }
    BookmarkNode(String name) { this(name, new StringPair());  }
    BookmarkNode(String name, StringPair xmlid) {
        super(name); m_XmlId = xmlid;
    }
    String getType() { return "Bookmark"; }
    public boolean equals(Object other) {
        return (other instanceof BookmarkNode)
            && super.equals((MarkNode) other)
            && MetaNode.eq(((BookmarkNode) other).m_XmlId, m_XmlId);
    }
    public String toString() {
        return super.toString()
            + "\txmlid: " + m_XmlId.First + "#" + m_XmlId.Second;
    }
}

class BookmarkStartNode extends BookmarkNode
{
    BookmarkStartNode dup() { return new BookmarkStartNode(getName()); }
    BookmarkStartNode(String name) { this(name, new StringPair()); }
    BookmarkStartNode(String name, StringPair xmlid) {
        super(name, xmlid); m_isPoint = false; m_isStart = true;
    }
}

class BookmarkEndNode extends BookmarkNode
{
    BookmarkEndNode dup() { return new BookmarkEndNode(getName()); }
    BookmarkEndNode(String name) { this(name, new StringPair()); }
    BookmarkEndNode(String name, StringPair xmlid) {
        super(name, xmlid); m_isPoint = false; m_isStart = false;
    }
}

class ReferenceMarkNode extends MarkNode
{
    ReferenceMarkNode dup() { return new ReferenceMarkNode(getName()); }
    ReferenceMarkNode(String name) { super(name); }
    String getType() { return "ReferenceMark"; }
    public boolean equals(Object other) {
        return (other instanceof ReferenceMarkNode)
            && super.equals((MarkNode) other);
    }
}

class ReferenceMarkStartNode extends ReferenceMarkNode
{
    ReferenceMarkStartNode dup()
    { return new ReferenceMarkStartNode(getName()); }
    ReferenceMarkStartNode(String name) {
        super(name); m_isPoint = false; m_isStart = true;
    }
}

class ReferenceMarkEndNode extends ReferenceMarkNode
{
    ReferenceMarkEndNode dup()
    { return new ReferenceMarkEndNode(getName()); }
    ReferenceMarkEndNode(String name) {
        super(name); m_isPoint = false; m_isStart = false;
    }
}

class DocumentIndexMarkNode extends MarkNode
{
    DocumentIndexMarkNode dup()
    { return new DocumentIndexMarkNode(getName()); }
    DocumentIndexMarkNode(String name) { super(name); }
    String getType() { return "DocumentIndexMark"; }
    public boolean equals(Object other) {
        return (other instanceof DocumentIndexMarkNode)
            && super.equals((MarkNode) other);
    }
}

class DocumentIndexMarkStartNode extends DocumentIndexMarkNode
{
    DocumentIndexMarkStartNode dup()
    { return new DocumentIndexMarkStartNode(getName()); }
    DocumentIndexMarkStartNode(String name) {
        super(name); m_isPoint = false; m_isStart = true;
    }
}

class DocumentIndexMarkEndNode extends DocumentIndexMarkNode
{
    DocumentIndexMarkEndNode dup()
    { return new DocumentIndexMarkEndNode(getName()); }
    DocumentIndexMarkEndNode(String name) {
        super(name); m_isPoint = false; m_isStart = false;
    }
}

abstract class ContentNode extends TreeNode
{
    private String m_Content;
    String getContent() { return m_Content; }
    boolean hasContent() { return true; }
    ContentNode(String content) {
        m_Content = content;
    }
    TreeNode appendChild(TreeNode t) {
        throw new RuntimeException("ContentNode.appendChild");
    }
    public String toString() {
        return super.toString() + "\tcontent: " + m_Content;
    }
    boolean equals(ContentNode other) {
        return (other.m_Content.equals(m_Content));
    }
}

class TextNode extends ContentNode
{
    TextNode dup() { return new TextNode(getContent()); }
    TextNode(String content) { super(content); }
    String getType() { return "Text"; }
    public boolean equals(Object other) {
        return (other instanceof TextNode) && super.equals((ContentNode) other);
    }
}

class TextFieldNode extends ContentNode
{
    TextFieldNode dup() { return new TextFieldNode(getContent()); }
    TextFieldNode(String content) { super(content); }
    String getType() { return "TextField"; }
    public boolean equals(Object other) {
        return (other instanceof TextFieldNode)
            && super.equals((ContentNode) other);
    }
}

class FrameNode extends TreeNode
{
    private String m_Name;
    private TextContentAnchorType m_Anchor;
    String getName() { return m_Name; }
    TextContentAnchorType getAnchor() { return m_Anchor; }
    FrameNode dup() { return new FrameNode(getName(), getAnchor()); }
    FrameNode(String name, TextContentAnchorType anchor) {
        m_Name = name; m_Anchor = anchor;
    }
    String getType() { return "Frame"; }
    public boolean equals(Object other) {
        return (other instanceof FrameNode)
            && (((FrameNode) other).m_Name.equals(m_Name))
            && (((FrameNode) other).m_Anchor == m_Anchor);
    }
    public String toString() {
        return super.toString()
            + "\tname: " + m_Name + "\tanchor: " + toString(m_Anchor);
    }
    static String toString(TextContentAnchorType anchor) {
        switch (anchor.getValue()) {
            case AS_CHARACTER_value: return "AS_CHARACTER";
            case AT_CHARACTER_value: return "AT_CHARACTER";
            case AT_PARAGRAPH_value: return "AT_PARAGRAPH";
            case AT_PAGE_value:      return "AT_PAGE";
            case AT_FRAME_value:     return "AT_FRAME";
            default: throw new RuntimeException("unknown anchor");
        }
    }
}

class FootnoteNode extends TreeNode
{
    private String m_Label;
    String getLabel() { return m_Label; }
    FootnoteNode dup() { return new FootnoteNode(getLabel()); }
    FootnoteNode(String label) { m_Label = label; }
    String getType() { return "Footnote"; }
    public boolean equals(Object other) {
        return (other instanceof FootnoteNode)
            && (((FootnoteNode) other).m_Label.equals(m_Label));
    }
    public String toString() {
        return super.toString() + "\tlabel: " + m_Label;
    }
}

class ControlCharacterNode extends TreeNode
{
    private short m_Char;
    short getChar() { return m_Char; }
    ControlCharacterNode dup() { return new ControlCharacterNode(getChar()); }
    ControlCharacterNode(short c) { m_Char = c; }
    String getType() { return "ControlCharacter"; }
    public boolean equals(Object other) {
        return (other instanceof ControlCharacterNode)
            && (((ControlCharacterNode) other).m_Char == m_Char);
    }
    public String toString() {
        return super.toString() + "\tchar: " + m_Char;
    }
}

class SoftPageBreakNode extends TreeNode
{
    String getType() { return "SoftPageBreak"; }
    public boolean equals(Object other) {
        return (other instanceof SoftPageBreakNode);
    }
}

class HyperlinkNode extends TreeNode
{
    private String m_URL;
    String getURL() { return m_URL; }
    HyperlinkNode dup() { return new HyperlinkNode(getURL()); }
    HyperlinkNode(String url) {
        if (url.length() == 0) throw new RuntimeException("HyperlinkNode");
        m_URL = url;
    }
    String getType() { return "Hyperlink"; }
    boolean isNesting() { return true; }
    public boolean equals(Object other) {
        return (other instanceof HyperlinkNode)
            && (((HyperlinkNode) other).m_URL.equals(m_URL));
    }
    public String toString() {
        return super.toString() + "\turl: " + m_URL;
    }
}

class RubyNode extends TreeNode
{
    private String m_Ruby;
    String getRubyText() { return m_Ruby; }
    RubyNode dup() { return new RubyNode(getRubyText()); }
    RubyNode(String ruby) {
        if (ruby.length() == 0) throw new RuntimeException("RubyNode");
        m_Ruby = ruby;
    }
    String getType() { return "Ruby"; }
    boolean isNesting() { return true; }
    public boolean equals(Object other) {
        return (other instanceof RubyNode)
            && (((RubyNode) other).m_Ruby.equals(m_Ruby));
    }
    public String toString() {
        return super.toString() + "\trubytext: " + m_Ruby;
    }
}

class MetaNode extends TreeNode
{
    private StringPair m_XmlId;
    StringPair getXmlId() { return m_XmlId; }
    MetaNode dup() { return new MetaNode(getXmlId()); }
    MetaNode (StringPair xmlid) { m_XmlId = xmlid; }
    String getType() { return "InContentMetadata"; }
    boolean isNesting() { return true; }
    public boolean equals(Object other) {
        return (other instanceof MetaNode)
            && eq(((MetaNode) other).m_XmlId, m_XmlId);
    }
    static boolean eq(StringPair left, StringPair right)
    {
        return left.First.equals(right.First)
            && left.Second.equals(right.Second);
    }
    public String toString() {
        return super.toString()
            + "\txmlid: " + m_XmlId.First + "#" + m_XmlId.Second;
    }
}

class MetaFieldNode extends MetaNode
{
    MetaFieldNode dup() { return new MetaFieldNode(getXmlId()); }
    MetaFieldNode (StringPair xmlid) { super(xmlid); }
    String getType() { return "MetadataField"; }
}

class Range
{
    private short m_Start;
    private short m_End;
    private TreeNode m_Node;
    short getStart()    { return m_Start; }
    short getEnd()      { return m_End  ; }
    short getExtent()   { return (short) (m_End - m_Start); }
    TreeNode getNode()  { return m_Node; }
    Range(int start, int end, TreeNode node)
    { m_Start = (short) start; m_End = (short) end; m_Node = node; }
}

//----------------------------------------------------------------------

/* this is where we nail the pudding to the wall */
class FuzzyTester
{
    private long m_DiffContent = 0;
    private long m_DiffMissing = 0;
    private long m_DiffNesting = 0;
    private long m_DiffSpuriousEmptyText = 0;
    private long m_DiffSequence = 0; // ignored?
    private Stack<Pair<TreeNode, TreeNodeEnum>> m_StackExpected;
    private Stack<Pair<TreeNode, TreeNodeEnum>> m_StackActual;
    private List<TreeNode> m_BufferExpected;
    private List<TreeNode> m_BufferActual;

    FuzzyTester() {
        m_BufferExpected = new ArrayList<TreeNode>();
        m_BufferActual = new ArrayList<TreeNode>();
        m_StackExpected = new Stack<Pair<TreeNode, TreeNodeEnum>>();
        m_StackActual = new Stack<Pair<TreeNode, TreeNodeEnum>>();
    }

    /** idea: traverse both trees, enumerate nodes, stopping at content nodes.
        then compare buffers. */
    void doTest(TreeNode expected, TreeNode actual)
    {
        assertEquals("__ROOT__", expected.getType());
        assertEquals("__ROOT__", actual.getType());
        m_StackExpected.push(new Pair<TreeNode, TreeNodeEnum>(expected, expected.createEnumeration()));
        m_StackActual.push(new Pair<TreeNode, TreeNodeEnum>(actual, actual.createEnumeration()));
        do {
            traverse(m_StackExpected, m_BufferExpected);
            traverse(m_StackActual, m_BufferActual);
        //??? root?
            testBuffer();
        } while (!m_StackExpected.empty() || !m_StackActual.empty());
        if (m_DiffSequence != 0) {
            System.out.println("warning: " +  m_DiffSequence
                    + " differences in sequence");
        }
        if (m_DiffSpuriousEmptyText != 0) {
            System.out.println("warning: " +  m_DiffSpuriousEmptyText
                    + " spurious empty text nodes");
        }
        if (m_DiffNesting != 0) {
            System.out.println("WARNING: " +  m_DiffNesting
                    + " differences in nesting");
        }
        assertEquals(0, m_DiffContent);
        assertEquals(0, m_DiffMissing);
    }

    private void traverse(Stack<Pair<TreeNode, TreeNodeEnum>> stack,
                  List<TreeNode> buffer)
    {
        while (!stack.empty()) {
            TreeNodeEnum topEnum = stack.peek().Second;
            if (topEnum.hasNext()) {
                TreeNode node = topEnum.next();
                buffer.add(node);
                TreeNodeEnum nodeEnum = node.createEnumeration();
                if (nodeEnum.hasNext()) {
                    stack.push(new Pair<TreeNode, TreeNodeEnum>(node, nodeEnum));
                }
                if (node.hasContent()) {
                    if (!((node instanceof TextNode) // spurious empty text?
                        && ((TextNode) node).getContent().length() == 0)) {
                            return; // break here
                    }
                }
            } else {
                buffer.add(stack.peek().First);
                stack.pop();
            }
        }
    }

    private void testTerminatingNode()
    {
        int lenExpected = m_BufferExpected.size();
        int lenActual   = m_BufferActual.size();
        if (lenExpected == 0 || lenActual == 0)
            return;
        TreeNode expected = m_BufferExpected.get(lenExpected - 1);
        TreeNode actual = m_BufferActual.get(lenActual - 1);

        boolean eRoot = expected.getType().equals("__ROOT__");
        boolean aRoot = actual.getType().equals("__ROOT__");
        if (eRoot || aRoot) {
            if (!(eRoot && aRoot)) {
                if (aRoot) printMissing(expected);
                else       printUnexpected(actual);
                m_DiffMissing++;
            }
            return;
        }

        testContentNode((ContentNode) expected, (ContentNode) actual);

        m_BufferExpected.set(lenExpected - 1, null);
        m_BufferActual.set(lenActual - 1, null);
    }

    private void testContentNode(ContentNode expected, ContentNode actual)
    {
        String contentExpected = expected.getContent();
        String contentActual = actual.getContent();
        if (!expected.equals(actual)) {
            printDiff("text content differs", contentExpected, contentActual);
            m_DiffContent++;
        }
    }

    private void testBuffer()
    {
        int lenExpected = m_BufferExpected.size();
        int lenActual   = m_BufferActual.size();
        for (int i = 0; i < lenExpected - 1; i++ )
        {
            TreeNode node = m_BufferExpected.get(i);
            int j = m_BufferActual.indexOf(node);
            if (j >= 0) {
                TreeNode other = m_BufferActual.get(j);
                if (j != i)
                {
                    //FIXME how bad is this?
                    printDiff("position differs",
                            String.valueOf(i), String.valueOf(j));
                    // a hacky hack
                    int min = Math.min(i,j);
                    int max = Math.max(Math.min(lenActual - 1, i),j);
                    for (int k = min; k != max; k ++) {
                        TreeNode tmp = m_BufferActual.get(k);
                        if (tmp != null && tmp.isNesting()) {
                            printNesting(node, tmp);
                            m_DiffNesting++;
                        }
                    }
                    m_DiffSequence++;
                }
                m_BufferActual.set(j, null);
            } else {
//System.out.println("testBuffer:");
                printMissing(node);
                m_DiffMissing++;
            }
        }
        for (int j = 0; j < lenActual - 1; j++)
        {
            TreeNode node = m_BufferActual.get(j);
            if (node != null)
            {
//System.out.println("testBuffer:");
                printUnexpected(node);
                if ((node instanceof TextNode) &&
                        ((TextNode) node).getContent().length() == 0) {
                    m_DiffSpuriousEmptyText++;
                } else {
                    m_DiffMissing++;
                }
            }
        }
        testTerminatingNode();
        m_BufferExpected.clear();
        m_BufferActual.clear();
    }

    void printDiff(String prefix, String expected, String actual)
    {
        System.out.println(prefix +
                ":\texpected: " + expected + "\tactual: " + actual);
    }

    void printNesting(TreeNode node, TreeNode nesting)
    {
        System.out.println("node: " + node.toString()
                + " possibly moved across nesting " + nesting.toString());
    }

    void printMissing(TreeNode node)
    {
        System.out.println("   missing node: " + node.toString());

    }

    void printUnexpected(TreeNode node)
    {
        System.out.println("unexpected node: " + node.toString());

    }
}


//----------------------------------------------------------------------

class EnumConverter
{
    private Stack<TreeNode> m_Stack;

    EnumConverter() {
        m_Stack = new Stack<TreeNode>();
    }

    TreeNode convert(XEnumeration xEnum) throws Exception
    {
        TreeNode root = new TreeNode();
        m_Stack.push(root);
        TreeNode ret = convertChildren(xEnum);
        assertTrue("EnumConverter.convert: stack", m_Stack.empty());
        return ret;
    }

    TreeNode convertChildren(XEnumeration xEnum) throws Exception
    {
        while (xEnum.hasMoreElements()) {
            TreeNode node;
            Object xElement = xEnum.nextElement();
            XTextRange xPortion = UnoRuntime.queryInterface(XTextRange.class, xElement);
            XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xPortion);
            String type = (String) xPropSet.getPropertyValue("TextPortionType");
            if (type.equals("Text")) {
                String text = xPortion.getString();
                node = new TextNode(text);
                String url = (String) xPropSet.getPropertyValue("HyperLinkURL");
                if (url.length() > 0) {
                    TreeNode temp = node;
                    node = new HyperlinkNode(url);
                    node.appendChild(temp);
                }
            } else if (type.equals("TextField")) {
                Object xField = xPropSet.getPropertyValue("TextField");
                XServiceInfo xService = UnoRuntime.queryInterface(XServiceInfo.class, xField);
                if (xService.supportsService(
                        "com.sun.star.text.textfield.MetadataField"))
                {
                    XMetadatable xMeta = UnoRuntime.queryInterface(XMetadatable.class, xField);
                    StringPair xmlid = xMeta.getMetadataReference();
                    node = new MetaFieldNode(xmlid);
                    m_Stack.push(node);
                    XEnumerationAccess xEA = UnoRuntime.queryInterface(XEnumerationAccess.class,
                    xMeta);
                    XEnumeration xEnumChildren = xEA.createEnumeration();
                    TreeNode node2 = convertChildren(xEnumChildren);
                    assertSame("stack error: meta-field", node2, node);
                } else {
                    XPropertySet xFieldPropSet = UnoRuntime.queryInterface(XPropertySet.class, xField);
                    String content = (String)
                        xFieldPropSet.getPropertyValue("Content");
                    boolean isFixed = (Boolean)
                        xFieldPropSet.getPropertyValue("IsFixed");
                    assertTrue("field not fixed?", isFixed);
                    node = new TextFieldNode(content);
                }
            } else if (type.equals("Footnote")) {
                Object xNote = xPropSet.getPropertyValue("Footnote");
                XFootnote xFootnote = UnoRuntime.queryInterface(XFootnote.class, xNote);
                String label = xFootnote.getLabel();
                node = new FootnoteNode(label);
            } else if (type.equals("Frame")) {
                XContentEnumerationAccess xCEA = UnoRuntime.queryInterface(XContentEnumerationAccess.class,
                    xPortion);
                XEnumeration xContentEnum = xCEA.createContentEnumeration("");
                while (xContentEnum.hasMoreElements()) {
                    Object xFrame = xContentEnum.nextElement();
                    XPropertySet xFramePropSet = UnoRuntime.queryInterface(XPropertySet.class, xFrame);
                    TextContentAnchorType anchor = (TextContentAnchorType)
                        xFramePropSet.getPropertyValue("AnchorType");
                    XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, xFrame);
                    String name = xNamed.getName();
                    node = new FrameNode(name, anchor);
                    m_Stack.peek().appendChild(node);
                }
                continue;
            } else if (type.equals("ControlCharacter")) {
                short c = (Short)
                    xPropSet.getPropertyValue("ControlCharacter");
                node = new ControlCharacterNode(c);
            } else if (type.equals("Bookmark")) {
                Object xMark = xPropSet.getPropertyValue("Bookmark");
                XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, xMark);
                String name = xNamed.getName();
                XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xMark);
                StringPair xmlid = xMetadatable.getMetadataReference();
                boolean isCollapsed = (Boolean)
                    xPropSet.getPropertyValue("IsCollapsed");
                if (isCollapsed) {
                    node = new BookmarkNode(name, xmlid);
                } else {
                    boolean isStart = (Boolean)
                        xPropSet.getPropertyValue("IsStart");
                    if (isStart) {
                        node = new BookmarkStartNode(name, xmlid);
                    } else {
                        node = new BookmarkEndNode(name, xmlid);
                    }
                }
            } else if (type.equals("ReferenceMark")) {
                Object xMark = xPropSet.getPropertyValue("ReferenceMark");
                XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, xMark);
                String name = xNamed.getName();
                boolean isCollapsed = (Boolean)
                    xPropSet.getPropertyValue("IsCollapsed");
                if (isCollapsed) {
                    node = new ReferenceMarkNode(name);
                } else {
                    boolean isStart = (Boolean)
                        xPropSet.getPropertyValue("IsStart");
                    if (isStart) {
                        node = new ReferenceMarkStartNode(name);
                    } else {
                        node = new ReferenceMarkEndNode(name);
                    }
                }
            } else if (type.equals("DocumentIndexMark")) {
                Object xMark = xPropSet.getPropertyValue("DocumentIndexMark");
                XPropertySet xMarkSet = UnoRuntime.queryInterface(XPropertySet.class, xMark);
                String name = (String) xMarkSet.getPropertyValue("PrimaryKey");
                boolean isCollapsed = (Boolean)
                    xPropSet.getPropertyValue("IsCollapsed");
                if (isCollapsed) {
                    node = new DocumentIndexMarkNode(name);
                } else {
                    boolean isStart = (Boolean)
                        xPropSet.getPropertyValue("IsStart");
                    if (isStart) {
                        node = new DocumentIndexMarkStartNode(name);
                    } else {
                        node = new DocumentIndexMarkEndNode(name);
                    }
                }
            } else if (type.equals("SoftPageBreak")) {
                node = new SoftPageBreakNode();
            } else if (type.equals("Ruby")) {
                boolean isStart = (Boolean)
                    xPropSet.getPropertyValue("IsStart");
                if (isStart) {
                    // ARRGH!!! stupid api...
                    // the text is ONLY at the start!
                    String ruby = (String)
                        xPropSet.getPropertyValue("RubyText");
                    node = new RubyNode(ruby);
                    m_Stack.push(node);
                    continue;
                } else {
                    node = m_Stack.pop();
                    assertTrue("stack error: Ruby expected; is: " +
                               node.toString(), node instanceof RubyNode);
                }
            } else if (type.equals("InContentMetadata")) {
                Object xMeta = xPropSet.getPropertyValue("InContentMetadata");
                XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xMeta);
                StringPair xmlid = xMetadatable.getMetadataReference();
                node = new MetaNode(xmlid);
                m_Stack.push(node);
                XEnumerationAccess xEA = UnoRuntime.queryInterface(XEnumerationAccess.class, xMeta);
                XEnumeration xEnumChildren = xEA.createEnumeration();
                TreeNode node2 = convertChildren(xEnumChildren);
                assertSame("stack error: meta", node2, node);
            } else {
                throw new RuntimeException("unexpected type: " + type);
            }
            m_Stack.peek().appendChild(node);
        }
        TreeNode ret = m_Stack.pop();
        return ret;
    }
}


//----------------------------------------------------------------------

abstract class Inserter
{
    private XMultiServiceFactory m_xDocFactory;
    XText m_xText;
    XParagraphCursor m_xCursor;

    Inserter(XTextDocument xDoc)
    {
        m_xDocFactory = UnoRuntime.queryInterface(XMultiServiceFactory.class, xDoc);
        m_xText = xDoc.getText();
        XTextCursor xCursor = m_xText.createTextCursor();
        m_xCursor = UnoRuntime.queryInterface(XParagraphCursor.class, xCursor);
    }

    void initParagraph() throws Exception
    {
        m_xCursor.gotoStartOfParagraph(false);
        m_xText.insertControlCharacter(m_xCursor, PARAGRAPH_BREAK, false);
        // we split the first (empty) paragraph, and then insert into the
        // second (empty) paragraph; this ensures first is always empty!
    }

    void insertControlCharacter(XTextCursor xCursor, short cchar)
        throws Exception
    {
        m_xText.insertControlCharacter(xCursor, cchar, false);
    }

    void insertText(XTextCursor xCursor, String text)
    {
        xCursor.setString(text);
    }

    void insertTextField(XTextCursor xCursor, String content) throws Exception
    {
        XTextContent xContent = makeTextField(content);
        xContent.attach(xCursor);
    }

    XTextContent makeTextField(String content)
        throws Exception
    {
        Object xField =
            m_xDocFactory.createInstance("com.sun.star.text.textfield.Author");
        XTextContent xContent = UnoRuntime.queryInterface(XTextContent.class, xField);
        XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xField);
        xPropSet.setPropertyValue("IsFixed", true);
        xPropSet.setPropertyValue("FullName", false);
        xPropSet.setPropertyValue("Content", content);
        return xContent;
    }

    void insertFrame(XTextRange xCursor, String name,
            TextContentAnchorType anchor) throws Exception
    {
        XTextContent xContent = makeFrame(name, anchor);
        xContent.attach(xCursor);
    }

    XTextContent makeFrame(String name, TextContentAnchorType anchor)
        throws Exception
    {
        Object xFrame =
            m_xDocFactory.createInstance("com.sun.star.text.TextFrame");
        XTextContent xContent = UnoRuntime.queryInterface(XTextContent.class, xFrame);
        XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xFrame);
        xPropSet.setPropertyValue("AnchorType", anchor);
        XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, xContent);
        xNamed.setName(name);
        return xContent;
    }

    void insertFootnote(XTextCursor xCursor, String label) throws Exception
    {
        XTextContent xContent = makeFootnote(label);
        xContent.attach(xCursor);
    }

    XTextContent makeFootnote(String label) throws Exception
    {
        Object xNote =
            m_xDocFactory.createInstance("com.sun.star.text.Footnote");
        XTextContent xContent = UnoRuntime.queryInterface(XTextContent.class, xNote);
        XFootnote xFootnote = UnoRuntime.queryInterface(XFootnote.class, xNote);
        xFootnote.setLabel(label);
        return xContent;
    }

    void insertBookmark(XTextCursor xCursor, String name, StringPair xmlid)
        throws Exception
    {
        XTextContent xContent = makeBookmark(name);
        xContent.attach(xCursor);
        if (!xmlid.First.equals(""))
        {
            XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xContent);
            xMetadatable.setMetadataReference(xmlid);
        }
    }

    XTextContent makeBookmark(String name) throws Exception
    {
        Object xBookmark =
            m_xDocFactory.createInstance("com.sun.star.text.Bookmark");
        XTextContent xContent = UnoRuntime.queryInterface(XTextContent.class, xBookmark);
        XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, xContent);
        xNamed.setName(name);
        return xContent;
    }

    void insertReferenceMark(XTextCursor xCursor, String name) throws Exception
    {
        XTextContent xContent = makeReferenceMark(name);
        xContent.attach(xCursor);
    }

    XTextContent makeReferenceMark(String name) throws Exception
    {
        Object xMark =
            m_xDocFactory.createInstance("com.sun.star.text.ReferenceMark");
        XTextContent xContent = UnoRuntime.queryInterface(XTextContent.class, xMark);
        XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, xContent);
        xNamed.setName(name);
        return xContent;
    }

    void insertDocumentIndexMark(XTextCursor xCursor, String key)
        throws Exception
    {
        XTextContent xContent = makeDocumentIndexMark(key);
        xContent.attach(xCursor);
    }

    XTextContent makeDocumentIndexMark(String key) throws Exception
    {
        Object xMark =
            m_xDocFactory.createInstance("com.sun.star.text.DocumentIndexMark");
        XTextContent xContent = UnoRuntime.queryInterface(XTextContent.class, xMark);
        XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xMark);
        xPropSet.setPropertyValue("PrimaryKey", key);
        return xContent;
    }

    void insertHyperlink(XTextCursor xCursor, String url) throws Exception
    {
        XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xCursor);
        xPropSet.setPropertyValue("HyperLinkURL", url);
    }

    void insertRuby(XTextCursor xCursor, String rubytext) throws Exception
    {
        XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xCursor);
        xPropSet.setPropertyValue("RubyText", rubytext);
    }

    XTextContent insertMeta(XTextCursor xCursor, StringPair xmlid)
        throws Exception
    {
        XTextContent xContent = makeMeta();
        xContent.attach(xCursor);
        XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xContent);
        xMetadatable.setMetadataReference(xmlid);
        return xContent;
    }

    XTextContent makeMeta() throws Exception
    {
        Object xMeta = m_xDocFactory.createInstance(
                "com.sun.star.text.InContentMetadata");
        XTextContent xContent = UnoRuntime.queryInterface(XTextContent.class, xMeta);
        return xContent;
    }

    XTextField insertMetaField(XTextCursor xCursor, StringPair xmlid)
        throws Exception
    {
        XTextField xContent = makeMetaField();
        xContent.attach(xCursor);
        XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xContent);
        xMetadatable.setMetadataReference(xmlid);
        return xContent;
    }

    XTextField makeMetaField() throws Exception
    {
        Object xMeta = m_xDocFactory.createInstance(
                "com.sun.star.text.textfield.MetadataField");
        XTextField xContent = UnoRuntime.queryInterface(XTextField.class, xMeta);
        return xContent;
    }

}

class TreeInserter extends Inserter
{
    private Map<String, XTextRange> m_BookmarkStarts;
    private Map<String, XTextRange> m_ReferenceMarkStarts;
    private Map<String, XTextRange> m_DocumentIndexMarkStarts;
    private List<Pair<XTextRange, FrameNode>> m_FrameHints;

    TreeInserter(XTextDocument xDoc)
    {
        super(xDoc);
        m_BookmarkStarts = new HashMap<String, XTextRange>();
        m_ReferenceMarkStarts = new HashMap<String, XTextRange>();
        m_DocumentIndexMarkStarts = new HashMap<String, XTextRange>();
        m_FrameHints = new ArrayList<Pair<XTextRange, FrameNode>>();
    }

    void insertTree(TreeNode tree) throws Exception
    {
        if (!tree.getType().equals("__ROOT__"))
            throw new RuntimeException("insertTree: test error: no root");
        initParagraph();
        insertChildren(tree.createEnumeration());
        for (int i = 0; i < m_FrameHints.size(); ++i) {
            Pair<XTextRange, FrameNode> p = m_FrameHints.get(i);
            insertFrame(p.First, p.Second.getName(), p.Second.getAnchor());
        }
    }

    void insertChildren(TreeNodeEnum children) throws Exception
    {
        while (children.hasNext()) {
            m_xCursor.gotoEndOfParagraph(false);
            TreeNode node = children.next();
            String type = node.getType();
            if (type.equals("Bookmark")) {
                BookmarkNode bkmk = (BookmarkNode) node;
                String name = bkmk.getName();
                StringPair id = bkmk.getXmlId();
                if (bkmk.isPoint()) {
                    insertBookmark(m_xCursor, name, id);
                } else if (bkmk.isStart()) {
                    m_BookmarkStarts.put(name, m_xCursor.getStart());
                } else {
                    XTextRange xRange = m_BookmarkStarts.get(name);
                    XParagraphCursor xCursor = mkCursor(xRange);
                    insertBookmark(xCursor, name, id);
                }
            } else if (type.equals("ReferenceMark")) {
                ReferenceMarkNode mark = (ReferenceMarkNode) node;
                String name = mark.getName();
                if (mark.isPoint()) {
                    insertReferenceMark(m_xCursor, name);
                } else if (mark.isStart()) {
                    m_ReferenceMarkStarts.put(name, m_xCursor.getStart());
                } else {
                    XTextRange xRange = m_ReferenceMarkStarts.get(name);
                    XParagraphCursor xCursor = mkCursor(xRange);
                    insertReferenceMark(xCursor, name);
                }
            } else if (type.equals("DocumentIndexMark")) {
                DocumentIndexMarkNode mark = (DocumentIndexMarkNode) node;
                String name = mark.getName();
                if (mark.isPoint()) {
                    insertDocumentIndexMark(m_xCursor, name);
                } else if (mark.isStart()) {
                    m_DocumentIndexMarkStarts.put(name, m_xCursor.getStart());
                } else {
                    XTextRange xRange = m_DocumentIndexMarkStarts.get(name);
                    XParagraphCursor xCursor = mkCursor(xRange);
                    insertDocumentIndexMark(xCursor, name);
                }
            } else if (type.equals("Hyperlink")) {
                HyperlinkNode href = (HyperlinkNode) node;
                XTextRange xRange = m_xCursor.getStart();
                insertChildren(href.createEnumeration());
                XParagraphCursor xCursor = mkCursor(xRange);
                insertHyperlink(xCursor, href.getURL());
            } else if (type.equals("Ruby")) {
                RubyNode ruby = (RubyNode) node;
                XTextRange xRange = m_xCursor.getStart();
                insertChildren(ruby.createEnumeration());
                XParagraphCursor xCursor = mkCursor(xRange);
                insertRuby(xCursor, ruby.getRubyText());
            } else if (type.equals("InContentMetadata")) {
                MetaNode meta = (MetaNode) node;
                XTextRange xRange = m_xCursor.getStart();
                insertChildren(meta.createEnumeration());
                XParagraphCursor xCursor = mkCursor(xRange);
                insertMeta(xCursor, meta.getXmlId());
            } else if (type.equals("MetadataField")) {
                MetaFieldNode meta = (MetaFieldNode) node;
                XTextRange xRange = m_xCursor.getStart();
                insertChildren(meta.createEnumeration());
                XParagraphCursor xCursor = mkCursor(xRange);
                insertMetaField(xCursor, meta.getXmlId());
            } else if (type.equals("Text")) {
                TextNode text = (TextNode) node;
                insertText(m_xCursor, text.getContent());
            } else if (type.equals("TextField")) {
                TextFieldNode field = (TextFieldNode) node;
                insertTextField(m_xCursor, field.getContent());
            } else if (type.equals("Footnote")) {
                FootnoteNode note = (FootnoteNode) node;
                insertFootnote(m_xCursor, note.getLabel());
            } else if (type.equals("Frame")) {
                FrameNode frame = (FrameNode) node;
                if (frame.getAnchor() == AT_CHARACTER) {
                    m_FrameHints.add( new Pair<XTextRange, FrameNode>(m_xCursor.getStart(), frame) );
                } else {
                    insertFrame(m_xCursor, frame.getName(), frame.getAnchor());
                }
            } else if (type.equals("ControlCharacter")) {
                ControlCharacterNode cchar = (ControlCharacterNode) node;
                insertControlCharacter(m_xCursor, cchar.getChar());
            } else if (type.equals("SoftPageBreak")) {
                SoftPageBreakNode spbk = (SoftPageBreakNode) node;
                throw new RuntimeException("sorry, cannot test SoftPageBreak");
            } else {
                throw new RuntimeException("unexpected type: " + type);
            }
        }
    }

    XParagraphCursor mkCursor(XTextRange xRange)
    {
        XTextCursor xCursor = m_xText.createTextCursorByRange(xRange);
        XParagraphCursor xParaCursor = UnoRuntime.queryInterface(XParagraphCursor.class, xCursor);
        xParaCursor.gotoEndOfParagraph(true);
        return xParaCursor;
    }
}


// FIXME: this does not account for inserted dummy characters!
class RangeInserter extends Inserter
{
    RangeInserter(XTextDocument xDoc) throws Exception
    { super(xDoc); initParagraph(); }

    /*
    void insertText(int pos, String text)
    {
        m_xCursor.gotoStartOfParagraph(false);
        m_xCursor.goRight((short) pos, false);
        insertText(m_xCursor, text);
    }
    */

    XTextContent insertRange(Range range) throws Exception
    {
        m_xCursor.gotoStartOfParagraph(false);
        m_xCursor.goRight(range.getStart(), false);
        m_xCursor.goRight(range.getExtent(), true);
        return insertNode(m_xCursor, range.getNode());
    }

    XTextContent insertNode(XParagraphCursor xCursor, TreeNode node)
        throws Exception
    {
        String type = node.getType();
        if (type.equals("Bookmark")) {
            BookmarkNode bkmk = (BookmarkNode) node;
            if (bkmk.isPoint()) throw new RuntimeException("range only");
            insertBookmark(xCursor, bkmk.getName(), bkmk.getXmlId());
        } else if (type.equals("ReferenceMark")) {
            ReferenceMarkNode mark = (ReferenceMarkNode) node;
            if (mark.isPoint()) throw new RuntimeException("range only");
            insertReferenceMark(xCursor, mark.getName());
        } else if (type.equals("DocumentIndexMark")) {
            DocumentIndexMarkNode mark = (DocumentIndexMarkNode) node;
            if (mark.isPoint()) throw new RuntimeException("range only");
            insertDocumentIndexMark(xCursor, mark.getName());
        } else if (type.equals("Hyperlink")) {
            HyperlinkNode href = (HyperlinkNode) node;
            insertHyperlink(xCursor, href.getURL());
        } else if (type.equals("Ruby")) {
            RubyNode ruby = (RubyNode) node;
            insertRuby(xCursor, ruby.getRubyText());
        } else if (type.equals("InContentMetadata")) {
            MetaNode meta = (MetaNode) node;
            return insertMeta(xCursor, meta.getXmlId());
        } else if (type.equals("MetadataField")) {
            MetaFieldNode meta = (MetaFieldNode) node;
            return insertMetaField(xCursor, meta.getXmlId());
        } else if (type.equals("Text")) {
            TextNode text = (TextNode) node;
            insertText(xCursor, text.getContent());
        } else if (type.equals("TextField")) {
            TextFieldNode field = (TextFieldNode) node;
            insertTextField(m_xCursor, field.getContent());
        } else if (type.equals("Footnote")) {
            FootnoteNode note = (FootnoteNode) node;
            insertFootnote(m_xCursor, note.getLabel());
        } else if (type.equals("Frame")) {
            FrameNode frame = (FrameNode) node;
            insertFrame(xCursor, frame.getName(), frame.getAnchor());
        } else if (type.equals("ControlCharacter")) {
            ControlCharacterNode cchar = (ControlCharacterNode) node;
            insertControlCharacter(m_xCursor, cchar.getChar());
        } else if (type.equals("SoftPageBreak")) {
            throw new RuntimeException("sorry, cannot test SoftPageBreak");
        } else {
            throw new RuntimeException("unexpected type: " + type);
        }
        return null;
    }
}


//----------------------------------------------------------------------

public class TextPortionEnumerationTest
{
    private XMultiServiceFactory m_xMSF = null;
    private XComponentContext m_xContext = null;
    private XTextDocument m_xDoc = null;
    private String m_TmpDir = null;

    private int m_Count = 1;

    @Before public void before() throws Exception
    {
        m_xMSF = UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            connection.getComponentContext().getServiceManager());
        XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, m_xMSF);
        Object defaultCtx = xPropertySet.getPropertyValue("DefaultContext");
        m_xContext = UnoRuntime.queryInterface(XComponentContext.class, defaultCtx);
        assertNotNull("could not get component context.", m_xContext);
        m_xDoc = util.WriterTools.createTextDoc(m_xMSF);
        m_TmpDir = util.utils.getOfficeTemp/*Dir*/(m_xMSF);
        System.out.println("tempdir: " + m_TmpDir);
    }

    @After public void after()
    {
        util.DesktopTools.closeDoc(m_xDoc);
    }

    @Test public void testText() throws Exception
    {
        TreeNode root = new TreeNode();
        TreeNode text = new TextNode("abc");
        root.appendChild(text);
        doTest(root);
    }

    @Test public void testTextField() throws Exception
    {
        String name = mkName("ruby");
        TreeNode root = new TreeNode();
        TreeNode txtf = new TextFieldNode("abc");
        root.appendChild(txtf);
        doTest(root);
    }

    /*@Test*/ public void testControlChar() throws Exception
    {
//FIXME this is converted to a text portion: ControlCharacter is obsolete
        TreeNode root = new TreeNode();
        TreeNode cchr = new ControlCharacterNode(HARD_HYPHEN);
        root.appendChild(cchr);
        doTest(root);
    }

    /*@Test*/ public void testSoftPageBreak() throws Exception
    {
//FIXME: insert a soft page break: not done
        TreeNode root = new TreeNode();
        TreeNode spbk = new SoftPageBreakNode();
        TreeNode text = new TextNode("abc");
        root.appendChild(spbk);
        root.appendChild(text);
        doTest(root);
    }

    @Test public void testFootnote() throws Exception
    {
        String name = mkName("ftn");
        TreeNode root = new TreeNode();
        TreeNode ftnd = new FootnoteNode(name);
        root.appendChild(ftnd);
        doTest(root);
    }

    @Test public void testFrameAs() throws Exception
    {
        String name = mkName("frame");
        TreeNode root = new TreeNode();
        TreeNode fram = new FrameNode(name, AS_CHARACTER);
        root.appendChild(fram);
        doTest(root);
    }

    @Test public void testFrameAt() throws Exception
    {
        String name = mkName("frame");
        TreeNode root = new TreeNode();
//        TreeNode text = new TextNode(""); // necessary?
        TreeNode fram = new FrameNode(name, AT_CHARACTER);
//        root.appendChild(text);
        root.appendChild(fram);
        doTest(root);
    }

    @Test public void testBookmarkPoint() throws Exception
    {
        String name = mkName("mark");
        TreeNode root = new TreeNode();
        TreeNode bkmk = new BookmarkNode(name);
        TreeNode text = new TextNode("abc");
        root.appendChild(bkmk);
        root.appendChild(text);
        doTest(root);
    }

    @Test public void testBookmark() throws Exception
    {
        String name = mkName("mark");
        TreeNode root = new TreeNode();
        TreeNode bkm1 = new BookmarkStartNode(name);
        TreeNode text = new TextNode("abc");
        TreeNode bkm2 = new BookmarkEndNode(name);
        root.appendChild(bkm1);
        root.appendChild(text);
        root.appendChild(bkm2);
        doTest(root);
    }

    @Test public void testBookmarkPointXmlId() throws Exception
    {
        String name = mkName("mark");
        StringPair id = mkId("id");
        TreeNode root = new TreeNode();
        TreeNode bkmk = new BookmarkNode(name, id);
        TreeNode text = new TextNode("abc");
        root.appendChild(bkmk);
        root.appendChild(text);
        doTest(root);
    }

    @Test public void testBookmarkXmlId() throws Exception
    {
        String name = mkName("mark");
        StringPair id = mkId("id");
        TreeNode root = new TreeNode();
        TreeNode bkm1 = new BookmarkStartNode(name, id);
        TreeNode text = new TextNode("abc");
        TreeNode bkm2 = new BookmarkEndNode(name, id);
        root.appendChild(bkm1);
        root.appendChild(text);
        root.appendChild(bkm2);
        doTest(root);
    }

    @Test public void testRefmarkPoint() throws Exception
    {
        String name = mkName("refmark");
        TreeNode root = new TreeNode();
        TreeNode rfmk = new ReferenceMarkNode(name);
        TreeNode text = new TextNode("abc");
        root.appendChild(rfmk);
        root.appendChild(text);
        doTest(root);
    }

    @Test public void testRefmark() throws Exception
    {
        String name = mkName("refmark");
        TreeNode root = new TreeNode();
        TreeNode rfm1 = new ReferenceMarkStartNode(name);
        TreeNode text = new TextNode("abc");
        TreeNode rfm2 = new ReferenceMarkEndNode(name);
        root.appendChild(rfm1);
        root.appendChild(text);
        root.appendChild(rfm2);
        doTest(root);
    }

    @Test public void testToxmarkPoint() throws Exception
    {
        String name = mkName("toxmark");
        TreeNode root = new TreeNode();
        TreeNode txmk = new DocumentIndexMarkNode(name);
        TreeNode text = new TextNode("abc");
        root.appendChild(txmk);
        root.appendChild(text);
        doTest(root);
    }

    @Test public void testToxmark() throws Exception
    {
        String name = mkName("toxmark");
        TreeNode root = new TreeNode();
        TreeNode txm1 = new DocumentIndexMarkStartNode(name);
        TreeNode text = new TextNode("abc");
        TreeNode txm2 = new DocumentIndexMarkEndNode(name);
        root.appendChild(txm1);
        root.appendChild(text);
        root.appendChild(txm2);
        doTest(root);
    }

    @Test public void testHyperlink() throws Exception
    {
        String name = mkName("url");
        TreeNode root = new TreeNode();
        TreeNode href = new HyperlinkNode(name);
        TreeNode text = new TextNode("abc");
        href.appendChild(text);
        root.appendChild(href);
        doTest(root);
    }

    @Test public void testHyperlinkEmpty() throws Exception
    {
        String name = mkName("url");
        TreeNode root = new TreeNode();
        TreeNode href = new HyperlinkNode(name);
        TreeNode text = new TextNode("");
        href.appendChild(text);
        root.appendChild(href);
        doTest(root);
    }

    @Test public void testRuby() throws Exception
    {
        String name = mkName("ruby");
        TreeNode root = new TreeNode();
        TreeNode ruby = new RubyNode(name);
        TreeNode text = new TextNode("abc");
        ruby.appendChild(text);
        root.appendChild(ruby);
        doTest(root);
    }

    @Test public void testRubyEmpty() throws Exception
    {
        // BUG: #i91534#
        String name = mkName("ruby");
        TreeNode root = new TreeNode();
        TreeNode ruby = new RubyNode(name);
        root.appendChild(ruby);
        doTest(root);
    }

    @Test public void testMeta() throws Exception
    {
        StringPair id = new StringPair("content.xml", mkName("id"));
        TreeNode root = new TreeNode();
        TreeNode meta = new MetaNode(id);
        TreeNode text = new TextNode("abc");
        root.appendChild(new TextNode("123"));
        meta.appendChild(text);
        root.appendChild(meta);
        doTest(root);
    }

    @Test public void testMetaEmpty() throws Exception
    {
        StringPair id = new StringPair("content.xml", mkName("id"));
        TreeNode root = new TreeNode();
        TreeNode meta = new MetaNode(id);
//        TreeNode text = new TextNode("");
//        meta.appendChild(text);
        root.appendChild(meta);
        doTest(root);
    }

    @Test public void testMetaField() throws Exception
    {
        StringPair id = new StringPair("content.xml", mkName("id"));
        TreeNode root = new TreeNode();
        TreeNode meta = new MetaFieldNode(id);
        TreeNode text = new TextNode("abc");
        root.appendChild(new TextNode("123"));
        meta.appendChild(text);
        root.appendChild(meta);
        doTest(root);
    }

    @Test public void testMetaFieldEmpty() throws Exception
    {
        StringPair id = new StringPair("content.xml", mkName("id"));
        TreeNode root = new TreeNode();
        TreeNode meta = new MetaFieldNode(id);
//        TreeNode text = new TextNode("");
//        meta.appendChild(text);
        root.appendChild(meta);
        doTest(root);
    }

    @Test public void testBookmark1() throws Exception
    {
        String name1 = mkName("mark");
        String name2 = mkName("mark");
        String name3 = mkName("mark");
        TreeNode root = new TreeNode();
        root.appendChild( new BookmarkStartNode(name1) );
        root.appendChild( new BookmarkNode(name2) );
        root.appendChild( new BookmarkStartNode(name3) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new BookmarkEndNode(name1) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new BookmarkEndNode(name3) );
        doTest(root);
    }

    @Test public void testBookmark2() throws Exception
    {
        String name1 = mkName("mark");
        String name2 = mkName("mark");
        String name3 = mkName("mark");
        TreeNode root = new TreeNode();
        root.appendChild( new BookmarkStartNode(name1) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new BookmarkNode(name2) );
        root.appendChild( new BookmarkStartNode(name3) );
        root.appendChild( new BookmarkEndNode(name1) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new BookmarkEndNode(name3) );
        doTest(root);
    }

    @Test public void testRefMark2() throws Exception
    {
        String name1 = mkName("refmark");
        TreeNode root = new TreeNode();
        root.appendChild( new ReferenceMarkStartNode(name1) );
        root.appendChild( new TextNode("abc") );
        // BUG: #i102541# (this is actually not unoportenum's fault)
        root.appendChild( new ReferenceMarkEndNode(name1) );
        root.appendChild( new TextNode("de") );
        doTest(root);
    }

    @Test public void testRefMark3() throws Exception
    {
        // BUG: #i107672# (non-deterministic; depends on pointer ordering)
        String name1 = mkName("refmark");
        String name2 = mkName("refmark");
        String name3 = mkName("refmark");
        String name4 = mkName("refmark");
        String name5 = mkName("refmark");
        String name6 = mkName("refmark");
        String name7 = mkName("refmark");
        TreeNode root = new TreeNode();
        root.appendChild( new ReferenceMarkStartNode(name1) );
        root.appendChild( new ReferenceMarkStartNode(name2) );
        root.appendChild( new ReferenceMarkStartNode(name3) );
        root.appendChild( new ReferenceMarkStartNode(name4) );
        root.appendChild( new ReferenceMarkStartNode(name5) );
        root.appendChild( new ReferenceMarkStartNode(name6) );
        root.appendChild( new ReferenceMarkStartNode(name7) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new ReferenceMarkEndNode(name7) );
        root.appendChild( new ReferenceMarkEndNode(name6) );
        root.appendChild( new ReferenceMarkEndNode(name5) );
        root.appendChild( new ReferenceMarkEndNode(name4) );
        root.appendChild( new ReferenceMarkEndNode(name3) );
        root.appendChild( new ReferenceMarkEndNode(name2) );
        root.appendChild( new ReferenceMarkEndNode(name1) );
        root.appendChild( new TextNode("de") );
        doTest(root);
    }

    @Test public void testToxMark2() throws Exception
    {
        String name1 = mkName("toxmark");
        TreeNode root = new TreeNode();
        root.appendChild( new DocumentIndexMarkStartNode(name1) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new DocumentIndexMarkEndNode(name1) );
        root.appendChild( new TextNode("de") );
        doTest(root);
    }

    @Test public void testToxMark3() throws Exception
    {
        // BUG: #i107672# (non-deterministic; depends on pointer ordering)
        String name1 = mkName("toxmark");
        String name2 = mkName("toxmark");
        String name3 = mkName("toxmark");
        String name4 = mkName("toxmark");
        String name5 = mkName("toxmark");
        String name6 = mkName("toxmark");
        String name7 = mkName("toxmark");
        TreeNode root = new TreeNode();
        root.appendChild( new DocumentIndexMarkStartNode(name1) );
        root.appendChild( new DocumentIndexMarkStartNode(name2) );
        root.appendChild( new DocumentIndexMarkStartNode(name3) );
        root.appendChild( new DocumentIndexMarkStartNode(name4) );
        root.appendChild( new DocumentIndexMarkStartNode(name5) );
        root.appendChild( new DocumentIndexMarkStartNode(name6) );
        root.appendChild( new DocumentIndexMarkStartNode(name7) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new DocumentIndexMarkEndNode(name7) );
        root.appendChild( new DocumentIndexMarkEndNode(name6) );
        root.appendChild( new DocumentIndexMarkEndNode(name5) );
        root.appendChild( new DocumentIndexMarkEndNode(name4) );
        root.appendChild( new DocumentIndexMarkEndNode(name3) );
        root.appendChild( new DocumentIndexMarkEndNode(name2) );
        root.appendChild( new DocumentIndexMarkEndNode(name1) );
        root.appendChild( new TextNode("de") );
        doTest(root);
    }

    @Test public void testMarks1() throws Exception
    {
        String name1 = mkName("bookmark");
        String name2 = mkName("toxmark");
        String name3 = mkName("refmark");
        String name4 = mkName("toxmark");
        TreeNode root = new TreeNode();
        root.appendChild( new BookmarkStartNode(name1) );
        root.appendChild( new DocumentIndexMarkNode(name2) );
        root.appendChild( new ReferenceMarkStartNode(name3) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new BookmarkEndNode(name1) );
        root.appendChild( new DocumentIndexMarkStartNode(name4) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new DocumentIndexMarkEndNode(name4) );
        root.appendChild( new ReferenceMarkEndNode(name3) );
        doTest(root);
    }

    @Test public void testMarks2() throws Exception
    {
        String name1 = mkName("bookmark");
        String name2 = mkName("refmark");
        String name3 = mkName("refmark");
        String name4 = mkName("toxmark");
        String name5 = mkName("refmark");
        TreeNode root = new TreeNode();
        root.appendChild( new BookmarkStartNode(name1) );
        root.appendChild( new ReferenceMarkNode(name2) );
        root.appendChild( new ReferenceMarkStartNode(name3) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new DocumentIndexMarkStartNode(name4) );
        root.appendChild( new ReferenceMarkStartNode(name5) );
        // BUG: #i102541# (this is actually not unoportenum's fault)
        root.appendChild( new ReferenceMarkEndNode(name3) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new DocumentIndexMarkEndNode(name4) );
        root.appendChild( new BookmarkEndNode(name1) );
        root.appendChild( new ReferenceMarkEndNode(name5) );
        doTest(root);
    }

    @Test public void testMarks3() throws Exception
    {
        String name1 = mkName("bookmark");
        String name2 = mkName("refmark");
        String name3 = mkName("refmark");
        String name4 = mkName("toxmark");
        String name5 = mkName("refmark");
        TreeNode root = new TreeNode();
        root.appendChild( new BookmarkStartNode(name1) );
        root.appendChild( new DocumentIndexMarkNode(name2) );
        root.appendChild( new DocumentIndexMarkStartNode(name3) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new ReferenceMarkStartNode(name4) );
        root.appendChild( new DocumentIndexMarkStartNode(name5) );
        root.appendChild( new DocumentIndexMarkEndNode(name3) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new ReferenceMarkEndNode(name4) );
        root.appendChild( new BookmarkEndNode(name1) );
        root.appendChild( new DocumentIndexMarkEndNode(name5) );
        doTest(root);
    }

    @Test public void testFrameMark1() throws Exception
    {
        String name1 = mkName("bookmark");
        String name2 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new TextNode("abc") );
        root.appendChild( new BookmarkNode(name1) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new FrameNode(name2, AS_CHARACTER) );
        doTest(root);
    }

    @Test public void testFrameMark2() throws Exception
    {
        // BUG: #i98530#
        String name1 = mkName("bookmark");
        String name2 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new TextNode("abc") );
        root.appendChild( new BookmarkNode(name1) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new FrameNode(name2, AT_CHARACTER) );
        doTest(root);
    }

    @Test public void testFrameMark3() throws Exception
    {
        String name1 = mkName("frame");
        String name2 = mkName("bookmark");
        TreeNode root = new TreeNode();
        root.appendChild( new TextNode("abc") );
        root.appendChild( new FrameNode(name1, AS_CHARACTER) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new BookmarkNode(name2) );
        doTest(root);
    }

    @Test public void testFrameMark4() throws Exception
    {
        String name1 = mkName("frame");
        String name2 = mkName("bookmark");
        TreeNode root = new TreeNode();
        root.appendChild( new TextNode("abc") );
        root.appendChild( new FrameNode(name1, AT_CHARACTER) );
        root.appendChild( new TextNode("de") );
        root.appendChild( new BookmarkNode(name2) );
        doTest(root);
    }

    @Test public void testFrames1() throws Exception
    {
        String name1 = mkName("frame");
        String name2 = mkName("frame");
        String name3 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new FrameNode(name1, AT_CHARACTER) );
        root.appendChild( new FrameNode(name2, AT_CHARACTER) );
        root.appendChild( new FrameNode(name3, AT_CHARACTER) );
        doTest(root);
    }

    @Test public void testFrames2() throws Exception
    {
        String name1 = mkName("frame");
        String name2 = mkName("frame");
        String name3 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new FrameNode(name1, AS_CHARACTER) );
        root.appendChild( new FrameNode(name2, AS_CHARACTER) );
        root.appendChild( new FrameNode(name3, AS_CHARACTER) );
        doTest(root);
    }

    @Test public void testFrames3() throws Exception
    {
        String name1 = mkName("frame");
        String name2 = mkName("frame");
        String name3 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new FrameNode(name1, AT_CHARACTER) );
        root.appendChild( new FrameNode(name2, AS_CHARACTER) );
        root.appendChild( new FrameNode(name3, AT_CHARACTER) );
        doTest(root);
    }

    @Test public void testFrames4() throws Exception
    {
        String name1 = mkName("frame");
        String name2 = mkName("frame");
        String name3 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new FrameNode(name1, AT_CHARACTER) );
        root.appendChild( new FrameNode(name2, AT_CHARACTER) );
        root.appendChild( new FrameNode(name3, AS_CHARACTER) );
        doTest(root);
    }

    @Test public void testFrames5() throws Exception
    {
        String name1 = mkName("frame");
        String name2 = mkName("frame");
        String name3 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new FrameNode(name1, AS_CHARACTER) );
        root.appendChild( new FrameNode(name2, AT_CHARACTER) );
        root.appendChild( new FrameNode(name3, AT_CHARACTER) );
        doTest(root);
    }

    @Test public void testRubyHyperlink1() throws Exception
    {
        String name1 = mkName("ruby");
        String name2 = mkName("url");
        TreeNode root = new TreeNode();
        TreeNode ruby = new RubyNode(name1);
        TreeNode href = new HyperlinkNode(name2);
        href.appendChild( new TextNode("abc") );
        ruby.appendChild(href);
        root.appendChild(ruby);
        doTest(root);
    }

    @Test public void testRubyHyperlink2() throws Exception
    {
        String name1 = mkName("url");
        String name2 = mkName("ruby");
        TreeNode root = new TreeNode();
        TreeNode href = new HyperlinkNode(name1);
        TreeNode ruby = new RubyNode(name2);
        ruby.appendChild( new TextNode("abc") );
        href.appendChild(ruby);
        root.appendChild(href);
        doTest(root);
    }

    @Test public void testEnd1() throws Exception
    {
        String name1 = mkName("bookmark");
        String name2 = mkName("toxmark");
        String name3 = mkName("refmark");
        TreeNode root = new TreeNode();
        root.appendChild( new TextNode("abc") );
        root.appendChild( new BookmarkNode(name1) );
        root.appendChild( new DocumentIndexMarkNode(name2) );
        root.appendChild( new ReferenceMarkNode(name3) );
        doTest(root);
    }

    @Test public void testEnd2() throws Exception
    {
        String name1 = mkName("bookmark");
        String name2 = mkName("frame");
        String name3 = mkName("refmark");
        String name4 = mkName("frame");
        String name5 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new TextNode("abc") );
        root.appendChild( new BookmarkNode(name1) );
        root.appendChild( new FrameNode(name2, AT_CHARACTER) );
        root.appendChild( new ReferenceMarkNode(name3) );
        root.appendChild( new FrameNode(name4, AT_CHARACTER) );
        root.appendChild( new FrameNode(name5, AT_CHARACTER) );
        doTest(root);
    }

    @Test public void testEnd3() throws Exception
    {
        String name1 = mkName("ftn");
        String name2 = mkName("toxmark");
        TreeNode root = new TreeNode();
        root.appendChild( new TextNode("abc") );
        root.appendChild( new FootnoteNode(name1) );
        root.appendChild( new DocumentIndexMarkNode(name2) );
        doTest(root);
    }

    @Test public void testEnd4() throws Exception
    {
        String name1 = mkName("bookmark");
        String name2 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new BookmarkStartNode(name1) );
        root.appendChild( new TextNode("abc") );
        root.appendChild( new FrameNode(name2, AS_CHARACTER) );
        root.appendChild( new BookmarkEndNode(name1) );
        doTest(root);
    }

    @Test public void testEnd5() throws Exception
    {
        String name1 = mkName("refmark");
        String name2 = mkName("ruby");
        TreeNode root = new TreeNode();
        root.appendChild( new ReferenceMarkStartNode(name1) );
        root.appendChild( new TextNode("abc") );
        TreeNode ruby = new RubyNode(name2);
        ruby.appendChild( new TextFieldNode("de") );
        root.appendChild(ruby);
        root.appendChild( new ReferenceMarkEndNode(name1) );
        doTest(root);
    }

    @Test public void testEmpty1() throws Exception
    {
        String name1 = mkName("refmark");
        String name2 = mkName("toxmark");
        String name3 = mkName("bookmark");
        String name4 = mkName("frame");
        String name7 = mkName("refmark");
        String name8 = mkName("toxmark");
        String name9 = mkName("bookmark");
        String nameA = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new ReferenceMarkNode(name1) );
        root.appendChild( new DocumentIndexMarkNode(name2) );
        root.appendChild( new BookmarkNode(name3) );
        root.appendChild( new FrameNode(name4, AT_CHARACTER) );
        root.appendChild( new ReferenceMarkNode(name7) );
        root.appendChild( new DocumentIndexMarkNode(name8) );
        root.appendChild( new BookmarkNode(name9) );
        root.appendChild( new FrameNode(nameA, AT_CHARACTER) );
        doTest(root);
    }

    @Test public void testEmpty2() throws Exception
    {
        String name3 = mkName("bookmark");
        String name4 = mkName("frame");
        String name9 = mkName("bookmark");
        String nameA = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new BookmarkNode(name3) );
        root.appendChild( new FrameNode(name4, AT_CHARACTER) );
        root.appendChild( new BookmarkNode(name9) );
        root.appendChild( new FrameNode(nameA, AT_CHARACTER) );
        doTest(root);
    }

    @Test public void testEmpty3() throws Exception
    {
        String name1 = mkName("refmark");
        String name2 = mkName("toxmark");
        String name3 = mkName("bookmark");
        String name4 = mkName("frame");
        String name5 = mkName("url");
        String name6 = mkName("ruby");
        String name7 = mkName("refmark");
        String name8 = mkName("toxmark");
        String name9 = mkName("bookmark");
        String nameA = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new ReferenceMarkNode(name1) );
        root.appendChild( new DocumentIndexMarkNode(name2) );
        root.appendChild( new BookmarkNode(name3) );
        root.appendChild( new FrameNode(name4, AT_CHARACTER) );
        /* currently empty hyperlinks may get eaten...
        TreeNode href = new HyperlinkNode(name5);
        href.appendChild( new TextNode("") );
        root.appendChild(href);
        */
        TreeNode ruby = new RubyNode(name6);
        root.appendChild(ruby);
        root.appendChild( new ReferenceMarkNode(name7) );
        root.appendChild( new DocumentIndexMarkNode(name8) );
        root.appendChild( new BookmarkNode(name9) );
        root.appendChild( new FrameNode(nameA, AT_CHARACTER) );
        doTest(root);
    }

    @Test public void test1() throws Exception
    {
        String name1 = mkName("frame");
        String name2 = mkName("bookmark");
        String name3 = mkName("ruby");
        String name4 = mkName("ftn");
        String name5 = mkName("frame");
        TreeNode root = new TreeNode();
        root.appendChild( new FrameNode(name1, AT_CHARACTER) );
        root.appendChild( new BookmarkStartNode(name2) );
        root.appendChild( new TextNode("abc") );
        TreeNode ruby = new RubyNode(name3);
        ruby.appendChild( new TextNode("de") );
        ruby.appendChild( new FootnoteNode(name4) );
        ruby.appendChild( new BookmarkEndNode(name2) );
        root.appendChild(ruby);
        root.appendChild( new TextNode("fg") );
        root.appendChild( new FrameNode(name5, AT_CHARACTER) );
        root.appendChild( new TextFieldNode("h") );
        doTest(root);
    }

    /* some range tests for the insertion: these are for the current
       API which treats hyperlinks and rubys not as entities, but as formatting
       attributes; if these ever become entities, they should not be split!
     */

    @Test public void testRange1() throws Exception
    {
        String name1 = mkName("url");
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("12345");
        inserter.insertRange(new Range(0, 0, text));
        TreeNode url1 = new HyperlinkNode(name1);
        Range range1 = new Range(0, 5, url1);
        inserter.insertRange(range1);
        TreeNode root = new TreeNode();
        root.appendChild( url1 );
        url1.appendChild( text );
        doTest(root, false);
    }

    @Test public void testRangeHyperlinkHyperlink() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode url1 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(1, 4, url1) );
        // overlap left
        TreeNode url2 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(0, 2, url2) );
        TreeNode root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("12") ) )
            .appendChild( url1.dup().appendChild( new TextNode("34") ) )
            .appendChild( new TextNode("56789") );
        doTest(root, false);
        // overlap right
        TreeNode url3 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(3, 7, url3) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("12") ) )
            .appendChild( url1.dup().appendChild( new TextNode("3") ) )
            .appendChild( url3.dup().appendChild( new TextNode("4567") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
        // around
        TreeNode url4 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(3, 7, url4) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("12") ) )
            .appendChild( url1.dup().appendChild( new TextNode("3") ) )
            .appendChild( url4.dup().appendChild( new TextNode("4567") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
        // inside
        TreeNode url5 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(4, 6, url5) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("12") ) )
            .appendChild( url1.dup().appendChild( new TextNode("3") ) )
            .appendChild( url4.dup().appendChild( new TextNode("4") ) )
            .appendChild( url5.dup().appendChild( new TextNode("56") ) )
            .appendChild( url4.dup().appendChild( new TextNode("7") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
        // empty
        TreeNode url6 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(7, 7, url6) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("12") ) )
            .appendChild( url1.dup().appendChild( new TextNode("3") ) )
            .appendChild( url4.dup().appendChild( new TextNode("4") ) )
            .appendChild( url5.dup().appendChild( new TextNode("56") ) )
            .appendChild( url4.dup().appendChild( new TextNode("7") ) )
// this one gets eaten, but we still need to test inserting it (#i106930#)
//            .appendChild( url6.dup().appendChild( new TextNode("") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
    }

    @Test public void testRangeHyperlinkRuby() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode url1 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(1, 4, url1) );
        // overlap left
        TreeNode rby2 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(0, 2, rby2) );
        TreeNode root = new TreeNode()
            .appendChild( rby2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( url1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( url1.dup().appendChild( new TextNode("34") ) )
            .appendChild( new TextNode("56789") );
        doTest(root, false);
        // overlap right
        TreeNode rby3 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(3, 5, rby3) );
        root = new TreeNode()
            .appendChild( rby2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( url1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( url1.dup().appendChild( new TextNode("3") ) )
            .appendChild( rby3.dup()
                .appendChild( url1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // around
        TreeNode rby4 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(2, 3, rby4) );
        root = new TreeNode()
            .appendChild( rby2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( url1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( rby4.dup()
                .appendChild( url1.dup().appendChild( new TextNode("3") ) ) )
            .appendChild( rby3.dup()
                .appendChild( url1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // inside
        TreeNode url5 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(6, 9, url5) );
        TreeNode rby6 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(7, 8, rby6) );
        root = new TreeNode()
            .appendChild( rby2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( url1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( rby4.dup()
                .appendChild( url1.dup().appendChild( new TextNode("3") ) ) )
            .appendChild( rby3.dup()
                .appendChild( url1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6") )
            .appendChild( url5.dup().appendChild( new TextNode("7") ) )
            .appendChild( rby6.dup()
                .appendChild( url5.dup().appendChild( new TextNode("8") ) ) )
            .appendChild( url5.dup().appendChild( new TextNode("9") ) );
        doTest(root, false);
    }

    @Test public void testRangeRubyHyperlink() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode rby1 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(1, 6, rby1) );
        // overlap left
        TreeNode url2 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(0, 3, url2) );
        TreeNode root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("1") ) )
            .appendChild( rby1.dup()
                .appendChild( url2.dup().appendChild( new TextNode("23") ) )
                .appendChild( new TextNode("456") ) )
            .appendChild( new TextNode("789") );
        doTest(root, false);
        // overlap right
        TreeNode url3 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(5, 7, url3) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("1") ) )
            .appendChild( rby1.dup()
                .appendChild( url2.dup().appendChild( new TextNode("23") ) )
                .appendChild( new TextNode("45") )
                .appendChild( url3.dup().appendChild( new TextNode("6") ) ) )
            .appendChild( url3.dup().appendChild( new TextNode("7") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
        // around (not quite, due to API)
        TreeNode url4 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(1, 8, url4) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("1") ) )
            .appendChild( rby1.dup()
                .appendChild( url4.dup()
                    .appendChild( new TextNode("23456") ) ) )
            .appendChild( url4.dup().appendChild( new TextNode("78") ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
        // inside
        TreeNode url5 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(3, 5, url5) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("1") ) )
            .appendChild( rby1.dup()
                .appendChild( url4.dup()
                    .appendChild( new TextNode("23") ) )
                .appendChild( url5.dup()
                    .appendChild( new TextNode("45") ) )
                .appendChild( url4.dup()
                    .appendChild( new TextNode("6") ) ) )
            .appendChild( url4.dup().appendChild( new TextNode("78") ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
    }

    @Test public void testRangeRubyRuby() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode rby1 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(1, 4, rby1) );
        // overlap left
        TreeNode rby2 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(0, 2, rby2) );
        TreeNode root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("12") ) )
            .appendChild( rby1.dup().appendChild( new TextNode("34") ) )
            .appendChild( new TextNode("56789") );
        doTest(root, false);
        // overlap right
        TreeNode rby3 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(3, 7, rby3) );
        root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("12") ) )
            .appendChild( rby1.dup().appendChild( new TextNode("3") ) )
            .appendChild( rby3.dup().appendChild( new TextNode("4567") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
        // around
        TreeNode rby4 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(3, 7, rby4) );
        root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("12") ) )
            .appendChild( rby1.dup().appendChild( new TextNode("3") ) )
            .appendChild( rby4.dup().appendChild( new TextNode("4567") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
        // inside
        TreeNode rby5 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(4, 6, rby5) );
        root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("12") ) )
            .appendChild( rby1.dup().appendChild( new TextNode("3") ) )
            .appendChild( rby4.dup().appendChild( new TextNode("4") ) )
            .appendChild( rby5.dup().appendChild( new TextNode("56") ) )
            .appendChild( rby4.dup().appendChild( new TextNode("7") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
    }

    @Test public void testRangeHyperlinkMeta() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode url1 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(1, 4, url1) );
        // overlap left
        TreeNode met2 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(0, 2, met2) );
        TreeNode root = new TreeNode()
            .appendChild( met2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( url1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( url1.dup().appendChild( new TextNode("34") ) )
            .appendChild( new TextNode("56789") );
        doTest(root, false);
        // overlap right
        TreeNode met3 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(4/*-1*/, 6/*-1*/, met3) );
        root = new TreeNode()
            .appendChild( met2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( url1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( url1.dup().appendChild( new TextNode("3") ) )
            .appendChild( met3.dup()
                .appendChild( url1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // around
        TreeNode met4 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(3/*-1*/, 4/*-1*/, met4) );
        root = new TreeNode()
            .appendChild( met2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( url1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( met4.dup()
                .appendChild( url1.dup().appendChild( new TextNode("3") ) ) )
            .appendChild( met3.dup()
                .appendChild( url1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // inside
        TreeNode url5 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(9/*-3*/, 12/*-3*/, url5) );
        TreeNode met6 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(10/*-3*/, 11/*-3*/, met6) );
        root = new TreeNode()
            .appendChild( met2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( url1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( met4.dup()
                .appendChild( url1.dup().appendChild( new TextNode("3") ) ) )
            .appendChild( met3.dup()
                .appendChild( url1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6") )
            .appendChild( url5.dup().appendChild( new TextNode("7") ) )
            .appendChild( met6.dup()
                .appendChild( url5.dup().appendChild( new TextNode("8") ) ) )
            .appendChild( url5.dup().appendChild( new TextNode("9") ) );
        doTest(root, false);
    }

    @Test public void testRangeRubyMeta() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode rby1 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(1, 4, rby1) );
        // overlap left
        TreeNode met2 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(0, 2, met2) );
        TreeNode root = new TreeNode()
            .appendChild( met2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( rby1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( rby1.dup().appendChild( new TextNode("34") ) )
            .appendChild( new TextNode("56789") );
        doTest(root, false);
        // overlap right
        TreeNode met3 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(4/*-1*/, 6/*-1*/, met3) );
        root = new TreeNode()
            .appendChild( met2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( rby1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( rby1.dup().appendChild( new TextNode("3") ) )
            .appendChild( met3.dup()
                .appendChild( rby1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // around
        TreeNode met4 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(3/*-1*/, 4/*-1*/, met4) );
        root = new TreeNode()
            .appendChild( met2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( rby1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( met4.dup()
                .appendChild( rby1.dup().appendChild( new TextNode("3") ) ) )
            .appendChild( met3.dup()
                .appendChild( rby1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // inside
        TreeNode rby5 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(9/*-3*/, 12/*-3*/, rby5) );
        TreeNode met6 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(10/*-3*/, 11/*-3*/, met6) );
        root = new TreeNode()
            .appendChild( met2.dup()
                .appendChild( new TextNode("1") )
                .appendChild( rby1.dup().appendChild( new TextNode("2") ) ) )
            .appendChild( met4.dup()
                .appendChild( rby1.dup().appendChild( new TextNode("3") ) ) )
            .appendChild( met3.dup()
                .appendChild( rby1.dup().appendChild( new TextNode("4") ) )
                .appendChild( new TextNode("5") ) )
            .appendChild( new TextNode("6") )
            .appendChild( rby5.dup()
                .appendChild( new TextNode("7") )
                .appendChild( met6.dup()
                    .appendChild( new TextNode("8") ) )
                .appendChild( new TextNode("9") ) );
        doTest(root, false);
    }

    @Test public void testRangeMetaHyperlink() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode met1 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(1, 6, met1) );
        // overlap left
        TreeNode url2 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(0, 4/*-1*/, url2) );
        TreeNode root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("1") ) )
            .appendChild( met1.dup()
                .appendChild( url2.dup().appendChild( new TextNode("23") ) )
                .appendChild( new TextNode("456") ) )
            .appendChild( new TextNode("789") );
        doTest(root, false);
        // overlap right
        TreeNode url3 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(6/*-1*/, 8/*-1*/, url3) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("1") ) )
            .appendChild( met1.dup()
                .appendChild( url2.dup().appendChild( new TextNode("23") ) )
                .appendChild( new TextNode("45") )
                .appendChild( url3.dup().appendChild( new TextNode("6") ) ) )
            .appendChild( url3.dup().appendChild( new TextNode("7") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
        // around (not quite, due to API)
        TreeNode url4 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(1, 9/*-1*/, url4) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("1") ) )
            .appendChild( met1.dup()
                .appendChild( url4.dup()
                    .appendChild( new TextNode("23456") ) ) )
            .appendChild( url4.dup().appendChild( new TextNode("78") ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
        // inside
        TreeNode url5 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(4/*-1*/, 6/*-1*/, url5) );
        root = new TreeNode()
            .appendChild( url2.dup().appendChild( new TextNode("1") ) )
            .appendChild( met1.dup()
                .appendChild( url4.dup()
                    .appendChild( new TextNode("23") ) )
                .appendChild( url5.dup()
                    .appendChild( new TextNode("45") ) )
                .appendChild( url4.dup()
                    .appendChild( new TextNode("6") ) ) )
            .appendChild( url4.dup().appendChild( new TextNode("78") ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
    }

    @Test public void testRangeMetaRuby() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode met1 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(1, 5, met1) );
        // overlap left
        TreeNode rby2 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(0, 3/*-1*/, rby2) );
        TreeNode root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("1") ) )
            .appendChild( met1.dup()
                .appendChild( rby2.dup().appendChild( new TextNode("2") ) )
                .appendChild( new TextNode("345") ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // overlap right
        TreeNode rby3 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(5/*-1*/, 7/*-1*/, rby3) );
        root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("1") ) )
            .appendChild( met1.dup()
                .appendChild( rby2.dup().appendChild( new TextNode("2") ) )
                .appendChild( new TextNode("34") )
                .appendChild( rby3.dup().appendChild( new TextNode("5") ) ) )
            .appendChild( rby3.dup().appendChild( new TextNode("6") ) )
            .appendChild( new TextNode("789") );
        doTest(root, false);
        // around
        TreeNode rby4 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(1, 7/*-1*/, rby4) );
        root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("1") ) )
            .appendChild( rby4.dup()
                .appendChild( met1.dup()
                    .appendChild( new TextNode("2345") ) )
                .appendChild( new TextNode("6") ) )
            .appendChild( new TextNode("789") );
        doTest(root, false);
        // inside
        TreeNode met5 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(7/*-1*/, 9/*-1*/, met5) );
        TreeNode rby6 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(9/*-2*/, 10/*-2*/, rby6) );
        root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("1") ) )
            .appendChild( rby4.dup()
                .appendChild( met1.dup()
                    .appendChild( new TextNode("2345") ) )
                .appendChild( new TextNode("6") ) )
            .appendChild( met5.dup()
                .appendChild( new TextNode("7") )
                .appendChild( rby6.dup()
                    .appendChild( new TextNode("8") ) ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
        // inside, with invalid range that includes the dummy char
        TreeNode rby7 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(7/*-1*/, 9/*-2*/, rby7) );
        root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("1") ) )
            .appendChild( rby4.dup()
                .appendChild( met1.dup()
                    .appendChild( new TextNode("2345") ) )
                .appendChild( new TextNode("6") ) )
            .appendChild( met5.dup()
                .appendChild( rby7.dup()
                    .appendChild( new TextNode("7") ) )
                .appendChild( rby6.dup()
                    .appendChild( new TextNode("8") ) ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
        // around, at same position as meta
        TreeNode rby8 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(7/*-1*/, 10/*-2*/, rby8) );
        root = new TreeNode()
            .appendChild( rby2.dup().appendChild( new TextNode("1") ) )
            .appendChild( rby4.dup()
                .appendChild( met1.dup()
                    .appendChild( new TextNode("2345") ) )
                .appendChild( new TextNode("6") ) )
            .appendChild( rby8.dup()
                .appendChild( met5.dup()
                    .appendChild( new TextNode("78") ) ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
    }

    @Test public void testRangeMetaMeta() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode met1 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(3, 6, met1) );
        // overlap left
        TreeNode met2 = new MetaNode( mkId("id") );
        try {
            inserter.insertRange( new Range(0, 4, met2) );
            fail("testRangeMetaMeta: overlap left allowed");
        } catch (com.sun.star.lang.IllegalArgumentException e) { /* ignore */ }
        TreeNode root = new TreeNode()
            .appendChild( new TextNode("123") )
            .appendChild( met1.dup().appendChild( new TextNode("456") ) )
            .appendChild( new TextNode("789") );
        doTest(root, false);
        // overlap right
        TreeNode met3 = new MetaNode( mkId("id") );
        try {
            inserter.insertRange( new Range(5/*-1*/, 8/*-1*/, met3) );
            fail("testRangeMetaMeta: overlap right allowed");
        } catch (com.sun.star.lang.IllegalArgumentException e) { /* ignore */ }
        root = new TreeNode()
            .appendChild( new TextNode("123") )
            .appendChild( met1.dup().appendChild( new TextNode("456") ) )
            .appendChild( new TextNode("789") );
        doTest(root, false);
        // around
        TreeNode met4 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(3, 7/*-1*/, met4) );
        root = new TreeNode()
            .appendChild( new TextNode("123") )
            .appendChild( met4.dup()
                .appendChild( met1.dup().appendChild( new TextNode("456") ) ) )
            .appendChild( new TextNode("789") );
        doTest(root, false);
        // inside
        TreeNode met5 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(6/*-2*/, 8/*-2*/, met5) );
        root = new TreeNode()
            .appendChild( new TextNode("123") )
            .appendChild( met4.dup()
                .appendChild( met1.dup()
                    .appendChild( new TextNode("4") )
                    .appendChild( met5.dup()
                        .appendChild( new TextNode("56") ) ) ) )
            .appendChild( new TextNode("789") );
        doTest(root, false);
    }

    @Test public void testRange2() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode met1 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(1, 8, met1) );
        TreeNode met2 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(3/*-1*/, 8/*-1*/, met2) );
        TreeNode met3 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(5/*-2*/, 8/*-2*/, met3) );
        TreeNode root = new TreeNode()
            .appendChild( new TextNode("1") )
            .appendChild( met1.dup()
                .appendChild( new TextNode("2") )
                .appendChild( met2.dup()
                    .appendChild( new TextNode("3") )
                    .appendChild( met3.dup()
                        .appendChild( new TextNode("456") ) )
                    .appendChild( new TextNode("7") ) )
                .appendChild( new TextNode("8") ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
        // split ruby at every meta start!
        TreeNode rby4 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(0, 7/*-3*/, rby4) );
        root = new TreeNode()
            .appendChild( rby4.dup()
                .appendChild( new TextNode("1") ) )
            .appendChild( met1.dup()
                .appendChild( rby4.dup()
                    .appendChild( new TextNode("2") ) )
                .appendChild( met2.dup()
                    .appendChild( rby4.dup()
                        .appendChild( new TextNode("3") ) )
                    .appendChild( met3.dup()
                        .appendChild( rby4.dup()
                            .appendChild( new TextNode("4") ) )
                        .appendChild( new TextNode("56") ) )
                    .appendChild( new TextNode("7") ) )
                .appendChild( new TextNode("8") ) )
            .appendChild( new TextNode("9") );
        doTest(root, false);
        // split ruby at every meta end!
        TreeNode rby5 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(8/*-3*/, 12/*-3*/, rby5) );
        root = new TreeNode()
            .appendChild( rby4.dup()
                .appendChild( new TextNode("1") ) )
            .appendChild( met1.dup()
                .appendChild( rby4.dup()
                    .appendChild( new TextNode("2") ) )
                .appendChild( met2.dup()
                    .appendChild( rby4.dup()
                        .appendChild( new TextNode("3") ) )
                    .appendChild( met3.dup()
                        .appendChild( rby4.dup()
                            .appendChild( new TextNode("4") ) )
                        .appendChild( new TextNode("5") )
                        .appendChild( rby5.dup()
                            .appendChild( new TextNode("6") ) ) )
                    .appendChild( rby5.dup()
                        .appendChild( new TextNode("7") ) ) )
                .appendChild( rby5.dup()
                    .appendChild( new TextNode("8") ) ) )
            .appendChild( rby5.dup()
                .appendChild( new TextNode("9") ) );
        doTest(root, false);
    }

    @Test public void testRange3() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode rby1 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(0, 9, rby1) );
        TreeNode met2 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(2, 7, met2) );
        TreeNode root = new TreeNode()
            .appendChild( rby1.dup()
                .appendChild( new TextNode("12") )
                .appendChild( met2.dup()
                    .appendChild( new TextNode("34567") ) )
                .appendChild( new TextNode("89") ) );
        doTest(root, false);
        // overwrite outer ruby, split remains at inner meta!
        TreeNode rby3 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(5/*-1*/, 6/*-1*/, rby3) );
        root = new TreeNode()
            .appendChild( rby1.dup()
                .appendChild( new TextNode("12") ) )
            .appendChild( met2.dup()
                .appendChild( rby1.dup()
                    .appendChild( new TextNode("34") ) )
                .appendChild( rby3.dup()
                    .appendChild( new TextNode("5") ) )
                .appendChild( rby1.dup()
                    .appendChild( new TextNode("67") ) ) )
            .appendChild( rby1.dup()
                .appendChild( new TextNode("89") ) );
        doTest(root, false);
    }

    @Test public void testRange4() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode rby1 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(0, 9, rby1) );
        TreeNode met2 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(1, 8, met2) );
        TreeNode met3 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(3/*-1*/, 8/*-1*/, met3) );
        TreeNode met4 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(5/*-2*/, 8/*-2*/, met4) );
        TreeNode root = new TreeNode()
            .appendChild( rby1.dup()
                .appendChild( new TextNode("1") )
                .appendChild( met2.dup()
                    .appendChild( new TextNode("2") )
                    .appendChild( met3.dup()
                        .appendChild( new TextNode("3") )
                        .appendChild( met4.dup()
                            .appendChild( new TextNode("456") ) )
                        .appendChild( new TextNode("7") ) )
                    .appendChild( new TextNode("8") ) )
                .appendChild( new TextNode("9") ) );
        doTest(root, false);
        // overwrite outer ruby, split remains at every inner meta!
        TreeNode rby5 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(7/*-3*/, 8/*-3*/, rby5) );
        root = new TreeNode()
            .appendChild( rby1.dup()
                .appendChild( new TextNode("1") ) )
            .appendChild( met2.dup()
                .appendChild( rby1.dup()
                    .appendChild( new TextNode("2") ) )
                .appendChild( met3.dup()
                    .appendChild( rby1.dup()
                        .appendChild( new TextNode("3") ) )
                    .appendChild( met4.dup()
                        .appendChild( rby1.dup()
                            .appendChild( new TextNode("4") ) )
                        .appendChild( rby5.dup()
                            .appendChild( new TextNode("5") ) )
                        .appendChild( rby1.dup()
                            .appendChild( new TextNode("6") ) ) )
                    .appendChild( rby1.dup()
                        .appendChild( new TextNode("7") ) ) )
                .appendChild( rby1.dup()
                    .appendChild( new TextNode("8") ) ) )
            .appendChild( rby1.dup()
                .appendChild( new TextNode("9") ) );
        doTest(root, false);
    }

    @Test public void testRange5() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode rby1 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(0, 9, rby1) );
        TreeNode met2 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(1, 3, met2) );
        TreeNode met3 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(5/*-1*/, 6/*-1*/, met3) );
        TreeNode met4 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(8/*-2*/, 10/*-2*/, met4) );
        TreeNode root = new TreeNode()
            .appendChild( rby1.dup()
                .appendChild( new TextNode("1") )
                .appendChild( met2.dup().appendChild( new TextNode("23") ) )
                .appendChild( new TextNode("4") )
                .appendChild( met3.dup().appendChild( new TextNode("5") ) )
                .appendChild( new TextNode("6") )
                .appendChild( met4.dup().appendChild( new TextNode("78") ) )
                .appendChild( new TextNode("9") ) );
        doTest(root, false);
        // overwrite outer ruby, but split at inner metas!
        TreeNode rby5 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(3/*-1*/, 10/*-3*/, rby5) );
        root = new TreeNode()
            .appendChild( rby1.dup()
                .appendChild( new TextNode("1") ) )
            .appendChild( met2.dup()
                .appendChild( rby1.dup()
                    .appendChild( new TextNode("2") ) )
                .appendChild( rby5.dup()
                    .appendChild( new TextNode("3") ) ) )
            .appendChild( rby5.dup()
                .appendChild( new TextNode("4") )
                .appendChild( met3.dup()
                    .appendChild( new TextNode("5") ) )
                .appendChild( new TextNode("6") ) )
            .appendChild( met4.dup()
                .appendChild( rby5.dup()
                    .appendChild( new TextNode("7") ) )
                .appendChild( rby1.dup()
                    .appendChild( new TextNode("8") ) ) )
            .appendChild( rby1.dup()
                .appendChild( new TextNode("9") ) );
        doTest(root, false);
    }

    @Test public void testRange6() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode met1 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(1, 5, met1) );
        TreeNode met2 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(3/*-1*/, 6/*-1*/, met2) );
        TreeNode met3 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(5/*-2*/, 7/*-2*/, met3) );
        TreeNode root = new TreeNode()
            .appendChild( new TextNode("1") )
            .appendChild( met1.dup()
                .appendChild( new TextNode("2") )
                .appendChild( met2.dup()
                    .appendChild( new TextNode("3") )
                    .appendChild( met3.dup()
                        .appendChild( new TextNode("45") ) ) ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // split at 3 metas, all at same position
        TreeNode rby4 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(7/*-3*/, 10/*-3*/, rby4) );
        root = new TreeNode()
            .appendChild( new TextNode("1") )
            .appendChild( met1.dup()
                .appendChild( new TextNode("2") )
                .appendChild( met2.dup()
                    .appendChild( new TextNode("3") )
                    .appendChild( met3.dup()
                        .appendChild( new TextNode("4") )
                        .appendChild( rby4.dup()
                            .appendChild( new TextNode("5") ) ) ) ) )
            .appendChild( rby4.dup()
                .appendChild( new TextNode("67") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
    }

    @Test public void testRange7() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        TreeNode url1 = new HyperlinkNode( mkName("url") );
        inserter.insertRange( new Range(1, 5, url1) );
        TreeNode met2 = new MetaNode( mkId("id") );
        inserter.insertRange( new Range(3, 5, met2) );
        TreeNode root = new TreeNode()
            .appendChild( new TextNode("1") )
            .appendChild( url1.dup()
                .appendChild( new TextNode("23") ) )
            .appendChild( met2.dup()
                .appendChild( url1.dup()
                    .appendChild( new TextNode("45") ) ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        // this should result in not splitting the hyperlink, but due to API
        // we can't tell :(
        TreeNode rby3 = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(5/*-1*/, 8/*-1*/, rby3) );
        root = new TreeNode()
            .appendChild( new TextNode("1") )
            .appendChild( url1.dup()
                .appendChild( new TextNode("23") ) )
            .appendChild( met2.dup()
                .appendChild( url1.dup()
                    .appendChild( new TextNode("4") ) )
                .appendChild( rby3.dup()
                    .appendChild( url1.dup()
                        .appendChild( new TextNode("5") ) ) ) )
            .appendChild( rby3.dup()
                .appendChild( new TextNode("67") ) )
            .appendChild( new TextNode("89") );
        doTest(root, false);
    }

    /* TODO: test partial selection, test UNDO/REDO */

    // #i109601# NestedTextContent and XChild
    @Test public void testMetaXChild() throws Exception
    {
        StringPair id1 = new StringPair("content.xml", mkName("id"));
        StringPair id2 = new StringPair("content.xml", mkName("id"));
        StringPair id3 = new StringPair("content.xml", mkName("id"));
        StringPair id4 = new StringPair("content.xml", mkName("id"));
        StringPair id5 = new StringPair("content.xml", mkName("id"));
        StringPair id6 = new StringPair("content.xml", mkName("id"));
        TreeNode meta1 = new MetaNode(id1);
        TreeNode meta2 = new MetaNode(id2);
        TreeNode meta3 = new MetaFieldNode(id3);
        TreeNode meta4 = new MetaNode(id4);
        TreeNode meta5 = new MetaNode(id5);
        TreeNode meta6 = new MetaFieldNode(id6);
        TreeNode root = new TreeNode()
            .appendChild( meta1.dup()
                .appendChild( new TextNode("1") ) )
            .appendChild( new TextNode("2") )
            .appendChild( meta2.dup()
                .appendChild( meta3.dup()
                    .appendChild( new TextNode("34") )
                    .appendChild( meta4.dup()
                        .appendChild( new TextNode("56") ) )
                    .appendChild( meta5.dup() )
                    .appendChild( new TextNode("7") ) ) )
            .appendChild( new TextNode("8") )
            .appendChild( meta6.dup()
                .appendChild( new TextNode("9") ) );

        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123456789");
        inserter.insertRange( new Range(0, 0, text) );
        XTextContent xMeta1 = inserter.insertRange( new Range(0, 1, meta1) );
        XTextContent xMeta2 = inserter.insertRange( new Range(3, 8, meta2) );
        XTextContent xMeta3 = inserter.insertRange( new Range(4, 9, meta3) );
        XTextContent xMeta4 = inserter.insertRange( new Range(7, 9, meta4) );
        XTextContent xMeta5 = inserter.insertRange( new Range(10, 10, meta5) );
        XTextContent xMeta6 = inserter.insertRange( new Range(13, 14, meta6) );

        doTest(root, false);

        XText xDocText = m_xDoc.getText();
        XTextCursor xDocTextCursor = xDocText.createTextCursor();
        XParagraphCursor xParagraphCursor = UnoRuntime.queryInterface(XParagraphCursor.class, xDocTextCursor);
        xParagraphCursor.gotoNextParagraph(false); // second paragraph
        // X12XX34X56X78X9
        // 1  23  4  5  6
        //  1       452  6
        //            3
        StringPair [] nestedTextContent = new StringPair[] {
            null,
            id1,
            id1,
            null,
            id2,
            id3,
            id3,
            id3,
            id4,
            id4,
            id4,
            id5,
            id3,
            null,
            id6,
            id6,
        };
        XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xDocTextCursor);
        for (int i = 0; i < nestedTextContent.length; ++i) {
            Object oNTC = xPropertySet.getPropertyValue("NestedTextContent");
            XTextContent xNTC = UnoRuntime.queryInterface(XTextContent.class, oNTC);
            if (null == nestedTextContent[i]) {
                assertNull("unexpected NestedTextContent at: " + i, xNTC);
            } else {
                XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xNTC);
                StringPair xmlid = xMetadatable.getMetadataReference();
                assertTrue("wrong NestedTextContent at: " + i,
                    MetaNode.eq(nestedTextContent[i], xmlid));
            }
            xDocTextCursor.goRight((short)1, false);
        }

        XChild xChild1 = UnoRuntime.queryInterface(XChild.class, xMeta1);
        XChild xChild2 = UnoRuntime.queryInterface(XChild.class, xMeta2);
        XChild xChild3 = UnoRuntime.queryInterface(XChild.class, xMeta3);
        XChild xChild4 = UnoRuntime.queryInterface(XChild.class, xMeta4);
        XChild xChild5 = UnoRuntime.queryInterface(XChild.class, xMeta5);
        XChild xChild6 = UnoRuntime.queryInterface(XChild.class, xMeta6);
        try {
            xChild1.setParent(xChild4);
            fail("setParent(): allowed?");
        } catch (NoSupportException e) { /* expected */ }
        assertNull("getParent(): not null", xChild1.getParent());
        assertNull("getParent(): not null", xChild2.getParent());
        assertNull("getParent(): not null", xChild6.getParent());
        {
            Object xParent3 = xChild3.getParent();
            assertNotNull("getParent(): null", xParent3);
            XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xParent3);
            StringPair xmlid = xMetadatable.getMetadataReference();
            assertTrue("getParent(): wrong", MetaNode.eq(xmlid, id2));
        }{
            Object xParent4 = xChild4.getParent();
            assertNotNull("getParent(): null", xParent4);
            XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xParent4);
            StringPair xmlid = xMetadatable.getMetadataReference();
            assertTrue("getParent(): wrong", MetaNode.eq(xmlid, id3));
        }{
            Object xParent5 = xChild5.getParent();
            assertNotNull("getParent(): null", xParent5);
            XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xParent5);
            StringPair xmlid = xMetadatable.getMetadataReference();
            assertTrue("getParent(): wrong", MetaNode.eq(xmlid, id3));
        }
    }

    /** test SwXMeta XText interface */
    @Test public void testMetaXText() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("12AB6789");
        inserter.insertRange( new Range(0, 0, text) );
        MetaNode meta = new MetaNode( mkId("id") );
//        inserter.insertRange( new Range(3, 5, met2) );
        XTextContent xMeta = inserter.makeMeta();

        XText xDocText = m_xDoc.getText();
        XTextCursor xDocTextCursor = xDocText.createTextCursor();
        xDocTextCursor.goRight((short)3, false);
        xDocTextCursor.goRight((short)2, true);
        xDocText.insertTextContent(xDocTextCursor, xMeta, true);
//        xMeta.attach(xDocTextCursor);

        XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xMeta);
        xMetadatable.setMetadataReference(meta.getXmlId());
        XText xText = UnoRuntime.queryInterface(XText.class, xMeta);

        XText xParentText = xText.getText();
        assertNotNull("getText(): no parent", xParentText);

        XTextRange xStart = xText.getStart();
        assertNotNull("getStart(): no start", xStart);

        XTextRange xEnd = xText.getEnd();
        assertNotNull("getEnd(): no end", xEnd);

        xText.setString("45");

        {
            String string = xText.getString();
            assertEquals("getString(): invalid string returned",
                         "45", string);
        }

        XTextCursor xTextCursor = xText.createTextCursor();
        assertNotNull("createTextCursor(): failed", xTextCursor);

        try {
            xText.createTextCursorByRange(null);
            fail("createTextCursorByRange(): null allowed?");
        } catch (RuntimeException e) { /* expected */ }

        XTextCursor xTextCursorStart = xText.createTextCursorByRange(xStart);
        assertNotNull("createTextCursorByRange(): failed for start",
                      xTextCursorStart);

        XTextCursor xTextCursorEnd = xText.createTextCursorByRange(xEnd);
        assertNotNull("createTextCursorByRange(): failed for end",
                      xTextCursorEnd);

        // move outside meta
        xDocTextCursor.gotoStart(false);

        try {
            xText.insertString(null, "foo", false);
            fail("insertString(): null allowed?");
        } catch (RuntimeException e) { /* expected */ }

        try {
            xText.insertString(xDocTextCursor, "foo", false);
            fail("insertString(): cursor outside allowed?");
        } catch (RuntimeException e) { /* expected */ }

        xStart = xText.getStart();
        xText.insertString(xStart, "A", false);
        {
            String string = xText.getString();
            assertEquals("getString(): invalid string returned",
                         "A45", string);
        }

        xText.insertString(xEnd, "B", false);
        {
            String string = xText.getString();
            assertEquals("getString(): invalid string returned",
                         "A45B", string);
        }

        try {
            xText.insertControlCharacter(null, HARD_HYPHEN, false);
            fail("insertControlCharacter(): null allowed?");
        } catch (com.sun.star.lang.IllegalArgumentException e) { /* ignore */ }

        xStart = xText.getStart();
        try {
            xText.insertControlCharacter(xDocTextCursor, HARD_HYPHEN, false);
            fail("insertControlCharacter(): cursor outside allowed?");
        } catch (com.sun.star.lang.IllegalArgumentException e) { /* ignore */ }

        xText.insertControlCharacter(xStart, HARD_HYPHEN, false);
        {
            String string = xText.getString();
            assertEquals("getString(): invalid string returned",
                         '\u2011' + "A45B", string);
        }

        xText.insertControlCharacter(xEnd, HARD_HYPHEN, false);
        {
            String string = xText.getString();
            assertEquals("getString(): invalid string returned",
                         '\u2011' + "A45B" + '\u2011', string);
        }

        xText.setString("45");

        try {
            xText.insertTextContent(null, xMeta, false);
            fail("insertTextContent(): null range allowed?");
        } catch (com.sun.star.lang.IllegalArgumentException e) { /* ignore */ }

        try {
            xText.insertTextContent(xStart, null, false);
            fail("insertTextContent(): null content allowed?");
        } catch (com.sun.star.lang.IllegalArgumentException e) { /* ignore */ }

        try {
            xText.insertTextContent(xDocTextCursor, xMeta, false);
            fail("insertTextContent(): cursor outside allowed?");
        } catch (com.sun.star.lang.IllegalArgumentException e) { /* ignore */ }

        TextFieldNode field1 = new TextFieldNode( "f1" );
        TextFieldNode field2 = new TextFieldNode( "f2" );
        XTextContent xField1 = inserter.makeTextField(field1.getContent());
        XTextContent xField2 = inserter.makeTextField(field2.getContent());

        xStart = xText.getStart();
        xText.insertTextContent(xStart, xField1, false);

        TreeNode root = new TreeNode()
            .appendChild( new TextNode("12") )
            .appendChild( meta.dup()
                .appendChild( field1.dup() )
                .appendChild( new TextNode("45") ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);

        xText.insertTextContent(xEnd, xField2, false);

        root = new TreeNode()
            .appendChild( new TextNode("12") )
            .appendChild( meta.dup()
                .appendChild( field1.dup() )
                .appendChild( new TextNode("45") )
                .appendChild( field2.dup() ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);

        try {
            xText.removeTextContent(null);
            fail("removeTextContent(): null content allowed?");
        } catch (RuntimeException e) { /* expected */ }

        xText.removeTextContent(xField1);

        XTextRange xAnchor = xMeta.getAnchor();
        assertNotNull("getAnchor(): null", xAnchor);

        // evil test case: insert ruby around meta
        RubyNode ruby = new RubyNode( mkName("ruby") );
        inserter.insertRange( new Range(2, 6, ruby) );

        /* prevent caching...
        root = new TreeNode()
            .appendChild( new TextNode("12") )
            .appendChild( ruby.dup()
                .appendChild( meta.dup()
                    .appendChild( new TextNode("45") )
                    .appendChild( field2.dup() ) ) )
            .appendChild( new TextNode("6789") );
        doTest(root, false);
        */

        XEnumerationAccess xEA = UnoRuntime.queryInterface(XEnumerationAccess.class, xMeta);
        XEnumeration xEnum = xEA.createEnumeration();
        assertNotNull("createEnumeration(): returns null", xEnum);
        {
            assertTrue("hasNext(): first missing", xEnum.hasMoreElements());
            Object xElement = xEnum.nextElement();
            XTextRange xPortion = UnoRuntime.queryInterface(XTextRange.class, xElement);
            XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xPortion);
            String type = (String) xPropSet.getPropertyValue("TextPortionType");
            assertEquals("first: not text", "Text", type);
            String txt = xPortion.getString();
            assertEquals("first: text differs", "45", txt);
        }
        {
            assertTrue("hasNext(): second missing", xEnum.hasMoreElements());
            Object xElement = xEnum.nextElement();
            XTextRange xPortion = UnoRuntime.queryInterface(XTextRange.class, xElement);
            XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xPortion);
            String type = (String) xPropSet.getPropertyValue("TextPortionType");
            assertEquals("second: not text", "TextField", type);
        }
        // no ruby end here!!!
        assertFalse("hasNext(): more elements?", xEnum.hasMoreElements());

        XComponent xComponent = UnoRuntime.queryInterface(XComponent.class, xMeta);
        xComponent.dispose();

        try {
            XTextCursor xCursor = xText.createTextCursor();
            assertNull("createTextCursor(): succeeds on disposed object?",
                       xCursor);
        } catch (RuntimeException e) { /* expected */ }
    }

    /** check that cursor move methods move to positions in the meta,
        but do not move to positions outside the meta. */
    @Test public void testMetaXTextCursor() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("Text. 12 More text here.");
        inserter.insertRange( new Range(0, 0, text) );
        MetaNode met1 = new MetaNode( mkId("id") );
        XTextContent xMeta = inserter.makeMeta();

        XText xDocText = m_xDoc.getText();
        XTextCursor xDocTextCursor = xDocText.createTextCursor();
        xDocTextCursor.goRight((short)7, false);
        xDocTextCursor.goRight((short)2, true);
        xDocText.insertTextContent(xDocTextCursor, xMeta, true);
        xDocTextCursor.gotoStart(true);

        XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xMeta);
        xMetadatable.setMetadataReference(met1.getXmlId());
        XText xText = UnoRuntime.queryInterface(XText.class, xMeta);

        XTextRange xStart = xText.getStart();
        assertNotNull("getStart(): no start", xStart);
        XTextRange xEnd = xText.getEnd();
        assertNotNull("getEnd(): no end", xEnd);

        XTextCursor xTextCursor = xText.createTextCursor();
        assertNotNull("createTextCursor(): no cursor", xTextCursor);

        // XTextCursor
        boolean bSuccess = false;
        xTextCursor.gotoStart(false);
        xTextCursor.gotoEnd(false);
        bSuccess = xTextCursor.goLeft((short)1, false);
        assertTrue("goLeft(): failed", bSuccess);
        bSuccess = xTextCursor.goLeft((short)1000, false);
        assertFalse("goLeft(): succeeded", bSuccess);
        bSuccess = xTextCursor.goRight((short)1, false);
        assertTrue("goRight(): failed", bSuccess);
        bSuccess = xTextCursor.goRight((short)1000, false);
        assertFalse("goRight(): succeeded", bSuccess);
        xTextCursor.gotoRange(xStart, false);
        xTextCursor.gotoRange(xEnd, false);
        try {
            xTextCursor.gotoRange(xDocTextCursor, false);
            fail("gotoRange(): succeeded");
        } catch (RuntimeException e) { /* expected */ }

        // XWordCursor
        xText.setString("Two words");
        xTextCursor.gotoStart(false);
        XWordCursor xWordCursor = UnoRuntime.queryInterface(XWordCursor.class, xTextCursor);

        bSuccess = xWordCursor.gotoNextWord(true);              //at start of "words"
        assertTrue("gotoNextWord(): failed", bSuccess);
        {
            String string = xTextCursor.getString();
            assertEquals("gotoNextWord(): wrong string",
                         "Two ", string);
        }
        bSuccess = xWordCursor.gotoNextWord(false);             //at end of "words", cannot leave metafield
        assertFalse("gotoNextWord(): succeeded", bSuccess);
        xTextCursor.collapseToEnd();
        bSuccess = xWordCursor.gotoPreviousWord(true);          //at start of "words"
        assertTrue("gotoPreviousWord(): failed", bSuccess);
        {
            String string = xTextCursor.getString();
            assertEquals("gotoPreviousWord(): wrong string",
                         "words", string);
        }
        bSuccess = xWordCursor.gotoPreviousWord(false);         //at start of "Two"
        assertTrue("gotoPreviousWord(): failed", bSuccess);


        bSuccess = xWordCursor.gotoPreviousWord(false);         //cannot leave metafield
        assertFalse("gotoPreviousWord(): succeeded", bSuccess);

        bSuccess = xWordCursor.gotoEndOfWord(true);             //at end of "Two"
        assertTrue("gotoEndOfWord(): failed", bSuccess);
        {
            String string = xTextCursor.getString();
            assertEquals("gotoEndOfWord(): wrong string",
                         "Two", string);
        }
        xTextCursor.gotoEnd(false);
        bSuccess = xWordCursor.gotoStartOfWord(true);
        assertTrue("gotoStartOfWord(): failed", bSuccess);
        {
            String string = xTextCursor.getString();
            assertEquals("gotoStartOfWord(): wrong string",
                         "words", string);
        }
        xText.setString("");
        bSuccess = xWordCursor.gotoEndOfWord(false);
        assertFalse("gotoEndOfWord(): succeeded", bSuccess);
        bSuccess = xWordCursor.gotoStartOfWord(false);
        assertFalse("gotoStartOfWord(): succeeded", bSuccess);

        // XSentenceCursor
        xText.setString("This is a sentence. Another sentence.");
        xTextCursor.gotoStart(false);
        XSentenceCursor xSentenceCursor = UnoRuntime.queryInterface(XSentenceCursor.class, xTextCursor);

        bSuccess = xSentenceCursor.gotoNextSentence(true);
        assertTrue("gotoNextSentence(): failed", bSuccess);
        {
            String string = xTextCursor.getString();
            assertEquals("gotoNextSentence(): wrong string",
                         "This is a sentence. ", string);
        }
        bSuccess = xSentenceCursor.gotoNextSentence(false);
        assertFalse("gotoNextSentence(): succeeded", bSuccess);
        // FIXME:
        // the sentence cursor seems to work differently than the word cursor
        xText.setString("This is a sentence. Another sentence. Sentence 3.");
        xTextCursor.gotoEnd(false);
        bSuccess = xSentenceCursor.gotoPreviousSentence(true);
        assertTrue("gotoPreviousSentence(): failed", bSuccess);
        {
            String string = xTextCursor.getString();
            assertEquals("gotoPreviousSentence(): wrong string",
                         "Another sentence. Sentence 3.", string);
        }
        bSuccess = xSentenceCursor.gotoPreviousSentence(false);
        assertFalse("gotoPreviousSentence(): succeeded", bSuccess);
        bSuccess = xSentenceCursor.gotoEndOfSentence(true);
        assertTrue("gotoEndOfSentence(): failed", bSuccess);
        {
            String string = xTextCursor.getString();
            assertEquals("gotoEndOfSentence(): wrong string",
                         "This is a sentence.", string);
        }
        xTextCursor.gotoEnd(false);
        bSuccess = xSentenceCursor.gotoStartOfSentence(true);
        assertTrue("gotoStartOfSentence(): failed", bSuccess);
        {
            String string = xTextCursor.getString();
            assertEquals("gotoStartOfSentence(): wrong string",
                         "Sentence 3.", string);
        }
        xText.setString("");
        bSuccess = xSentenceCursor.gotoEndOfSentence(false);
        assertFalse("gotoEndOfSentence(): succeeded", bSuccess);
        bSuccess = xSentenceCursor.gotoStartOfSentence(false);
        assertFalse("gotoStartOfSentence(): succeeded", bSuccess);

        XParagraphCursor xParagraphCursor = UnoRuntime.queryInterface(XParagraphCursor.class, xTextCursor);

        // XParagraphCursor (does not make sense)
        bSuccess = xParagraphCursor.gotoNextParagraph(false);
        assertFalse("gotoNextParagraph(): succeeded", bSuccess);
        bSuccess = xParagraphCursor.gotoPreviousParagraph(false);
        assertFalse("gotoPreviousParagraph(): succeeded", bSuccess);
        bSuccess = xParagraphCursor.gotoStartOfParagraph(false);
        assertFalse("gotoStartOfParagraph(): succeeded", bSuccess);
        bSuccess = xParagraphCursor.gotoEndOfParagraph(false);
        assertFalse("gotoEndOfParagraph(): succeeded", bSuccess);
    }

    /** See https://bugs.freedesktop.org/show_bug.cgi?id=49629
        ensure that gotoEndOfWord does not fail when footnote is at word end*/
    @Test public void testXTextCursor() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        XText xDocText = m_xDoc.getText();
        XTextCursor xDocTextCursor = xDocText.createTextCursor();
        inserter.insertText(xDocTextCursor, "Text");
        XWordCursor xWordCursor = UnoRuntime.queryInterface(XWordCursor.class, xDocTextCursor);
        xWordCursor.gotoEndOfWord(false);
        inserter.insertFootnote(xDocTextCursor, "footnote");
        xDocTextCursor.gotoStart(false);
        boolean bSuccess = xWordCursor.gotoEndOfWord(true);
        assertTrue("gotoEndOfWord(): failed", bSuccess);
        String string = xWordCursor.getString();
        assertEquals("gotoEndOfWord(): wrong string", "Text", string);
    }

    abstract class AttachHelper
    {
        abstract boolean isAttribute();
        abstract TreeNode mkTreeNode();
        abstract XTextContent mkTextContent(Inserter inserter, TreeNode node)
            throws Exception;
        void postInserted(TreeNode node, XTextContent xContent)
            throws Exception { }
    }

    @Test public void testMetaXTextAttachToxMark() throws Exception
    {
        doMetaXTextAttach( new AttachHelper()
            {
                boolean isAttribute() { return true; }
                TreeNode mkTreeNode() {
                    return new DocumentIndexMarkNode( mkName("toxmark") );
                }
                XTextContent mkTextContent(Inserter inserter, TreeNode node)
                        throws Exception {
                    return inserter.makeDocumentIndexMark(
                        ((DocumentIndexMarkNode)node).getName());
                }
            });
    }

    @Test public void testMetaXTextAttachRefMark() throws Exception
    {
        doMetaXTextAttach( new AttachHelper()
            {
                boolean isAttribute() { return true; }
                TreeNode mkTreeNode() {
                    return new ReferenceMarkNode( mkName("refmark") );
                }
                XTextContent mkTextContent(Inserter inserter, TreeNode node)
                        throws Exception {
                    return inserter.makeReferenceMark(
                        ((ReferenceMarkNode)node).getName());
                }
            });
    }

    @Test public void testMetaXTextAttachTextField() throws Exception
    {
        doMetaXTextAttach( new AttachHelper()
            {
                boolean isAttribute() { return false; }
                TreeNode mkTreeNode() {
                    return new TextFieldNode( mkName("field") );
                }
                XTextContent mkTextContent(Inserter inserter, TreeNode node)
                        throws Exception {
                    return inserter.makeTextField(
                        ((TextFieldNode)node).getContent());
                }
            });
    }

    @Test public void testMetaXTextAttachFootnote() throws Exception
    {
        doMetaXTextAttach( new AttachHelper()
            {
                boolean isAttribute() { return false; }
                TreeNode mkTreeNode() {
                    return new FootnoteNode( mkName("ftn") );
                }
                XTextContent mkTextContent(Inserter inserter, TreeNode node)
                        throws Exception {
                    return inserter.makeFootnote(
                        ((FootnoteNode)node).getLabel());
                }
            });
    }

    @Test public void testMetaXTextAttachMeta() throws Exception
    {
        doMetaXTextAttach( new AttachHelper()
            {
                boolean isAttribute() { return true; }
                TreeNode mkTreeNode() {
                    return new MetaNode( mkId("id") );
                }
                XTextContent mkTextContent(Inserter inserter, TreeNode node)
                        throws Exception {
                    return inserter.makeMeta();
                }
                void postInserted(TreeNode node, XTextContent xContent)
                        throws Exception {
                    XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xContent);
                    xMetadatable.setMetadataReference(
                            ((MetaNode)node).getXmlId());
                }
            });
    }

    void doMetaXTextAttach(AttachHelper helper) throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("12AB6789");
        inserter.insertRange( new Range(0, 0, text) );
        MetaNode met1 = new MetaNode( mkId("id") );
        XTextContent xMeta = inserter.makeMeta();

        XText xDocText = m_xDoc.getText();
        XTextCursor xDocTextCursor = xDocText.createTextCursor();
        xDocTextCursor.goRight((short)3, false);
        xDocTextCursor.goRight((short)2, true);
        xDocText.insertTextContent(xDocTextCursor, xMeta, true);

        XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xMeta);
        xMetadatable.setMetadataReference(met1.getXmlId());
        XText xText = UnoRuntime.queryInterface(XText.class, xMeta);
        XTextRange xStart = null;
        XTextRange xEnd = null;

        {
            xStart = xText.getStart();
            xEnd = xText.getEnd();

            TreeNode nod1 = helper.mkTreeNode();
            TreeNode nod2 = helper.mkTreeNode();
            XTextContent xContent1 = helper.mkTextContent(inserter, nod1);
            XTextContent xContent2 = helper.mkTextContent(inserter, nod2);

            // insertTextContent with meta getStart()/getEnd()
            xText.insertTextContent(xStart, xContent1, false);
            xText.insertTextContent(xEnd  , xContent2, false);

            helper.postInserted(nod1, xContent1);
            helper.postInserted(nod2, xContent2);

            TreeNode root = new TreeNode()
                .appendChild( new TextNode("12") )
                .appendChild( met1.dup()
                    .appendChild( nod1.dup() )
                    .appendChild( new TextNode("AB") )
                    .appendChild( nod2.dup() ) )
                .appendChild( new TextNode("6789") );
            doTest(root, false);
        }
        {
            xText.setString("AB");
            xStart = xText.getStart();
            xEnd = xText.getEnd();

            TreeNode nod1 = helper.mkTreeNode();
            TreeNode nod2 = helper.mkTreeNode();
            XTextContent xContent1 = helper.mkTextContent(inserter, nod1);
            XTextContent xContent2 = helper.mkTextContent(inserter, nod2);

            XTextCursor xTextCursor = xText.createTextCursor();
            xTextCursor.gotoStart(false);

            // insertTextContent with meta cursor
            xText.insertTextContent(xTextCursor, xContent1, false);
            xTextCursor.gotoEnd(false);
            xText.insertTextContent(xTextCursor, xContent2, false);

            helper.postInserted(nod1, xContent1);
            helper.postInserted(nod2, xContent2);

            TreeNode root = new TreeNode()
                .appendChild( new TextNode("12") )
                .appendChild( met1.dup()
                    .appendChild( nod1.dup() )
                    .appendChild( new TextNode("AB") )
                    .appendChild( nod2.dup() ) )
                .appendChild( new TextNode("6789") );
            doTest(root, false);
        }
        if (!helper.isAttribute())
        {
//            xText.setString("AB");
            xStart = xText.getStart();
            xEnd = xText.getEnd();

            TreeNode nod1 = helper.mkTreeNode();
            TreeNode nod2 = helper.mkTreeNode();
            XTextContent xContent1 = helper.mkTextContent(inserter, nod1);
            XTextContent xContent2 = helper.mkTextContent(inserter, nod2);

            XTextCursor xTextCursor = xText.createTextCursor();
            xTextCursor.gotoStart(false);
            xTextCursor.goRight((short)1, true);

            // insertTextContent with meta cursor and absorb
            xText.insertTextContent(xTextCursor, xContent1, true);
            xTextCursor.gotoEnd(false);
            xTextCursor.goLeft((short)1, true);
            xText.insertTextContent(xTextCursor, xContent2, true);

            helper.postInserted(nod1, xContent1);
            helper.postInserted(nod2, xContent2);

            TreeNode root = new TreeNode()
                .appendChild( new TextNode("12") )
                .appendChild( met1.dup()
                    .appendChild( nod1.dup() )
                    .appendChild( new TextNode("AB") )
                    .appendChild( nod2.dup() ) )
                .appendChild( new TextNode("6789") );
            doTest(root, false);
        }
        {
            xText.setString("AB");
            xStart = xText.getStart();
            xEnd = xText.getEnd();

            TreeNode nod1 = helper.mkTreeNode();
            TreeNode nod2 = helper.mkTreeNode();
            XTextContent xContent1 = helper.mkTextContent(inserter, nod1);
            XTextContent xContent2 = helper.mkTextContent(inserter, nod2);

            xDocTextCursor.gotoRange(xStart, false);

            // insertTextContent with document cursor
            xText.insertTextContent(xDocTextCursor, xContent1, false);
            xDocTextCursor.gotoRange(xEnd, false);
            xText.insertTextContent(xDocTextCursor, xContent2, false);

            helper.postInserted(nod1, xContent1);
            helper.postInserted(nod2, xContent2);

            TreeNode root = new TreeNode()
                .appendChild( new TextNode("12") )
                .appendChild( met1.dup()
                    .appendChild( nod1.dup() )
                    .appendChild( new TextNode("AB") )
                    .appendChild( nod2.dup() ) )
                .appendChild( new TextNode("6789") );
            doTest(root, false);
        }
        if (!helper.isAttribute())
        {
//            xText.setString("AB");
            xStart = xText.getStart();
            xEnd = xText.getEnd();

            TreeNode nod1 = helper.mkTreeNode();
            TreeNode nod2 = helper.mkTreeNode();
            XTextContent xContent1 = helper.mkTextContent(inserter, nod1);
            XTextContent xContent2 = helper.mkTextContent(inserter, nod2);

            xDocTextCursor.gotoRange(xStart, false);
            xDocTextCursor.goRight((short)1, true);

            // insertTextContent with document cursor and absorb
            xText.insertTextContent(xDocTextCursor, xContent1, true);
            xDocTextCursor.gotoRange(xEnd, false);
            xDocTextCursor.goLeft((short)1, true);
            xText.insertTextContent(xDocTextCursor, xContent2, true);

            helper.postInserted(nod1, xContent1);
            helper.postInserted(nod2, xContent2);

            TreeNode root = new TreeNode()
                .appendChild( new TextNode("12") )
                .appendChild( met1.dup()
                    .appendChild( nod1.dup() )
                    .appendChild( new TextNode("AB") )
                    .appendChild( nod2.dup() ) )
                .appendChild( new TextNode("6789") );
            doTest(root, false);
        }
        {
            xText.setString("AB");
            xStart = xText.getStart();
            xEnd = xText.getEnd();

            TreeNode nod1 = helper.mkTreeNode();
            TreeNode nod2 = helper.mkTreeNode();
            XTextContent xContent1 = helper.mkTextContent(inserter, nod1);
            XTextContent xContent2 = helper.mkTextContent(inserter, nod2);

            // attach to range from meta getStart()/getEnd()
            xContent1.attach(xStart);
            xContent2.attach(xEnd);

            helper.postInserted(nod1, xContent1);
            helper.postInserted(nod2, xContent2);

            TreeNode root = new TreeNode()
                .appendChild( new TextNode("12") )
                .appendChild( met1.dup()
                    .appendChild( nod1.dup() )
                    .appendChild( new TextNode("AB") )
                    .appendChild( nod2.dup() ) )
                .appendChild( new TextNode("6789") );
            doTest(root, false);
        }
        {
            xText.setString("AB");
            xStart = xText.getStart();
            xEnd = xText.getEnd();

            TreeNode nod1 = helper.mkTreeNode();
            TreeNode nod2 = helper.mkTreeNode();
            XTextContent xContent1 = helper.mkTextContent(inserter, nod1);
            XTextContent xContent2 = helper.mkTextContent(inserter, nod2);

            XTextCursor xTextCursor = xText.createTextCursor();
            xTextCursor.gotoStart(false);

            // attach to cursor from meta XText
            xContent1.attach(xTextCursor);
            xTextCursor.gotoEnd(false);
            xContent2.attach(xTextCursor);

            helper.postInserted(nod1, xContent1);
            helper.postInserted(nod2, xContent2);

            TreeNode root = new TreeNode()
                .appendChild( new TextNode("12") )
                .appendChild( met1.dup()
                    .appendChild( nod1.dup() )
                    .appendChild( new TextNode("AB") )
                    .appendChild( nod2.dup() ) )
                .appendChild( new TextNode("6789") );
            doTest(root, false);
        }
    }

    @Test public void testMetaFieldXTextField() throws Exception
    {
        com.sun.star.rdf.XRepositorySupplier xModel =
            UnoRuntime.queryInterface(
            com.sun.star.rdf.XRepositorySupplier.class, m_xDoc);
        com.sun.star.rdf.XRepository xRepo = xModel.getRDFRepository();
        // for testing just add it to the first graph
        com.sun.star.rdf.XURI[] Graphs = xRepo.getGraphNames();
        com.sun.star.rdf.XNamedGraph xGraph = xRepo.getGraph(Graphs[0]);
        com.sun.star.rdf.XURI xOdfPrefix =
            com.sun.star.rdf.URI.createKnown(m_xContext,
                com.sun.star.rdf.URIs.ODF_PREFIX);
        com.sun.star.rdf.XURI xOdfSuffix =
            com.sun.star.rdf.URI.createKnown(m_xContext,
                com.sun.star.rdf.URIs.ODF_SUFFIX);
        com.sun.star.rdf.XNode xPrefix =
            com.sun.star.rdf.Literal.create(m_xContext, "foo");
        com.sun.star.rdf.XNode xSuffix =
            com.sun.star.rdf.Literal.create(m_xContext, "bar");

        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("abc");
        inserter.insertRange( new Range(0, 0, text) );
        XText xDocText = m_xDoc.getText();
        XTextCursor xDocTextCursor = xDocText.createTextCursor();
        xDocTextCursor.goRight((short)1, false);
        xDocTextCursor.goRight((short)3, true);

        XTextField xMetaField = inserter.makeMetaField();

        xDocText.insertTextContent(xDocTextCursor, xMetaField, true);

        XMetadatable xMetadatable = UnoRuntime.queryInterface(XMetadatable.class, xMetaField);
        xMetadatable.ensureMetadataReference();

        xGraph.addStatement(xMetadatable, xOdfPrefix, xPrefix);
        xGraph.addStatement(xMetadatable, xOdfSuffix, xSuffix);
        assertEquals("getPresentation(): wrong",
                     "fooabcbar", xMetaField.getPresentation(false));
        inserter.insertRange( new Range(0, 0, text) );
    }

    @Test public void testMetaFieldXPropertySet() throws Exception
    {
        RangeInserter inserter = new RangeInserter(m_xDoc);
        TreeNode text = new TextNode("123");
        inserter.insertRange( new Range(0, 0, text) );
        XText xDocText = m_xDoc.getText();
        XTextCursor xDocTextCursor = xDocText.createTextCursor();
        xDocTextCursor.goRight((short)1, false);
        xDocTextCursor.goRight((short)3, true);

        XTextField xMetaField = inserter.makeMetaField();

        xDocText.insertTextContent(xDocTextCursor, xMetaField, true);

        XPropertySet xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xMetaField);
        assertNotNull("PropertySet: not supported?", xPropertySet);
        XPropertySetInfo xPropertySetInfo = xPropertySet.getPropertySetInfo();
        assertTrue("hasPropertyByName(\"NumberFormat\"):",
                   xPropertySetInfo.hasPropertyByName("NumberFormat"));
        assertTrue("hasPropertyByName(\"IsFixedLanguage\"):",
                   xPropertySetInfo.hasPropertyByName("IsFixedLanguage"));

        int def = (Integer) xPropertySet.getPropertyValue("NumberFormat");
        System.out.println("NumberFormat: default is " + def);
        short INT = com.sun.star.i18n.NumberFormatIndex.NUMBER_INT;
        xPropertySet.setPropertyValue("NumberFormat", INT);
        xPropertySet.setPropertyValue("IsFixedLanguage", true);
        int format = (Integer) xPropertySet.getPropertyValue("NumberFormat");
        assertEquals("NumberFormat: failed", INT, format);
        boolean isFixed = (Boolean)
            xPropertySet.getPropertyValue("IsFixedLanguage");
        assertTrue("IsFixedLanguage: failed", isFixed);
    }

    @Test public void testLoadStore() throws Exception
    {
        XTextDocument xComp = null;
        String filename = "TESTMETA.odt";
        String file;
        try {
            file = TestDocument.getUrl(filename);
            xComp = doLoad(file);
            if (xComp != null)
            {
                checkLoadMeta(xComp);
                file = m_TmpDir + filename;
                doStore(xComp, file);
                close(xComp);
                xComp = doLoad(file);
                checkLoadMeta(xComp);
            }
        } finally {
            close(xComp);
        }
    }

    private void doStore(XComponent xComp, String file) throws Exception
    {
        System.out.println("Storing test document...");

        XStorable xStor = UnoRuntime.queryInterface(
                    XStorable.class, xComp);

        xStor.storeToURL(file, new PropertyValue[0]);

        System.out.println("...done");
    }

    public XTextDocument doLoad(String file) throws Exception
    {
        XComponent xComp = null;

        System.out.println("Loading test document...");

        PropertyValue[] loadProps = new PropertyValue[1];
        loadProps[0] = new PropertyValue();
        loadProps[0].Name = "Hidden";
        loadProps[0].Value = new Boolean(true);

        xComp = util.DesktopTools.loadDoc(m_xMSF, file, loadProps);
//        xComp =  util.DesktopTools.getCLoader(m_xMSF).loadComponentFromURL(file, "_blank", 0, loadProps);

        XTextDocument xTextDoc = UnoRuntime.queryInterface(XTextDocument.class, xComp);

        assertNotNull("cannot load: " + file, xTextDoc);

        System.out.println("...done");

        return xTextDoc;
    }

    public void checkLoadMeta(XTextDocument xTextDoc) throws Exception
    {
        XText xText = xTextDoc.getText();

        System.out.println("Checking meta(-field)s in loaded test document...");

        TreeNode root = new TreeNode()
            .appendChild( new RubyNode("ruby1")
                .appendChild( new TextNode("1") ) )
            .appendChild( new MetaNode(mkId_("id1"))
                .appendChild( new TextNode("2") ) )
            .appendChild( new MetaFieldNode(mkId_("id2"))
                .appendChild( new TextNode("3") ) )
            .appendChild( new RubyNode("ruby2")
                .appendChild( new MetaNode(mkId_("id3"))
                    .appendChild( new TextNode("4") ) ) )
            .appendChild( new RubyNode("ruby3")
                .appendChild( new MetaFieldNode(mkId_("id4"))
                    .appendChild( new TextNode("5") ) ) )
            .appendChild( new MetaNode(mkId_("id5"))
                .appendChild( new RubyNode("ruby4")
                    .appendChild( new TextNode("6") ) ) )
            .appendChild( new MetaFieldNode(mkId_("id6"))
                .appendChild( new RubyNode("ruby5")
                    .appendChild( new TextNode("7") ) ) )
            .appendChild( new MetaNode(mkId_("id7"))
                .appendChild( new MetaNode(mkId_("id8"))
                    .appendChild( new TextNode("8") ) ) )
            .appendChild( new MetaNode(mkId_("id9"))
                .appendChild( new MetaFieldNode(mkId_("id10"))
                    .appendChild( new TextNode("9") ) ) )
            .appendChild( new MetaFieldNode(mkId_("id11"))
                .appendChild( new MetaNode(mkId_("id12"))
                    .appendChild( new TextNode("10") ) ) )
            .appendChild( new MetaFieldNode(mkId_("id13"))
                .appendChild( new MetaFieldNode(mkId_("id14"))
                    .appendChild( new TextNode("11") ) ) )
            .appendChild( new MetaNode(mkId_("id15"))
                .appendChild( new RubyNode("ruby6")
                    .appendChild( new MetaFieldNode(mkId_("id16"))
                        .appendChild( new TextNode("12") ) ) ) )
            .appendChild( new MetaNode(mkId_("")) {
                                public boolean equals(Object other) {
                                    return (other instanceof MetaNode);
                                } }
                .appendChild( new TextNode("13") ) )
            .appendChild( new TextNode(" X X ") );
        doTest(xTextDoc, root, false);

        System.out.println("...done");
    }

    @Test public void testLoadStoreXmlid() throws Exception
    {
        XTextDocument xComp = null;
        String filename = "TESTXMLID.odt";
        String file;
        try {
            file = TestDocument.getUrl(filename);
            xComp = doLoad(file);
            if (xComp != null)
            {
                checkLoadXmlId(xComp);
                file = m_TmpDir + filename;
                doStore(xComp, file);
                close(xComp);
                xComp = doLoad(file);
                checkLoadXmlId(xComp);
            }
        } finally {
            close(xComp);
        }
    }

    public void checkLoadXmlId(XTextDocument xTextDoc) throws Exception
    {
        XText xText = xTextDoc.getText();

        System.out.println("Checking bookmarks in loaded test document...");

        XRepositorySupplier xRS = UnoRuntime.queryInterface(XRepositorySupplier.class, xTextDoc);
        XDocumentRepository xRepo = UnoRuntime.queryInterface(XDocumentRepository.class,
            xRS.getRDFRepository());
        XBookmarksSupplier xBMS = UnoRuntime.queryInterface(XBookmarksSupplier.class, xTextDoc);
        XNameAccess xBookmarks = xBMS.getBookmarks();
        XMetadatable xMark1 = UnoRuntime.queryInterface(
                XMetadatable.class, xBookmarks.getByName("mk1"));
        assertTrue("mark1",
                eq(xMark1.getMetadataReference(),
                    new StringPair("content.xml", "id90")));

        XMetadatable xMark2 = UnoRuntime.queryInterface(
                XMetadatable.class, xBookmarks.getByName("mk2"));
        Pair<Statement[], Boolean> result = xRepo.getStatementRDFa(xMark2);
        assertTrue("mark2", (result.First.length == 1)
            && result.First[0].Subject.getStringValue().equals("uri:foo")
            && result.First[0].Predicate.getStringValue().equals("uri:bar")
            && result.First[0].Object.getStringValue().contains("a fooish bar")
            );

        XMetadatable xMark3 = UnoRuntime.queryInterface(
                XMetadatable.class, xBookmarks.getByName("mk3"));
        assertTrue("mark3",
                eq(xMark3.getMetadataReference(),
                    new StringPair("content.xml", "id91")));

        System.out.println("...done");

        System.out.println("Checking sections in loaded test document...");

        XTextSectionsSupplier xTSS = UnoRuntime.queryInterface(XTextSectionsSupplier.class, xTextDoc);

        XNameAccess xSections = xTSS.getTextSections();

        XMetadatable xSection1 = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Section 1"));
        assertTrue("idsection1", eq(xSection1.getMetadataReference(),
                    new StringPair("content.xml", "idSection1")));

        XMetadatable xSection2 = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Section 2"));
        assertTrue("idSection2", eq(xSection2.getMetadataReference(),
                    new StringPair("content.xml", "idSection2")));

        XMetadatable xSection3 = UnoRuntime.queryInterface(
                XMetadatable.class,
                xSections.getByName("Table of Contents1_Head"));
        assertTrue("idTOCTitle", eq(xSection3.getMetadataReference(),
                    new StringPair("content.xml", "idTOCTitle")));

        XMetadatable xSection4 = UnoRuntime.queryInterface(
                XMetadatable.class,
                xSections.getByName("Alphabetical Index1_Head"));
        assertTrue("idAITitle", eq(xSection4.getMetadataReference(),
                    new StringPair("content.xml", "idAITitle")));

        XMetadatable xSection5 = UnoRuntime.queryInterface(
                XMetadatable.class,
                xSections.getByName("Illustration Index1_Head"));
        assertTrue("idIITitle", eq(xSection5.getMetadataReference(),
                    new StringPair("content.xml", "idIITitle")));

        XMetadatable xSection6 = UnoRuntime.queryInterface(
                XMetadatable.class,
                xSections.getByName("Index of Tables1_Head"));
        assertTrue("idIOTTitle", eq(xSection6.getMetadataReference(),
                    new StringPair("content.xml", "idIOTTitle")));

        XMetadatable xSection7 = UnoRuntime.queryInterface(
                XMetadatable.class,
                xSections.getByName("User-Defined1_Head"));
        assertTrue("idUDTitle", eq(xSection7.getMetadataReference(),
                    new StringPair("content.xml", "idUDTitle")));

        XMetadatable xSection8 = UnoRuntime.queryInterface(
                XMetadatable.class,
                xSections.getByName("Table of Objects1_Head"));
        assertTrue("idTOOTitle", eq(xSection8.getMetadataReference(),
                    new StringPair("content.xml", "idTOOTitle")));

        XMetadatable xSection9 = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Bibliography1_Head"));
        assertTrue("idBibTitle", eq(xSection9.getMetadataReference(),
                    new StringPair("content.xml", "idBibTitle")));

        System.out.println("...done");

        System.out.println("Checking indexes in loaded test document...");

        XDocumentIndexesSupplier xDIS = UnoRuntime.queryInterface(XDocumentIndexesSupplier.class, xTextDoc);
        XIndexAccess xIndexesIA = xDIS.getDocumentIndexes();
        XNameAccess xIndexes =
            UnoRuntime.queryInterface(XNameAccess.class, xIndexesIA);

        XMetadatable xIndex1 = UnoRuntime.queryInterface(
                XMetadatable.class, xIndexes.getByName("Table of Contents1"));
        assertTrue("idTOC", eq(xIndex1.getMetadataReference(),
                    new StringPair("content.xml", "idTOC")));
        XMetadatable xIndex1s = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Table of Contents1"));
        assertTrue("idTOC", eq(xIndex1s.getMetadataReference(),
                    new StringPair("content.xml", "idTOC")));

        XMetadatable xIndex2 = UnoRuntime.queryInterface(
                XMetadatable.class, xIndexes.getByName("Alphabetical Index1"));
        assertTrue("idAI", eq(xIndex2.getMetadataReference(),
                    new StringPair("content.xml", "idAI")));
        XMetadatable xIndex2s = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Alphabetical Index1"));
        assertTrue("idAI", eq(xIndex2s.getMetadataReference(),
                    new StringPair("content.xml", "idAI")));

        XMetadatable xIndex3 = UnoRuntime.queryInterface(
                XMetadatable.class, xIndexes.getByName("Illustration Index1"));
        assertTrue("idII", eq(xIndex3.getMetadataReference(),
                    new StringPair("content.xml", "idII")));
        XMetadatable xIndex3s = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Illustration Index1"));
        assertTrue("idII", eq(xIndex3s.getMetadataReference(),
                    new StringPair("content.xml", "idII")));

        XMetadatable xIndex4 = UnoRuntime.queryInterface(
                XMetadatable.class, xIndexes.getByName("Index of Tables1"));
        assertTrue("idIOT", eq(xIndex4.getMetadataReference(),
                    new StringPair("content.xml", "idIOT")));
        XMetadatable xIndex4s = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Index of Tables1"));
        assertTrue("idIOT", eq(xIndex4s.getMetadataReference(),
                    new StringPair("content.xml", "idIOT")));

        XMetadatable xIndex5 = UnoRuntime.queryInterface(
                XMetadatable.class, xIndexes.getByName("User-Defined1"));
        assertTrue("idUD", eq(xIndex5.getMetadataReference(),
                    new StringPair("content.xml", "idUD")));
        XMetadatable xIndex5s = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("User-Defined1"));
        assertTrue("idUD", eq(xIndex5s.getMetadataReference(),
                    new StringPair("content.xml", "idUD")));

        XMetadatable xIndex6 = UnoRuntime.queryInterface(
                XMetadatable.class, xIndexes.getByName("Table of Objects1"));
        assertTrue("idTOO", eq(xIndex6.getMetadataReference(),
                    new StringPair("content.xml", "idTOO")));
        XMetadatable xIndex6s = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Table of Objects1"));
        assertTrue("idTOO", eq(xIndex6s.getMetadataReference(),
                    new StringPair("content.xml", "idTOO")));

        XMetadatable xIndex7 = UnoRuntime.queryInterface(
                XMetadatable.class, xIndexes.getByName("Bibliography1"));
        assertTrue("idBib", eq(xIndex7.getMetadataReference(),
                    new StringPair("content.xml", "idBib")));
        XMetadatable xIndex7s = UnoRuntime.queryInterface(
                XMetadatable.class, xSections.getByName("Bibliography1"));
        assertTrue("idBib", eq(xIndex7s.getMetadataReference(),
                    new StringPair("content.xml", "idBib")));

        System.out.println("...done");
    }

    static void close(XComponent i_comp)
    {
        try {
            XCloseable xClos = UnoRuntime.queryInterface(
                        XCloseable.class, i_comp);
            if (xClos != null) xClos.close(true);
        } catch (Exception e) {
        }
    }

    private void doTest(TreeNode intree) throws Exception
    {
        doTest(m_xDoc, intree, true);
    }

    private void doTest(TreeNode intree, boolean insert) throws Exception
    {
        doTest(m_xDoc, intree, insert);
    }

    private void doTest(XTextDocument xDoc, TreeNode intree,
            boolean insert) throws Exception
    {
        dumpTree(intree, "I: ");

        if (insert) {
            new TreeInserter(xDoc).insertTree(intree);
        }

//Thread.sleep(10000);

        XText xText = xDoc.getText();
        XEnumerationAccess xTextEA = UnoRuntime.queryInterface(XEnumerationAccess.class, xText);
        XEnumeration xTextEnum = xTextEA.createEnumeration();
        // skip to right paragraph
        xTextEnum.nextElement(); // skip first -- always empty!
        Object xElement = xTextEnum.nextElement(); // second contains test case
        XEnumerationAccess xEA = UnoRuntime.queryInterface(XEnumerationAccess.class, xElement);
        XEnumeration xEnum = xEA.createEnumeration();
        TreeNode outtree = new EnumConverter().convert(xEnum);

        dumpTree(outtree, "O: ");

        new FuzzyTester().doTest(intree, outtree);
    }

    private void dumpTree(TreeNode tree) { dumpTree(tree, "> "); }

    private void dumpTree(TreeNode tree, String prefix)
    {
        System.out.println(prefix + tree.toString());
        TreeNodeEnum children = tree.createEnumeration();
        while (children.hasNext()) {
            TreeNode node = children.next();
            dumpTree(node, prefix + "  ");
        }
    }

    private String mkName(String prefix)
    {
        return prefix + String.valueOf(m_Count++);
    }

    private StringPair mkId(String prefix)
    {
        return new StringPair("content.xml", mkName(prefix));
    }

    private StringPair mkId_(String id)
    {
        return new StringPair("content.xml", id);
    }

    static boolean eq(StringPair i_Left, StringPair i_Right)
    {
        return ((i_Left.First).equals(i_Right.First)) &&
            ((i_Left.Second).equals(i_Right.Second));
    }

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
}


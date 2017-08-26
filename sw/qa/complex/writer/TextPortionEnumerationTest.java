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
    private final Object[] m_Elements;
    private int m_Pos;
    TreeNodeEnum(Object[] Elements) { m_Elements = Elements; m_Pos = 0; }
    boolean hasNext() { return m_Pos < m_Elements.length; }
    TreeNode next() { return (TreeNode) m_Elements[m_Pos++]; }
}

/** base class for tree nodes. only instance: root of tree. */
class TreeNode
{
    private final List<TreeNode> m_Children;
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
    @Override
    public String toString() {
        return "<" + getType() + ">";
    }
}

abstract class MarkNode extends TreeNode
{
    boolean m_isPoint;
    boolean m_isStart = false;
    private final String m_Name;
    boolean isPoint() { return m_isPoint; }
    boolean isStart() { return m_isStart; }
    String getName() { return m_Name; }
    MarkNode(String name) { m_Name = name; m_isPoint = true; }
    boolean equals(MarkNode other) {
        return (other.m_Name.equals(m_Name)) && (other.m_isPoint == m_isPoint)
            && (other.m_isStart == m_isStart);
    }
    @Override
    public String toString() {
        return super.toString() + "\tname: " + m_Name +
            "\tisPoint: " + m_isPoint + "\tisStart: " + m_isStart;
    }
}

class BookmarkNode extends MarkNode
{
    private StringPair m_XmlId;
    StringPair getXmlId() { return m_XmlId; }
    @Override
    BookmarkNode dup() { return new BookmarkNode(getName(), getXmlId()); }
    BookmarkNode(String name) { this(name, new StringPair());  }
    BookmarkNode(String name, StringPair xmlid) {
        super(name); m_XmlId = xmlid;
    }
    @Override
    String getType() { return "Bookmark"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof BookmarkNode)
            && super.equals((MarkNode) other)
            && MetaNode.eq(((BookmarkNode) other).m_XmlId, m_XmlId);
    }
    @Override
    public String toString() {
        return super.toString()
            + "\txmlid: " + m_XmlId.First + "#" + m_XmlId.Second;
    }
}

class BookmarkStartNode extends BookmarkNode
{
    @Override
    BookmarkStartNode dup() { return new BookmarkStartNode(getName()); }
    BookmarkStartNode(String name) { this(name, new StringPair()); }
    BookmarkStartNode(String name, StringPair xmlid) {
        super(name, xmlid); m_isPoint = false; m_isStart = true;
    }
}

class BookmarkEndNode extends BookmarkNode
{
    @Override
    BookmarkEndNode dup() { return new BookmarkEndNode(getName()); }
    BookmarkEndNode(String name) { this(name, new StringPair()); }
    BookmarkEndNode(String name, StringPair xmlid) {
        super(name, xmlid); m_isPoint = false; m_isStart = false;
    }
}

class ReferenceMarkNode extends MarkNode
{
    @Override
    ReferenceMarkNode dup() { return new ReferenceMarkNode(getName()); }
    ReferenceMarkNode(String name) { super(name); }
    @Override
    String getType() { return "ReferenceMark"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof ReferenceMarkNode)
            && super.equals((MarkNode) other);
    }
}

class ReferenceMarkStartNode extends ReferenceMarkNode
{
    @Override
    ReferenceMarkStartNode dup()
    { return new ReferenceMarkStartNode(getName()); }
    ReferenceMarkStartNode(String name) {
        super(name); m_isPoint = false; m_isStart = true;
    }
}

class ReferenceMarkEndNode extends ReferenceMarkNode
{
    @Override
    ReferenceMarkEndNode dup()
    { return new ReferenceMarkEndNode(getName()); }
    ReferenceMarkEndNode(String name) {
        super(name); m_isPoint = false; m_isStart = false;
    }
}

class DocumentIndexMarkNode extends MarkNode
{
    @Override
    DocumentIndexMarkNode dup()
    { return new DocumentIndexMarkNode(getName()); }
    DocumentIndexMarkNode(String name) { super(name); }
    @Override
    String getType() { return "DocumentIndexMark"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof DocumentIndexMarkNode)
            && super.equals((MarkNode) other);
    }
}

class DocumentIndexMarkStartNode extends DocumentIndexMarkNode
{
    @Override
    DocumentIndexMarkStartNode dup()
    { return new DocumentIndexMarkStartNode(getName()); }
    DocumentIndexMarkStartNode(String name) {
        super(name); m_isPoint = false; m_isStart = true;
    }
}

class DocumentIndexMarkEndNode extends DocumentIndexMarkNode
{
    @Override
    DocumentIndexMarkEndNode dup()
    { return new DocumentIndexMarkEndNode(getName()); }
    DocumentIndexMarkEndNode(String name) {
        super(name); m_isPoint = false; m_isStart = false;
    }
}

abstract class ContentNode extends TreeNode
{
    private final String m_Content;
    String getContent() { return m_Content; }
    @Override
    boolean hasContent() { return true; }
    ContentNode(String content) {
        m_Content = content;
    }
    @Override
    TreeNode appendChild(TreeNode t) {
        throw new RuntimeException("ContentNode.appendChild");
    }
    @Override
    public String toString() {
        return super.toString() + "\tcontent: " + m_Content;
    }
    boolean equals(ContentNode other) {
        return other.m_Content.equals(m_Content);
    }
}

class TextNode extends ContentNode
{
    @Override
    TextNode dup() { return new TextNode(getContent()); }
    TextNode(String content) { super(content); }
    @Override
    String getType() { return "Text"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof TextNode) && super.equals((ContentNode) other);
    }
}

class TextFieldNode extends ContentNode
{
    @Override
    TextFieldNode dup() { return new TextFieldNode(getContent()); }
    TextFieldNode(String content) { super(content); }
    @Override
    String getType() { return "TextField"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof TextFieldNode)
            && super.equals((ContentNode) other);
    }
}

class FrameNode extends TreeNode
{
    private final String m_Name;
    private final TextContentAnchorType m_Anchor;
    String getName() { return m_Name; }
    TextContentAnchorType getAnchor() { return m_Anchor; }
    @Override
    FrameNode dup() { return new FrameNode(getName(), getAnchor()); }
    FrameNode(String name, TextContentAnchorType anchor) {
        m_Name = name; m_Anchor = anchor;
    }
    @Override
    String getType() { return "Frame"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof FrameNode)
            && (((FrameNode) other).m_Name.equals(m_Name))
            && (((FrameNode) other).m_Anchor == m_Anchor);
    }
    @Override
    public String toString() {
        return super.toString()
            + "\tname: " + m_Name + "\tanchor: " + toString(m_Anchor);
    }
    private static String toString(TextContentAnchorType anchor) {
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
    private final String m_Label;
    String getLabel() { return m_Label; }
    @Override
    FootnoteNode dup() { return new FootnoteNode(getLabel()); }
    FootnoteNode(String label) { m_Label = label; }
    @Override
    String getType() { return "Footnote"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof FootnoteNode)
            && (((FootnoteNode) other).m_Label.equals(m_Label));
    }
    @Override
    public String toString() {
        return super.toString() + "\tlabel: " + m_Label;
    }
}

class ControlCharacterNode extends TreeNode
{
    private final short m_Char;
    short getChar() { return m_Char; }
    @Override
    ControlCharacterNode dup() { return new ControlCharacterNode(getChar()); }
    ControlCharacterNode(short c) { m_Char = c; }
    @Override
    String getType() { return "ControlCharacter"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof ControlCharacterNode)
            && (((ControlCharacterNode) other).m_Char == m_Char);
    }
    @Override
    public String toString() {
        return super.toString() + "\tchar: " + m_Char;
    }
}

class SoftPageBreakNode extends TreeNode
{
    @Override
    String getType() { return "SoftPageBreak"; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof SoftPageBreakNode);
    }
}

class HyperlinkNode extends TreeNode
{
    private final String m_URL;
    String getURL() { return m_URL; }
    @Override
    HyperlinkNode dup() { return new HyperlinkNode(getURL()); }
    HyperlinkNode(String url) {
        if (url.length() == 0) throw new RuntimeException("HyperlinkNode");
        m_URL = url;
    }
    @Override
    String getType() { return "Hyperlink"; }
    @Override
    boolean isNesting() { return true; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof HyperlinkNode)
            && (((HyperlinkNode) other).m_URL.equals(m_URL));
    }
    @Override
    public String toString() {
        return super.toString() + "\turl: " + m_URL;
    }
}

class RubyNode extends TreeNode
{
    private final String m_Ruby;
    String getRubyText() { return m_Ruby; }
    @Override
    RubyNode dup() { return new RubyNode(getRubyText()); }
    RubyNode(String ruby) {
        if (ruby.length() == 0) throw new RuntimeException("RubyNode");
        m_Ruby = ruby;
    }
    @Override
    String getType() { return "Ruby"; }
    @Override
    boolean isNesting() { return true; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof RubyNode)
            && (((RubyNode) other).m_Ruby.equals(m_Ruby));
    }
    @Override
    public String toString() {
        return super.toString() + "\trubytext: " + m_Ruby;
    }
}

class MetaNode extends TreeNode
{
    private final StringPair m_XmlId;
    StringPair getXmlId() { return m_XmlId; }
    @Override
    MetaNode dup() { return new MetaNode(getXmlId()); }
    MetaNode (StringPair xmlid) { m_XmlId = xmlid; }
    @Override
    String getType() { return "InContentMetadata"; }
    @Override
    boolean isNesting() { return true; }
    @Override
    public boolean equals(Object other) {
        return (other instanceof MetaNode)
            && eq(((MetaNode) other).m_XmlId, m_XmlId);
    }
    static boolean eq(StringPair left, StringPair right)
    {
        return left.First.equals(right.First)
            && left.Second.equals(right.Second);
    }
    @Override
    public String toString() {
        return super.toString()
            + "\txmlid: " + m_XmlId.First + "#" + m_XmlId.Second;
    }
}

class MetaFieldNode extends MetaNode
{
    @Override
    MetaFieldNode dup() { return new MetaFieldNode(getXmlId()); }
    MetaFieldNode (StringPair xmlid) { super(xmlid); }
    @Override
    String getType() { return "MetadataField"; }
}

class Range
{
    private final short m_Start;
    private final short m_End;
    private final TreeNode m_Node;
    short getStart()    { return m_Start; }
    short getExtent()   { return (short) (m_End - m_Start); }
    TreeNode getNode()  { return m_Node; }
    Range(int start, int end, TreeNode node)
    { m_Start = (short) start; m_End = (short) end; m_Node = node; }
}

/* this is where we nail the pudding to the wall */
class FuzzyTester
{
    private long m_DiffContent = 0;
    private long m_DiffMissing = 0;
    private long m_DiffNesting = 0;
    private long m_DiffSpuriousEmptyText = 0;
    private long m_DiffSequence = 0; // ignored?
    private final Stack<Pair<TreeNode, TreeNodeEnum>> m_StackExpected;
    private final Stack<Pair<TreeNode, TreeNodeEnum>> m_StackActual;
    private final List<TreeNode> m_BufferExpected;
    private final List<TreeNode> m_BufferActual;

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
                m_BufferActual.get(j);
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
                printMissing(node);
                m_DiffMissing++;
            }
        }
        for (int j = 0; j < lenActual - 1; j++)
        {
            TreeNode node = m_BufferActual.get(j);
            if (node != null)
            {
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

    private void printDiff(String prefix, String expected, String actual)
    {
        System.out.println(prefix +
                ":\texpected: " + expected + "\tactual: " + actual);
    }

    private void printNesting(TreeNode node, TreeNode nesting)
    {
        System.out.println("node: " + node.toString()
                + " possibly moved across nesting " + nesting.toString());
    }

    private void printMissing(TreeNode node)
    {
        System.out.println("   missing node: " + node.toString());

    }

    private void printUnexpected(TreeNode node)
    {
        System.out.println("unexpected node: " + node.toString());

    }
}

class EnumConverter
{
    private final Stack<TreeNode> m_Stack;

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

    private TreeNode convertChildren(XEnumeration xEnum) throws Exception
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

abstract class Inserter
{
    private final XMultiServiceFactory m_xDocFactory;
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

    private XTextContent makeFrame(String name, TextContentAnchorType anchor)
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

    private XTextContent makeBookmark(String name) throws Exception
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
    private final Map<String, XTextRange> m_BookmarkStarts;
    private final Map<String, XTextRange> m_ReferenceMarkStarts;
    private final Map<String, XTextRange> m_DocumentIndexMarkStarts;
    private final List<Pair<XTextRange, FrameNode>> m_FrameHints;

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

    private void insertChildren(TreeNodeEnum children) throws Exception
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

    private XParagraphCursor mkCursor(XTextRange xRange)
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

    private XTextContent insertNode(XParagraphCursor xCursor, TreeNode node)
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

    private abstract class AttachHelper
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
                @Override
                boolean isAttribute() { return true; }
                @Override
                TreeNode mkTreeNode() {
                    return new DocumentIndexMarkNode( mkName("toxmark") );
                }
                @Override
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
                @Override
                boolean isAttribute() { return true; }
                @Override
                TreeNode mkTreeNode() {
                    return new ReferenceMarkNode( mkName("refmark") );
                }
                @Override
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
                @Override
                boolean isAttribute() { return false; }
                @Override
                TreeNode mkTreeNode() {
                    return new TextFieldNode( mkName("field") );
                }
                @Override
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
                @Override
                boolean isAttribute() { return false; }
                @Override
                TreeNode mkTreeNode() {
                    return new FootnoteNode( mkName("ftn") );
                }
                @Override
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
                @Override
                boolean isAttribute() { return true; }
                @Override
                TreeNode mkTreeNode() {
                    return new MetaNode( mkId("id") );
                }
                @Override
                XTextContent mkTextContent(Inserter inserter, TreeNode node)
                        throws Exception {
                    return inserter.makeMeta();
                }
                @Override
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
        loadProps[0].Value = Boolean.TRUE;

        xComp = util.DesktopTools.loadDoc(m_xMSF, file, loadProps);

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
                                @Override
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
        return prefix + (m_Count++);
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
        return (i_Left.First.equals(i_Right.First)) &&
               (i_Left.Second.equals(i_Right.Second));
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


Writer application code.

Exact history was lost before Sept. 18th, 2000, but old source code
comments show that Writer core dates back until at least November
1990.

== Module contents ==
 * inc: headers available to all source files inside the module
 * qa: unit, slow and subsequent tests
 * sdi
 * source: see below
 * uiconfig: user interface configuration
 * util: UNO passive registration config

== Source contents ==
 * core: Writer core (document model, layout, UNO API implementation)
 * filter: Writer internal filters
   * ascii: plain text filter
   * basflt
   * docx: wrapper for the UNO DOCX import filter (in writerfilter) for autotext purposes
   * html: HTML filter
   * inc: include files for filters
   * rtf: thin copy&paste helper around the UNO RTF import filter (in writerfilter)
   * writer
   * ww8: DOC import, DOC/DOCX/RTF export
   * xml: ODF import/export, subclassed from xmloff (where most of the work is done)
 * uibase: user interface (those parts that are linked into sw & always loaded)
 * ui: user interface (optional parts that are loaded on demand (swui))

== Core ==

There is a good overview documentation of basic architecture of Writer core
in the OOo wiki:

http://wiki.openoffice.org/wiki/Writer/Core_And_Layout
http://wiki.openoffice.org/wiki/Writer/Text_Formatting

Writer specific WhichIds are defined in sw/inc/hintids.hxx.

The details below are mainly about details missing from the Wiki pages.

=== SwDoc ===

The central class for a document is SwDoc, which represents a document.

A lot of the functionality is split out into separate Manager classes,
each of which implements some IDocument* interface; there are
SwDoc::getIDocument*() methods to retrieve the managers.

However there are still too many members and methods in this class,
many of which could be moved to some Manager or other...

=== SwNodes ===

Basically a (fancy) array of SwNode pointers.  There are special subclasses of
SwNode (SwStartNode and SwEndNode) which are used to encode a nested tree
structure into the flat array; the range of nodes from SwStartNode to its
corresponding SwEndNode is sometimes called a "section" (but is not necessarily
what the high-level document model calls a "Section"; that is just one of the
possibilities).

The SwNodes contains the following top-level sections:

1. Empty
2. Footnote content
3. Frame / Header / Footer content
4. Deleted Change Tracking content
5. Body content

=== Undo ===

The Undo/Redo information is stored in a sw::UndoManager member of SwDoc,
which implements the IDocumentUndoRedo interface.
Its members include a SwNodes array containing the document content that
is currently not in the actual document but required for Undo/Redo, and
a stack of SwUndo actions, each of which represents one user-visible
Undo/Redo step.

There are also ListActions which internally contain several individual SwUndo
actions; these are created by the StartUndo/EndUndo wrapper methods.

=== Text Attributes ===

The sub-structure of paragraphs is stored in the SwpHintsArray member
SwTextNode::m_pSwpHints.  There is a base class SwTextAttr with numerous
subclasses; the SwTextAttr has a start and end index and a SfxPoolItem
to store the actual formatting attribute.

There are several sub-categories of SwTextAttr:

- formatting attributes: Character Styles (SwTextCharFormat, RES_TXTATR_CHARFMT)
  and Automatic Styles (no special class, RES_TXTATR_AUTOFMT):
  these are handled by SwpHintsArray::BuildPortions and MergePortions,
  which create non-overlapping portions of formatting attributes.

- nesting attributes: Hyperlinks (SwTextINetFormat, RES_TXTATR_INETFMT),
  Ruby (SwTextRuby, RES_TXTATR_CJK_RUBY) and Meta/MetaField (SwTextMeta,
  RES_TXTATR_META/RES_TXTATR_METAFIELD):
  these maintain a properly nested tree structure.
  The Meta/Metafield are "special" because they have both start/end
  and a dummy character at the start.

- misc. attributes: Reference Marks, ToX Marks

- attributes without end: Fields, Footnotes, Flys (AS_CHAR)
  These all have a corresponding dummy character in the paragraph text, which
  is a placeholder for the "expansion" of the attribute, e.g. field content.

=== Fields ===

There are multiple model classes involved for fields:

- enum SwFieldIds enumerates the different types of fields.
- SwFieldType contains some shared stuff for all fields of a type.
  There are many subclasses of SwFieldType, one for each different type
  of field.
  For most types of fields there is one shared instance of this per type,
  which is created in DocumentFieldsManager::InitFieldTypes()
  but for some there are more than one, and they are dynamically created, see
  DocumentFieldsManager::InsertFieldType().  An example for the latter are
  variable fields (SwFieldIds::GetExp/SwFieldIds::SetExp), with one SwFieldType per
  variable.
- SwXFieldMaster is the UNO wrapper of a field type.
  It is a SwClient registered at the SwFieldType.
  Its life-cycle is determined by UNO clients outside of sw; it will get
  disposed when the SwFieldType dies.
- SwFormatField is the SfxPoolItem of a field.
  The SwFormatField is a SwClient registered at its SwFieldType.
  The SwFormatField owns the SwField of the field.
- SwField contains the core logic of a field.
  The SwField is owned by the SwFormatField of the field.
  There are many subclasses of SwField, one for each different type of field.
  Note that there are not many places that can Expand the field to its
  correct value, since for example page number fields require a View
  with an up to date layout; therefore the correct expansion is cached.
- SwTextField is the text attribute of a field.
  It owns the SwFormatField of the field (like all text attributes).
- SwXTextField is the UNO wrapper object of a field.
  It is a SwClient registered at the SwFormatField.
  Its life-cycle is determined by UNO clients outside of sw; it will get
  disposed when the SwFormatField dies.

=== Lists ===

- SwNumFormat (subclass of SvxNumFormat) determines the formatting of a single
  numbering level.

- SwNumRule (NOT a subclass of SvxNumRule) is a *list style*, containing one
  SwNumFormat per list level.
  SwNumRule::maTextNodeList is the list of SwTextNode that have this list style
  applied.

- SwNumberTreeNode is a base class that represents an abstract node in a
  hierarchical tree of numbered nodes.

- SwNodeNum is the subclass of SwNumberTreeNode that connects it with an
  actual SwTextNode and also with a SwNumRule;
  SwTextNode::mpNodeNum points back in the other direction

- SwList represents a list, which is (mostly) a vector of SwNodeNum trees,
  one per SwNodes top-level section (why that?).

- IDocumentListsAccess, sw::DocumentListsManager owns all SwList instances,
  and maintains mappings:
  + from list-id to SwList
  + from list style name to SwList (the "default" SwList for that list style)

- IDocumentListItems, sw::DocumentListItemsManager contains a set of all
  SwNodeNum instances, ordered by SwNode index

- the special Outline numbering rule: SwDoc::mpOutlineRule

- IDocumentOutlineNodes, sw::DocumentOutlineNodesManager maintain
  a list (which is actually stored in SwNodes::m_pOutlineNodes) of SwTextNodes
  that either have the Outline numrule applied,
  or have the RES_PARATR_OUTLINELEVEL item set (note that in the latter case,
  the SwTextNode does not have a SwNodeNum and is not associated with the
  SwDoc::mpOutlineRule).

- SwTextNodes and paragraph styles have items/properties:
  + RES_PARATR_OUTLINELEVEL/"OutlineLevel" to specify an outline level without
    necessarily having the outline SwNumRule assigned
  + RES_PARATR_NUMRULE/"NumberingStyleName" the list style to apply; may be
    empty "" which means no list style (to override inherited value)
  Only SwTextNode has these items:
  + RES_PARATR_LIST_ID/"ListId"
    determines the SwList to which the node is added
  + RES_PARATR_LIST_LEVEL/"NumberingLevel"
    the level at which the SwTextNode will appear in the list
  + RES_PARATR_LIST_ISRESTART/"ParaIsNumberingRestart"
    restart numbering sequence at this SwTextNode
  + RES_PARATR_LIST_RESTARTVALUE/"NumberingStartValue"
    restart numbering sequence at this SwTextNode with this value
  + RES_PARATR_LIST_ISCOUNTED/"NumberingIsNumber"
    determines if the node is actually counted in the numbering sequence;
    these are different from "phantoms" because there's still a SwTextNode.

Note that there is no UNO service to represent a list.

=== Layout ===

The layout is a tree of SwFrame subclasses, the following relationships are
possible between frames:

- You can visit the tree by following the upper, lower, next and previous pointers.
- The functionality of flowing of a frame across multiple parents (e.g. pages)
  is implemented in SwFlowFrame, which is not an SwFrame subclass. The logical
  chain of such frames can be visited using the follow and precede pointers.
  ("Leaf" is a term that refers to such a relationship.)
- In case a frame is split into multiple parts, then the first one is called
  master, while the others are called follows.

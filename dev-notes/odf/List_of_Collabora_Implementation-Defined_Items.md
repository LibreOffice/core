
# ODF Implementer Notes: List of Collabora implementation-defined items

The OpenDocument Format standard defines, "Implementation-defined is
used in this standard for values or processing that may differ between
ODF implementations, but is required to be specified by the implementor
for each particular ODF-implementation."
(section 1.2 Terminology, Part 1, ODF 1.2).
The list below specifies these values and processing for Collabora Online
and Collabora Office.

## General

The references "ODF 1.2" refer the documents of the standard as
published at
[https://www.oasis-open.org/standards#opendocumentv1.2](https://www.oasis-open.org/standards#opendocumentv1.2)

The references "ODF 1.3" currently (as of July 2020) refer the documents
of "Committee Specification 01" of the to become standard as published
at
[https://docs.oasis-open.org/office/OpenDocument/v1.3/cs01/](https://docs.oasis-open.org/office/OpenDocument/v1.3/cs01/)

The OpenDocumentFormat is standardized too as ISO/IEC 26300 "Information
technology -- Open Document Format for Office Applications
(OpenDocument) v1.2", and available from
[https://standards.iso.org/ittf/PubliclyAvailableStandards/index.html](https://standards.iso.org/ittf/PubliclyAvailableStandards/index.html)

The list items do not yet contain the information about the
implementation in Collabora. Help to get this done is highly
appreciated.

## List of "implementation-defined" items

### Cursor Position Setting

3.11 Cursor Position Setting, Part 1, ODF 1.2  
3.11 Cursor Position Setting, Part 3, ODF 1.3

Collabora Online and Collabora Office do not implement this
processing instruction. No implementation defined values are supported.

### Document Signatures

3.16 Document Signatures, Part 1, ODF 1.2  
3.16 Document Signatures, Part 3, ODF 1.3

No signatures other than document signatures are supported.

### Non-RDF Metadata

4.3.1 General, Part 1, ODF 1.2  
4.3.1 General, Part 3, ODF 1.3

Existing Custom metadata elements will be preserved when consuming, but
otherwise no new Custom metadata elements will be produced.

### text:alphabetical-index-auto-mark-file

8.8.3 \<text:alphabetical-index-auto-mark-file\>, Part 1, ODF 1.2  
8.8.3 \<text:alphabetical-index-auto-mark-file\>, Part 3, ODF 1.3

The index mark file is a newline-separated list of entries.

The encoding of the file is the platform dependent default encoding
(e.g. Windows Code Page).

Lines that start with `#` are ignored as comments.

Entries are tokenised by semicolons `;` as follows:

`TextToSearchFor;AlternativeString;PrimaryKey;SecondaryKey;CaseSensitive;WordOnly`

`CaseSensitive` and `WordOnly` are booleans, with values encoded as
strings `0` and `1`.

Leading and trailing blanks are ignored.

### chart:coordinate-region

11.3 \<chart:coordinate-region\>, Part 3, ODF 1.3

Collabora Online and Collabora Office have no non-predefined chart classes.

### chart:plot-area\>

11.5 \<chart:plot-area\>, Part 3, ODF 1.3

### form:generic-control

13.5.25 \<form:generic-control\>, Part 1, ODF 1.2  
13.5.25 \<form:generic-control\>, Part 3, ODF 1.3

The generic control may have a name and a service name and events and
possibly more.

The only currently supported generic control is a navigation bar; it can
be recognized by its attribute
`form:control-implementation="ooo:com.sun.star.form.component.NavigationToolBar"`.

It displays a label "Record N of M" and some navigation buttons to jump
to the first, previous, next, and last record.

### Event Types

14.4.3 Event Types, Part 1, ODF 1.2  
14.5.3 Event Types, Part 3, ODF 1.3

The following events may be implemented:

|                                      |                        |
|--------------------------------------|------------------------|
| Value of script:event-name Attribute | Internal event name    |
| dom:select                           | "OnSelect"             |
| office:insert-start                  | "OnInsertStart"        |
| office:insert-done                   | "OnInsertDone"         |
| office:mail-merge                    | "OnMailMerge"          |
| office:alpha-char-input              | "OnAlphaCharInput"     |
| office:non-alpha-char-input          | "OnNonAlphaCharInput"  |
| dom:resize                           | "OnResize"             |
| office:move                          | "OnMove"               |
| office:page-count-change             | "OnPageCountChange"    |
| dom:mouseover                        | "OnMouseOver"          |
| dom:click                            | "OnClick"              |
| dom:mouseout                         | "OnMouseOut"           |
| office:load-error                    | "OnLoadError"          |
| office:load-cancel                   | "OnLoadCancel"         |
| office:load-done                     | "OnLoadDone"           |
| dom:load                             | "OnLoad"               |
| dom:unload                           | "OnUnload"             |
| office:start-app                     | "OnStartApp"           |
| office:close-app                     | "OnCloseApp"           |
| office:new                           | "OnNew"                |
| office:save                          | "OnSave"               |
| office:save-as                       | "OnSaveAs"             |
| dom:DOMFocusIn                       | "OnFocus"              |
| dom:DOMFocusOut                      | "OnUnfocus"            |
| office:print                         | "OnPrint"              |
| dom:error                            | "OnError"              |
| office:load-finished                 | "OnLoadFinished"       |
| office:save-finished                 | "OnSaveFinished"       |
| office:modify-changed                | "OnModifyChanged"      |
| office:prepare-unload                | "OnPrepareUnload"      |
| office:new-mail                      | "OnNewMail"            |
| office:toggle-fullscreen             | "OnToggleFullscreen"   |
| office:save-done                     | "OnSaveDone"           |
| office:save-as-done                  | "OnSaveAsDone"         |
| office:copy-to                       | "OnCopyTo"             |
| office:copy-to-done                  | "OnCopyToDone"         |
| office:view-created                  | "OnViewCreated"        |
| office:prepare-view-closing          | "OnPrepareViewClosing" |
| office:view-close                    | "OnViewClosed"         |
| office:visarea-changed               | "OnVisAreaChanged"     |
| office:create                        | "OnCreate"             |
| office:save-as-failed                | "OnSaveAsFailed"       |
| office:save-failed                   | "OnSaveFailed"         |
| office:copy-to-failed                | "OnCopyToFailed"       |
| office:title-changed                 | "OnTitleChanged"       |
| office:mode-changed                  | "OnModeChanged"        |
| office:save-to                       | "OnSaveTo"             |
| office:save-to-done                  | "OnSaveToDone"         |
| office:save-to-failed                | "OnSaveToFailed"       |
| office:subcomponent-opened           | "OnSubComponentOpened" |
| office:subcomponent-closed           | "OnSubComponentClosed" |
| office:storage-changed               | "OnStorageChanged"     |
| office:mail-merge-finished           | "OnMailMergeFinished"  |
| office:field-merge                   | "OnFieldMerge"         |
| office:field-merge-finished          | "OnFieldMergeFinished" |
| office:layout-finished               | "OnLayoutFinished"     |
| office:dblclick                      | "OnDoubleClick"        |
| office:contextmenu                   | "OnRightClick"         |
| office:content-changed               | "OnChange"             |
| office:calculated                    | "OnCalculate"          |

Forms may support the following events:

|  |  |
|----|----|
| Value of script:event-name Attribute | Internal event name |
| form:approveaction | "XApproveActionListener::approveAction" |
| form:performaction | "XActionListener::actionPerformed" |
| dom:change | "XChangeListener::changed" |
| form:textchange | "XTextListener::textChanged" |
| form:itemstatechange | "XItemListener::itemStateChanged" |
| dom:DOMFocusIn | "XFocusListener::focusGained" |
| dom:DOMFocusOut | "XFocusListener::focusLost" |
| dom:keydown | "XKeyListener::keyPressed" |
| dom:keyup | "XKeyListener::keyReleased" |
| dom:mouseover | "XMouseListener::mouseEntered" |
| form:mousedrag | "XMouseMotionListener::mouseDragged" |
| dom:mousemove | "XMouseMotionListener::mouseMoved" |
| dom:mousedown | "XMouseListener::mousePressed" |
| dom:mouseup | "XMouseListener::mouseReleased" |
| dom:mouseout | "XMouseListener::mouseExited" |
| form:approvereset | "XResetListener::approveReset" |
| dom:reset | "XResetListener::resetted" |
| dom:submit | "XSubmitListener::approveSubmit" |
| form:approveupdate | "XUpdateListener::approveUpdate" |
| form:update | "XUpdateListener::updated" |
| dom:load | "XLoadListener::loaded" |
| form:startreload | "XLoadListener::reloading" |
| form:reload | "XLoadListener::reloaded" |
| form:startunload | "XLoadListener::unloading" |
| dom:unload | "XLoadListener::unloaded" |
| form:confirmdelete | "XConfirmDeleteListener::confirmDelete" |
| form:approverowchange | "XRowSetApproveListener::approveRowChange" |
| form:rowchange | "XRowSetListener::rowChanged" |
| form:approvecursormove | "XRowSetApproveListener::approveCursorMove" |
| form:cursormove | "XRowSetListener::cursorMoved" |
| form:supplyparameter | "XDatabaseParameterListener::approveParameter" |
| dom:error | "XSQLErrorListener::errorOccured" |
| form:adjust | "XAdjustmentListener::adjustmentValueChanged" |

### text:notes-configuration

16.29.3 \<text:notes-configuration\>, Part 1, ODF 1.2  
16.31.3 \<text:notes-configuration\>, Part 3, ODF 1.3

The default values for `text:note-class="footnote"`:

|                                        |                       |
|----------------------------------------|-----------------------|
| Attribute                              | Default value         |
| style:num-format                       | "1"                   |
| style:num-letter-sync                  | "false"               |
| style:num-prefix                       | ""                    |
| style:num-suffix                       |  ""                   |
| text:citation-body-style-name          | "Footnote Anchor"     |
| text:citation-style-name               | "Footnote Characters" |
| text:default-style-name                | "Footnote"            |
| text:footnotes-position                | "page"                |
| text:master-page-name                  | "Footnote"            |
| text:start-numbering-at                | "document"            |
| text:start-value                       | "1"                   |
| text:note-continuation-notice-backward | ""                    |
| text:note-continuation-notice-forward  | ""                    |

The default values for `text:note-class="endnote"`:

|                                        |                      |
|----------------------------------------|----------------------|
| Attribute                              | Default value        |
| style:num-format                       | "i"                  |
| style:num-letter-sync                  | "false"              |
| style:num-prefix                       | ""                   |
| style:num-suffix                       | ""                   |
| text:citation-body-style-name          | "Endnote Anchor"     |
| text:citation-style-name               | "Endnote Characters" |
| text:default-style-name                | "Endnote"            |
| text:footnotes-position                | "document"           |
| text:master-page-name                  | "Endnote"            |
| text:start-numbering-at                | "document"           |
| text:start-value                       | "1"                  |
| text:note-continuation-notice-backward | ""                   |
| text:note-continuation-notice-forward  | ""                   |

### db:table-definition

19.85.4 \<db:table-definition\>, Part 1, ODF 1.2  
19.85.4 \<db:table-definition\>, Part 3, ODF 1.3

Collabora Online and Collabora Office do not support the
element `db:schema-definition` or its children.

### draw:extrusion-first-light-harsh

19.155 draw:extrusion-first-light-harsh, Part 1, ODF 1.2  
19.155 draw:extrusion-first-light-harsh, Part 3, ODF 1.3

up to 7.2: The attribute is not evaluated.  
7.3: The value `false` is rendered same as
`draw:extrusion-specularity="0"`.  
since 7.4: Four light directions are added with 60deg angle to
`draw:extrusion-first-light-direction`. The light intensity given in
`draw:extrusion-first-light-level` is distributed so that a surface
perpendicular to the first light direction gets the same amount of light
as in case value `true`. The implementation is in method
[EnhancedCustomShape3d::Create3DObject(...)](https://cgit.collaboraoffice.com/c/online/tree/engine/svx/source/customshapes/EnhancedCustomShape3d.cxx#n245).
In case the second light is soft too, not four but two additional
directions are used.

### draw:extrusion-second-light-harsh

19.164 draw:extrusion-second-light-harsh, Part 1, ODF 1.2  
19.164 draw:extrusion-second-light-harsh, Part 3, ODF 1.3

up to 7.3: The attribute is not evaluated.
since 7.4: Four light directions are added with 60deg angle to
`draw:extrusion-second-light-direction`. The light intensity given in
`draw:extrusion-second-light-level` is distributed so that a surface
perpendicular to the second light direction gets the same amount of
light as in case value `true`. The implementation is in method
[EnhancedCustomShape3d::Create3DObject(...)](https://cgit.collaboraoffice.com/c/online/tree/engine/svx/source/customshapes/EnhancedCustomShape3d.cxx#n245).
In case the first light is soft too, not four but two additional
directions are used.

### form:datasource

19.264 form:datasource, Part 1, ODF 1.2  
19.264 form:datasource, Part 3, ODF 1.3

The mapping from data source names to specific instances of database
front end documents is implemented by the
`com.sun.star.sdb.DatabaseContext` service and stored in the
configuration; refer to the API documentation below for details.

[com::sun::star::sdb::DatabaseContext](https://api.libreoffice.org/docs/idl/ref/servicecom_1_1sun_1_1star_1_1sdb_1_1DatabaseContext.html)

### form:default-button

19.265 form:default-button, Part 1, ODF 1.2  
19.265 form:default-button, Part 3, ODF 1.3

If multiple buttons with `form:default-button` attributes with values of
`true` are present in a form, the first such button is the default for the
form.

### presentation:show-end-of-presentation-slide

19.414 presentation:show-end-of-presentation-slide, part 1, ODF 1.2  
19.418 presentation:show-end-of-presentation-slide, part 3, ODF 1.3

Collabora Online and Collabora Office do not implement this attribute.

### presentation:show-logo

19.419 presentation:show-logo, Part 3, ODF 1.3

The logo can be retrieved from:
[about.svg](https://cgit.collaboraoffice.com/c/online/tree/engine/icon-themes/colibre/brand/shell/about.svg?id=main)

### presentation:speed

19.417 presentation:speed, Part 1, ODF 1.2  
19.421 presentation:speed, Part 3, ODF 1.3

|                                       |      |
|---------------------------------------|------|
| Value of presentation:speed attribute | rate |
| slow                                  | 2.0s |
| medium                                | 1.0s |
| fast                                  | 0.5s |

### style:leader-width

19.489 style:leader-width, Part 1, ODF 1.2  
19.493 style:leader-width, Part 3, ODF 1.3

### style:line-style

19.493 style:line-style, Part 1, ODF 1.2  
19.497 style:line-style, Part 3, ODF 1.3

### table:formula, Host-dependent Properties

19.642 table:formula, Part 1, ODF 1.2  
19.646 table:formula, Part 3, ODF 1.3

The property `HOST_LOCALE` is implementation-defined.

### table:name in \<table:operation\>

19.673.11 \<table:operation\>, Part 1, ODF 1.2  
19.677.11 \<table:operation\>, Part 3, ODF 1.3

### table:protection-key-digest-algorithm

19.698 table:protection-key-digest-algorithm, Part 1, ODF 1.2  
19.702 table:protection-key-digest-algorithm, Part 3, ODF 1.3

The following algorithms are supported:

|  |  |
|----|----|
| Identifying IRI | algorithm |
| [http://www.w3.org/2000/09/xmldsig#sha1](http://www.w3.org/2000/09/xmldsig#sha1) | UTF16 LE/BE encoded StarOffice-SHA1, OOo legacy and allowed by ODF 1.1 |
| [http://www.w3.org/2000/09/xmldsig#sha1](http://www.w3.org/2000/09/xmldsig#sha1) | UTF8 encoded SHA1, as required by ODF 1.2 |
| [http://www.w3.org/2000/09/xmldsig#sha256](http://www.w3.org/2000/09/xmldsig#sha256) | UTF8 encoded SHA256, as required by ODF 1.2 |
| [http://www.w3.org/2001/04/xmlenc#sha256](http://www.w3.org/2001/04/xmlenc#sha256) | UTF8 encoded SHA256, with W3C URI |
| [http://docs.oasis-open.org/office/ns/table/legacy-hash-excel](http://docs.oasis-open.org/office/ns/table/legacy-hash-excel) combined with `loext:protection-key-digest-algorithm-2="`[`http://www.w3.org/2000/09/xmldsig#sha1`](http://www.w3.org/2000/09/xmldsig#sha1)`"` | Excel+SHA1 double-hash, see OFFICE-2112 [https://wiki.oasis-open.org/office/Spreadsheet_Table_Protection_Options](https://wiki.oasis-open.org/office/Spreadsheet_Table_Protection_Options) |

### table:show-empty

19.710 table:show-empty, Part 1, ODF 1.2  
19.714 table:show-empty, Part 3, ODF 1.3

### text:protection-key-digest-algorithm

19.851 text:protection-key-digest-algorithm, Part 1, ODF 1.2  
19.857 text:protection-key-digest-algorithm, Part 3, ODF 1.3

The following algorithms are supported:

|  |  |
|----|----|
| Identifying IRI | algorithm |
| [http://www.w3.org/2000/09/xmldsig#sha1](http://www.w3.org/2000/09/xmldsig#sha1) | UTF16 LE/BE encoded StarOffice-SHA1, OOo legacy and allowed by ODF 1.1 |
| [http://www.w3.org/2000/09/xmldsig#sha1](http://www.w3.org/2000/09/xmldsig#sha1) | UTF8 encoded SHA1, as required by ODF 1.2 |
| [http://www.w3.org/2000/09/xmldsig#sha256](http://www.w3.org/2000/09/xmldsig#sha256) | UTF8 encoded SHA256, as required by ODF 1.2 |
| [http://www.w3.org/2001/04/xmlenc#sha256](http://www.w3.org/2001/04/xmlenc#sha256) | UTF8 encoded SHA256, with W3C URI |

### chart:symbol-name

20.54 chart:symbol-name, Part 1, ODF 1.2  
20.61 chart:symbol-name, Part 3, ODF 1.3

### draw:visible-area-height

20.169 draw:visible-area-height, Part 1, ODF 1.2  
20.176 draw:visible-area-height, Part 3, ODF 1.3

For embedded objects that are internal to Collabora Office, the object itself
provides the visible area; for external embedded objects, the
draw:visible-area-height attribute is used.

### draw:visible-area-left

20.170 draw:visible-area-left, Part 1, ODF 1.2  
20.177 draw:visible-area-left, Part 3, ODF 1.3

For embedded objects that are internal to Collabora Office, the object itself
provides the visible area; for external embedded objects, the
`draw:visible-area-left` attribute is used.

### draw:visible-area-top

20.171 draw:visible-area-top, Part 1, ODF 1.2  
20.177 draw:visible-area-top, Part 3, ODF 1.3

For embedded objects that are internal to Collabora Office, the object itself
provides the visible area; for external embedded objects, the
`draw:visible-area-top` attribute is used.

### draw:visible-area-width

20.172 draw:visible-area-width, Part 1, ODF 1.2  
20.178 draw:visible-area-width, Part 3, ODF 1.3

For embedded objects that are internal to Collabora Office, the object itself
provides the visible area; for external embedded objects, the
`draw:visible-area-width` attribute is used.

### fo:wrap-option

20.223 fo:wrap-option, Part 1, ODF 1.2  
20.230 fo:wrap-option, Part 3, ODF 1.3

In cells in spreadsheet documents, text in a cell with wrapping disabled
is visible until the next non-empty cell starts.

In drawing objects, text with wrapping disabled is visible and the text
is displayed according to the object's `draw:textarea-horizontal-align`
and `fo:text-align` attributes.

In text boxes, text wrapping cannot be disabled.

### presentation:transition-speed

20.231 presentation:transition-speed, Part 1, ODF 1.2  
20.238 presentation:transition-speed, Part 3, ODF 1.3

|                                                  |      |
|--------------------------------------------------|------|
| Value of presentation:transition-speed attribute | rate |
| slow                                             | 2.0s |
| medium                                           | 1.0s |
| fast                                             | 0.5s |

### style:line-break

20.307 style:line-break, Part 1, ODF 1.2  
20.315 style:line-break, Part 3, ODF 1.3

### style:text-line-through-width

20.367 style:text-line-through-width, Part 1, ODF 1.2  
20.377 style:text-line-through-width, Part 3, ODF 1.3

### style:text-overline-width

20.373 style:text-overline-width, Part 1, ODF 1.2  
20.383 style:text-overline-width, Part 3, ODF 1.3

### style:text-underline-width

20.382 style:text-underline-width, Part 1, ODF 1.2  
20.392 style:text-underline-width, Part 3, ODF 1.3

### style:use-window-font-color

20.385 style:use-window-font-color, Part 1, ODF 1.2  
20.395 style:use-window-font-color, Part 3, ODF 1.3

Defined in
[Color::IsDark](https://cgit.collaboraoffice.com/c/online/tree/engine/tools/source/generic/color.cxx#n52)

`IsDark()` is `GetLuminance() <= 62`

`GetLuminace()` is `(Blue * 29 + Green * 151 + Red * 76) >> 8`

\[state 2021-08-21\]

### Host-Defined Behaviors

3.4 Host-Defined Behaviors, Part 2, ODF 1.2  
3.4 Host-Defined Behaviors, Part 4, ODF 1.3

### Types

#### Unicode Normaization

4.2 Text (String), Part 2, ODF 1.2  
4.2 Text (String), Part 4, ODF 1.3

#### Date

4.3.3 Date, Part 2, ODF 1.2  
4.3.3 Date, Part 4, ODF 1.3

#### Logical

4.3.7 Logical (Number), Part2, ODF 1.2

caution, new structure in ODF 1.3

#### Complex Number

4.4 Complex Number, Part 2, ODF 1.2  
4.4 Complex Number, Part 4, ODF 1.3

#### Processing a ReferenceList

4.11.12 Sequences (NumberSequence, NumberSequenceList, DateSequence,
LogicalSequence, and ComplexSequence), Part 2, ODF 1.2  
4.11.12 Sequences (NumberSequence, NumberSequenceList, DateSequence,
LogicalSequence, and ComplexSequence), Part 4, ODF 1.3

### Nonstandard Function Names

5.7 Nonstandard Function Names, Part 2, ODF 1.2  
5.7 Nonstandard Function Names, Part 4, ODF 1.3

See
[List_of_Collabora_OpenFormula_Extensions.md](List_of_Collabora_OpenFormula_Extensions.md)

### Common Template for Functions and Operators

6.2 Common Template for Functions and Operators, Part 4, ODF 1.3

The implementation-defined behavior is documented in the online-help
about functions.

For Collabora Office version n.k you access the help with
https://help.collaboraoffice.com/n.k/en-US/text/shared/05/new_help.html.
For example  
https://help.collaboraoffice.com/25.04/en-US/text/shared/05/new_help.html

### Standard Operators and Functions - Implicit Conversion Operators

#### Conversion to Number

6.3.5 Conversion to Number, Part 2, ODF 1.2  
6.3.5 Conversion to Number, Part 4, ODF 1.3

#### Conversion to Integer

Non-integer values are truncated to integer. If a function has a special
behavior in regard to a parameter, that requires data type Integer, its
behavior is described in the help of that function.

6.3.6 Conversion to Integer, Part 2, ODF 1.2  
6.3.6 Conversion to Integer, Part 4, ODF 1.3  
6.3.6 Conversion to Integer, Part 4, ODF 1.4

#### Conversion to Logic

6.3.12 Conversion to Logic, Part 2, ODF 1.2  
6.3.12 Conversion to Logic, Part 4, ODF 1.3

#### Conversion to DateParam

6.3.15 Conversion to DateParam, Part 2, ODF 1.2  
6.3.15 Conversion to DateParam, Part 4, ODF 1.3

#### Conversion to TimeParam

6.3.16 Conversion to TimeParam, Part 2, ODF 1.2  
6.3.16 Conversion to TimeParam, Part 4, ODF 1.3

### Standard Operators - Infix Operator Ordered Comparison

6.4.9 Infix Operator Ordered Comparison ("\<", "\<=", "\>", "\>="), Part
2, ODF 1.2  
6.4.9 Infix Operator Ordered Comparison ("\<", "\<=", "\>", "\>="),
Part4, ODF 1.3

### Bit operation functions

6.6.1 General, Bit operation functions, Part 2, ODF 1.2  
6.6.1 General, Bit operation functions, Part 4, ODF 1.3

### Functions

The implementation-defined behavior of functions is documented in the
online-help about the functions.

For Collabora Office version n.k you access the help with
https://help.collaboraoffice.com/n.k/en-US/text/shared/05/new_help.html.
For example  
https://help.collaboraoffice.com/25.04/en-US/text/shared/05/new_help.html

Affected functions are (in order of the specification): IMARGUMENT,
IMSUM, AMORLINC, COUNT, COUNTA, COUNTBLANK, FORMULA, ISEVEN, ISFORMULA,
ISODD, N, VALUE, ATAN2, GCD, POWER, MAX (ODF 1.2), MAXA (ODF 1.2), MIN,
MINA, STDEVA, STDEVPA, VARA, VARPA, BIN2DEC, BIN2HEX, BIN2OCT, DEC2BIN,
DEC2HEX, DEC2OCT, HEX2BIN, HEX2DEC, HEX2OCT, OCT2BIN, OCT2DEC, OCT2HEX,
CODE, TEXT

### implementation-defined IRI of an alternative algorithm

#### manifest:algorithm-name

4.16.1 manifest:algorithm-name, Part 2, ODF 1.3

No implementation defined values produced currently, only "Blowfish CFB"
and
"[http://www.w3.org/2001/04/xmlenc#aes256-cbc](http://www.w3.org/2001/04/xmlenc#aes256-cbc)".

#### manifest:checksum-type

4.16.3 manifest:checksum-type, Part 2, ODF 1.3

No implementation defined values produced currently, only "SHA1/1K" and
"[urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k](urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k)".

#### manifest:start-key-generation-name

4.16.6 manifest:start-key-generation-name, Part 2, ODF 1.3

No implementation defined values produced currently, only "SHA1" and
"[http://www.w3.org/2000/09/xmldsig#sha256](http://www.w3.org/2000/09/xmldsig#sha256)".

#### manifest:PGPAlgorithm

4.17 manifest:PGPAlgorithm, Part 2, ODF 1.3

[http://www.w3.org/2001/04/xmlenc#rsa-oaep-mgf1p](http://www.w3.org/2001/04/xmlenc#rsa-oaep-mgf1p)

see [\[1\]](https://www.w3.org/TR/xmlenc-core1/#sec-RSA-OAEP)

#### manifest:key-derivation-name

4.8.9 manifest:key-derivation-name, Part 3, ODF 1.2  
4.16.9 manifest:key-derivation-name, Part 2, ODF 1.3

- "[urn:org:documentfoundation:names:experimental:office:manifest:argon2id](urn:org:documentfoundation:names:experimental:office:manifest:argon2id)"

The Argon2id algorithm as specified by \[RFC9106\]. From section 3.1.
Argon2 Inputs and Outputs, the parameters "K" and "X" are not used. If
this value is used, the attributes `loext:argon2-iterations`,
`loext:argon2-memory`, and `loext:argon2-lanes` shall be present to provide
the "t", "m", and "p" parameters.

#### manifest:preferred-view-mode

4.8.11 manifest:preferred-view-mode, Part 3, ODF 1.2  
4.16.11 manifest:preferred-view-mode, Part 2, ODF 1.3

No implementation defined values supported (the attribute isn't
implemented at all).

#### loext:MetalMSCompatible

This is an implementation-defined value of the attribute
draw:extrusion-metal-type, which will be included in ODF 1.4, see
[https://issues.oasis-open.org/browse/OFFICE-4123](https://issues.oasis-open.org/browse/OFFICE-4123)

Calculations for specular color are done in HSB color model  
Specular color Hue = Material color Hue  
Specular color Saturation = Material color Saturation  
Specular color Brightness = Material color Brightness \* extrusion-specularity \* extrusion-first-light-level

The code is around
[https://cgit.collaboraoffice.com/c/online/tree/engine/svx/source/customshapes/EnhancedCustomShape3d.cxx#n1054](https://cgit.collaboraoffice.com/c/online/tree/engine/svx/source/customshapes/EnhancedCustomShape3d.cxx#n1054)

Evaluation of extrusion-shininess is the same as in case of
`extrusion-metal="false"`.

------------------------------------------------------------------------

## Footer

This page is based on a modified version of
[https://wiki.documentfoundation.org/index.php?title=Development/ODF_Implementer_Notes/List_of_LibreOffice_ODF_implementation-defined_items&oldid=872344](https://wiki.documentfoundation.org/index.php?title=Development/ODF_Implementer_Notes/List_of_LibreOffice_ODF_implementation-defined_items&oldid=872344)
and the Original Authors are: Olivier Hallot, Michael Stahl, Kirk Abbott, Ady and Regina Henschel.

This page is licensed under the [Creative Commons Attribution-ShareAlike 3.0 Unported License](https://creativecommons.org/licenses/by-sa/3.0/legalcode.en). The original work has been modified.


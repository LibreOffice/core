
---------------------------------------------------------------------

Summary of new features in RtfExport

---------------------------------------------------------------------

Miklos Vajna

<vmiklos@frugalware.org>
---------------------------------------------------------------------

Table of Contents

1. Introduction

    1.1. Terminology
    1.2. General

2. List if fixed bugs
3. List of new features

    3.1. Nested tables
    3.2. Character properties
    3.3. Sections
    3.4. Graphics
    3.5. Bookmarks
    3.6. Fields
    3.7. Drawing
    3.8. Form fields
    3.9. OLE objects

4. Changes in the source code outside RTF


---------------------------------------------------------------------

1.�Introduction

---------------------------------------------------------------------

The biggest difference is that the new exporter is an UNO component,
and it?s based on the MSWord base classes, the vision here is that
this way much less code can achieve the same set of features,
reducing the amount of duplicated code.


1.1.�Terminology

--------------

  * The "MSO OK, OOo KO" and similar abbreviations describe if the
    given new feature is supported by the OOo RTF importer or it can
    be tested using Microsoft Office.
  * RtfExport refers to the new UNO-based exporter, RtfWriter refers
    to the old built-in one.


1.2.�General

--------------

RtfWriter sometimes created documents where the first { is closed in
the middle of the document. MSO ignores this problem, but OOo stops
parsing the rest of the document if this happens, in other words
everything after such a bug is ignored. This can be reproduced by for
example parprops.odt, but it?s triggered in several other cases as
well. RtfExport has no automatic prevention for this, either - but
during development I primarily test the output with OOo, so hopefully
the bug will pop up less frequently.


---------------------------------------------------------------------

2.�List if fixed bugs

---------------------------------------------------------------------

  * http://www.openoffice.org/issues/show_bug.cgi?id=51469 postit
    fields
  * http://www.openoffice.org/issues/show_bug.cgi?id=66619 page
    margins
  * http://www.openoffice.org/issues/show_bug.cgi?id=69856 page
    numbers
  * http://www.openoffice.org/issues/show_bug.cgi?id=81569 { and } in
    document title
  * http://www.openoffice.org/issues/show_bug.cgi?id=84703 redlines
  * http://www.openoffice.org/issues/show_bug.cgi?id=91166 russian
    chars
  * http://www.openoffice.org/issues/show_bug.cgi?id=92673 bookmarks
    across tables
  * http://www.openoffice.org/issues/show_bug.cgi?id=100507 ole
    object export
  * http://www.openoffice.org/issues/show_bug.cgi?id=103993 same as #
    81569 just for doc comments
  * http://www.openoffice.org/issues/show_bug.cgi?id=106677
    listoverride index starts at zero
  * http://www.openoffice.org/issues/show_bug.cgi?id=38344 enhanced
    character space


---------------------------------------------------------------------

3.�List of new features

---------------------------------------------------------------------


3.1.�Nested tables

--------------

This was new in Word2000 and it?s now supported by RtfExport (MSO OK,
OOo KO)


3.2.�Character properties

--------------

The followings are now supported:

  * blinking (MSO OK, OOo KO)
  * expanded spacing (MSO OK, OOo OK)
  * pair kerning (MSO OK, OOo OK)


3.3.�Sections

--------------

RtfExport writes:

  * column breaks (MSO OK, OOo OK)
  * special breaks (when the next page should be an odd or an even
    page; MSO OK, OOo KO)
  * the write-protected property of sections is experted properly
    (MSO OK, OOo KO)
  * better page numbers (inherited type from page styles, restarts;
    MSO OK, OOo KO)
  * line numbering (MSO OK, OOo KO)


3.4.�Graphics

--------------

PNG graphics are exported in WMF format as well, so that not only MSO
and OOo can display graphics from the output document, but Wordpad as
well.


3.5.�Bookmarks

--------------

Implicit bookmarks like reference to a footnote did not work in OOo
(one got an Error: Reference source not found message when opening
the result), this now works as expected. (MSO OK - the importer
previously autocorrected this as well, OO OK)


3.6.�Fields

--------------

  * Table of contents is now written as a field, so it?s properly
    read-only (MSO OK, OOo KO)
  * Postit comments are now exported. (MSO OK, OOo KO)


3.7.�Drawing

--------------

Drawing objects for Word 97 through Word 2007 (shapes) are now
implemented:

  * basic shapes (rectangle, ellipse, etc.)
  * lines, including free-form ones
  * texts, including vertical ones and their (paragraph and
    character) formatting

(MSO OK, OOo KO)


3.8.�Form fields

--------------

All types supported by the RTF format are exported, namely:

  * text boxes
  * check boxes
  * list boxes

(MSO OK, OOo KO)


3.9.�OLE objects

--------------

Their result is exported as a picture - RtfWriter did not export
anything. (MSO OK, OOo OK)

For math, the native data is written as well, so you can edit the
object, too. (MSO OK, OOo KO)


---------------------------------------------------------------------

4.�Changes in the source code outside RTF

---------------------------------------------------------------------

These are refactorings I needed for RTF. To my best knowledge they do
not change the output of other filters from a user?s point of view.

  * The code that splits runs according to bookmarks is moved from
    DocxExport to MSWordExportBase
  * WW8_SdrAttrIter has been refactored to MSWord_SdrAttrIter
  * MSWordExportBase::SubstituteBullet can avoid replacing bullets
  * wwFontHelper::InitFontTable can really load all fonts
  * An obvious typo in WW8AttributeOutput::CharTwoLines has been
    fixed


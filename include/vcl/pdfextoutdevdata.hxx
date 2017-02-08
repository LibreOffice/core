/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_VCL_PDFEXTOUTDEVDATA_HXX
#define INCLUDED_VCL_PDFEXTOUTDEVDATA_HXX

#include <vcl/dllapi.h>

#include <vcl/pdfwriter.hxx>
#include <vcl/extoutdevdata.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/mapmod.hxx>
#include <vector>
#include <deque>

class Graphic;

namespace vcl { class PDFWriter; }

namespace vcl
{

/*
    A PDFExtOutDevBookmarkEntry is being created by the EditEngine if
    a bookmark URL has been parsed. The Application is requested to take
    care of each bookmark entry by emptying out the bookmark vector.
*/
struct PDFExtOutDevBookmarkEntry
{
    /** ID of the link pointing to the bookmark, or -1 if the entry denotes a destination instead of a link.
    */
    sal_Int32       nLinkId;

    /** ID of the named destination denoted by the bookmark, or -1 if the entry denotes a link instead of a named destination.
    */
    sal_Int32       nDestId;

    /** link target name, respectively destination name
    */
    OUString   aBookmark;

    PDFExtOutDevBookmarkEntry()
        :nLinkId( -1 )
        ,nDestId( -1 )
        ,aBookmark()
    {
    }
};

/*
 Class that is being set at the OutputDevice allowing the
 application to send enhanced PDF commands like CreateLink
*/
struct PageSyncData;
struct GlobalSyncData;
class VCL_DLLPUBLIC PDFExtOutDevData : public ExtOutDevData
{

    const OutputDevice&         mrOutDev;

    bool                        mbTaggedPDF;
    bool                        mbExportNotes;
    bool                        mbExportNotesPages;
    bool                        mbTransitionEffects;
    bool                        mbUseLosslessCompression;
    bool                        mbReduceImageResolution;
    bool                        mbExportFormFields;
    bool                        mbExportBookmarks;
    bool                        mbExportHiddenSlides;
    bool                        mbExportNDests; //i56629
    sal_Int32                   mnFormsFormat;
    sal_Int32                   mnPage;
    sal_Int32                   mnCompressionQuality;
    sal_Int32                   mnMaxImageResolution;
    css::lang::Locale           maDocLocale;

    PageSyncData*               mpPageSyncData;
    GlobalSyncData*             mpGlobalSyncData;

    std::vector< PDFExtOutDevBookmarkEntry > maBookmarks;

public:

    PDFExtOutDevData( const OutputDevice& rOutDev );
    virtual ~PDFExtOutDevData() override;

    bool PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rCurGDIMtfAction );
    void ResetSyncData();

    void PlayGlobalActions( PDFWriter& rWriter );

    bool    GetIsExportNotes() const { return mbExportNotes;}
    void        SetIsExportNotes( const bool bExportNotes );

    bool    GetIsExportNotesPages() const { return mbExportNotesPages;}
    void        SetIsExportNotesPages( const bool bExportNotesPages );

    bool    GetIsExportTaggedPDF() const { return mbTaggedPDF;}
    void        SetIsExportTaggedPDF( const bool bTaggedPDF );

    bool    GetIsExportTransitionEffects() const { return mbTransitionEffects;}
    void        SetIsExportTransitionEffects( const bool bTransitionalEffects );

    bool    GetIsExportFormFields() const { return mbExportFormFields;}
    void        SetIsExportFormFields( const bool bExportFormFields );

    void        SetFormsFormat( const sal_Int32 nFormsFormat );

    bool    GetIsExportBookmarks() const { return mbExportBookmarks;}
    void        SetIsExportBookmarks( const bool bExportBookmarks );

    bool    GetIsExportHiddenSlides() const { return mbExportHiddenSlides;}
    void        SetIsExportHiddenSlides( const bool bExportHiddenSlides );

    bool    GetIsExportNamedDestinations() const { return mbExportNDests;} //i56629
    void        SetIsExportNamedDestinations( const bool bExportNDests ); //i56629

    // PageNumber, Compression is being set by the PDFExport
    sal_Int32   GetCurrentPageNumber() const { return mnPage;}
    void        SetCurrentPageNumber( const sal_Int32 nPage );

    bool        GetIsLosslessCompression() const { return mbUseLosslessCompression;}
    void        SetIsLosslessCompression( const bool bLosslessCompression );

    void        SetCompressionQuality( const sal_Int32 nQuality );

    void        SetMaxImageResolution( const sal_Int32 nQuality );

    bool        GetIsReduceImageResolution() const { return mbReduceImageResolution;}
    void        SetIsReduceImageResolution( const bool bReduceImageResolution );

    const css::lang::Locale& GetDocumentLocale() const { return maDocLocale;}
    void        SetDocumentLocale( const css::lang::Locale& rLoc );

    std::vector< PDFExtOutDevBookmarkEntry >& GetBookmarks() { return maBookmarks;}

    const Graphic& GetCurrentGraphic() const;

    /** Start a new group of render output

        Use this method to group render output.
     */
    void        BeginGroup();

    /** End render output

        This method ends grouped render output, that can be
        represented by a GfxLink. This is typically used for
        external graphic files, such as JPEGs, EPS files etc.
        The BeginGroup/EndGroup calls must exactly enclose the
        relevant OutputDevice calls issued to render the
        graphic the normal way.

        @param rGraphic
        The link to the original graphic

        @param nTransparency
        Eight bit transparency value, with 0 denoting full opacity,
        and 255 full transparency.

        @param rOutputRect
        The output rectangle of the graphic.

        @param rVisibleOutputRect
        The visible part of the output. This might be less than
        rOutputRect, e.g. for cropped graphics.
     */
    void        EndGroup( const Graphic&    rGraphic,
                          sal_uInt8         nTransparency,
                          const Rectangle&  rOutputRect,
                          const Rectangle&  rVisibleOutputRect );

    /// Detect if stream is compressed enough to avoid de-compress / scale & re-compress
    bool        HasAdequateCompression( const Graphic &rGraphic,
                                        const Rectangle &rOutputRect,
                                        const Rectangle &rVisibleOutputRect ) const;

//--->i56629
    /** Create a new named destination to be used in a link to this document from another PDF document
 (see PDF spec 1.4, 8.2.1)

    @param sDestName
    the name this destination will be addressed with from others PDF document

    @param rRect
    target rectangle on page to be displayed if dest is jumped to

    @param nPageNr
    number of page the dest is on (as returned by NewPage)
    or -1 in which case the current page is used

    @returns
    the destination id (to be used in SetLinkDest) or
    -1 if page id does not exist
    */
    sal_Int32 CreateNamedDest( const OUString& sDestName,  const Rectangle& rRect, sal_Int32 nPageNr = -1 );

    /** registers a destination for which a destination ID needs to be known immediately, instead of later on setting it via
        SetLinkDest.

        This is used in contexts where a destination is referenced by means other than a link.

        Later in the export process, a call to DescribeRegisteredDest must be made, providing the information about
        the destination.

        @return
            the unique Id of the destination
    */
    sal_Int32   RegisterDest();

    /** provides detailed information about a destination range which previously has been registered using RegisterDest.
    */
    void        DescribeRegisteredDest( sal_Int32 nDestId, const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType = PDFWriter::DestAreaType::XYZ );

//<---i56629

    /** Create a new destination to be used in a link

    @param rRect
    target rectangle on page to be displayed if dest is jumped to

    @param nPageNr
    number of page the dest is on (as returned by NewPage)
    or -1 in which case the current page is used

    @param eType
    what dest type to use

    @returns
    the destination id (to be used in SetLinkDest) or
    -1 if page id does not exist
    */
    sal_Int32 CreateDest( const Rectangle& rRect, sal_Int32 nPageNr = -1, PDFWriter::DestAreaType eType = PDFWriter::DestAreaType::XYZ );
    /** Create a new link on a page

    @param rRect
    active rectangle of the link (that is the area that has to be
    hit to activate the link)

    @param nPageNr
    number of page the link is on (as returned by NewPage)
    or -1 in which case the current page is used

    @returns
    the link id (to be used in SetLinkDest, SetLinkURL) or
    -1 if page id does not exist
    */
    sal_Int32 CreateLink( const Rectangle& rRect, sal_Int32 nPageNr = -1 );

    /// Create a Screen annotation.
    sal_Int32 CreateScreen(const Rectangle& rRect, sal_Int32 nPageNr);

    /** Set the destination for a link
        <p>will change a URL type link to a dest link if necessary</p>

        @param nLinkId
        the link to be changed

        @param nDestId
        the dest the link shall point to
        @returns
        0 for success
        -1 in case the link id does not exist
        -2 in case the dest id does not exist
    */
    sal_Int32 SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId );
    /** Set the URL for a link
        <p>will change a dest type link to an URL type link if necessary</p>
        @param nLinkId
        the link to be changed

        @param rURL
        the URL the link shall point to.
        there will be no error checking or any kind of
        conversion done to this parameter execept this:
        it will be output as 7bit Ascii. The URL
        will appear literally in the PDF file produced

        @returns
        0 for success
        -1 in case the link id does not exist
    */
    sal_Int32 SetLinkURL( sal_Int32 nLinkId, const OUString& rURL );

    /// Set URL for a linked Screen annotation.
    void SetScreenURL(sal_Int32 nScreenId, const OUString& rURL);
    /// Set URL for an embedded Screen annotation.
    void SetScreenStream(sal_Int32 nScreenId, const OUString& rURL);

    /** Create a new outline item

        @param nParent
        declares the parent of the new item in the outline hierarchy.
        An invalid value will result in a new toplevel item.

        @param rText
        sets the title text of the item

        @param nDestID
        declares which Dest (created with CreateDest) the outline item
        will point to

        @returns
        the outline item id of the new item
    */
    sal_Int32 CreateOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID );

    /** Create a new note on a page

    @param rRect
    active rectangle of the note (that is the area that has to be
    hit to popup the annotation)

    @param rNote
    specifies the contents of the note

    @param nPageNr
    number of page the note is on (as returned by NewPage)
    or -1 in which case the current page is used
    */
    void CreateNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr = -1 );

    /** begin a new logical structure element

    BeginStructureElement/EndStructureElement calls build the logical structure
    of the PDF - the basis for tagged PDF. Structural elements are implemented
    using marked content tags. Each structural element can contain sub elements
    (e.g. a section can contain a heading and a paragraph). The structure hierarchy
    is build automatically from the Begin/EndStructureElement calls.

    A structural element need not be contained on one page; e.g. paragraphs often
    run from one page to the next. In this case the corresponding EndStructureElement
    must be called while drawing the next page.

    BeginStructureElement and EndStructureElement must be called only after
    PDFWriter::NewPage has been called and before
    PDFWriter::Emit gets called. The current page
    number is an implicit context parameter for Begin/EndStructureElement.

    For pagination artifacts that are not part of the logical structure
    of the document (like header, footer or page number) the special
    StructElement NonStructElement exists. To place content
    outside of the structure tree simply call
    BeginStructureElement( NonStructElement ) then draw your
    content and then call EndStructureElement(). Any children
    of a NonStructElement will not be part of the structure as well.

    @param eType
    denotes what kind of element to begin (e.g. a heading or paragraph)

    @param rAlias
    the specified alias will be used as structure tag. Also an entry in the PDF's
    role map will be created mapping alias to regular structure type.

    @returns
    the id of the newly created structural element
     */
     sal_Int32 BeginStructureElement( PDFWriter::StructElement eType, const OUString& rAlias = OUString() );
    /** end a logical structure element

    @see BeginStructureElement
     */
    void EndStructureElement();
    /** set the current structure element

    <p>
    For different purposes it may be useful to paint a structure element's
    content discontinously. In that case an already existing structure element
    can be appended to by using SetCurrentStructureElement. The
    refenrenced structure element becomes the current structure element with
    all consequences: all following structure elements are appended as children
    of the current element.
    </p>

    @param nElement
    the id of the new current structure element

    @returns
    True if the current structure element could be set successfully
    False if the current structure element could not be changed
    (e.g. if the passed element id is invalid)
     */
    bool SetCurrentStructureElement( sal_Int32 nElement );
    /** get the current structure element id

    @returns
    the id of the current structure element
    */
    sal_Int32 GetCurrentStructureElement();

    /** set a structure attribute on the current structural element

    SetStructureAttribute sets an attribute of the current structural element to a
    new value. A consistency check is performed before actually setting the value;
    if the check fails, the function returns False and the attribute remains
    unchanged.

    @param eAttr
    denotes what attribute to change

    @param eVal
    the value to set the attribute to

    @returns
    True if the value was valid and the change has been performed,
    False if the attribute or value was invalid; attribute remains unchanged
     */
    bool SetStructureAttribute( PDFWriter::StructAttribute eAttr, PDFWriter::StructAttributeValue eVal );
    /** set a structure attribute on the current structural element

    SetStructureAttributeNumerical sets an attribute of the current structural element
    to a new numerical value. A consistency check is performed before actually setting
    the value; if the check fails, the function returns False and the attribute
    remains unchanged.

    @param eAttr
    denotes what attribute to change

    @param nValue
    the value to set the attribute to

    @returns
    True if the value was valid and the change has been performed,
    False if the attribute or value was invalid; attribute remains unchanged
     */
    bool SetStructureAttributeNumerical( PDFWriter::StructAttribute eAttr, sal_Int32 nValue );
    /** set the bounding box of a structural element

    SetStructureBoundingBox sets the BBox attribute to a new value. Since the BBox
    attribute can only be applied to Table, Figure,
    Form and Formula elements, a call of this function
    for other element types will be ignored and the BBox attribute not be set.

    @param rRect
    the new bounding box for the structural element
     */
    void SetStructureBoundingBox( const Rectangle& rRect );

    /** set the ActualText attribute of a structural element

    ActualText contains the Unicode text without layout artifacts that is shown by
    a structural element. For example if a line is ended prematurely with a break in
    a word and continued on the next line (e.g. "happen-<newline>stance") the
    corresponding ActualText would contain the unbroken line (e.g. "happenstance").

    @param rText
    contains the complete logical text the structural element displays.
     */
    void SetActualText( const OUString& rText );

    /** set the Alt attribute of a strutural element

    Alt is s replacement text describing the contents of a structural element. This
    is mainly used by accessibility applications; e.g. a screen reader would read
    the Alt replacement text for an image to a visually impaired user.

    @param rText
    contains the replacement text for the structural element
    */
    void SetAlternateText( const OUString& rText );

    /** Sets the transitional effect to be applied when the current page gets shown.

    @param eType
    the kind of effect to be used; use Regular to disable transitional effects
    for this page

    @param nMilliSec
    the duration of the transitional effect in milliseconds;
    set 0 to disable transitional effects
    */
    void SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec );

    /** create a new form control

    This function creates a new form control in the PDF and sets its various
    properties. Do not pass an actual AnyWidget as rControlType
    will be cast to the type described by the type member.

    @param rControlType
    a descendant of AnyWidget determining the control's properties
     */
    void    CreateControl( const PDFWriter::AnyWidget& rControlType );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xeescher.hxx,v $
 * $Revision: 1.10.128.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_XEESCHER_HXX
#define SC_XEESCHER_HXX

#include <vcl/graph.hxx>
#include "xcl97rec.hxx"
#include "xlescher.hxx"

namespace com { namespace sun { namespace star {
    namespace script { struct ScriptEventDescriptor; }
} } }

// ============================================================================

/** Provides export of bitmap data to an IMGDATA record. */
class XclExpImgData : public XclExpRecordBase
{
public:
    explicit            XclExpImgData( const Graphic& rGraphic, sal_uInt16 nRecId );

    /** Writes the BITMAP record. */
    virtual void        Save( XclExpStream& rStrm );

private:
    Graphic             maGraphic;      /// The VCL graphic.
    sal_uInt16          mnRecId;        /// Record identifier for the IMGDATA record.
};

// ============================================================================

/** Helper class for form controils to manage spreadsheet links . */
class XclExpControlHelper : protected XclExpRoot
{
public:
    explicit            XclExpControlHelper( const XclExpRoot& rRoot );
    virtual             ~XclExpControlHelper();

protected:
    /** Tries to get spreadsheet cell link and source range link from the passed shape. */
    void                ConvertSheetLinks(
                            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );


    /** Returns the Excel token array of the cell link, or 0, if no link present. */
    inline const XclTokenArray* GetCellLinkTokArr() const { return mxCellLink.get(); }
    /** Returns the Excel token array of the source range, or 0, if no link present. */
    inline const XclTokenArray* GetSourceRangeTokArr() const { return mxSrcRange.get(); }
    /** Returns the number of entries in the source range, or 0, if no source set. */
    inline sal_uInt16   GetSourceEntryCount() const { return mnEntryCount; }

    /** Writes a formula with special style only valid in OBJ records. */
    void                WriteFormula( XclExpStream& rStrm, const XclTokenArray& rTokArr ) const;
    /** Writes a formula subrecord with special style only valid in OBJ records. */
    void                WriteFormulaSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId, const XclTokenArray& rTokArr ) const;

private:
    XclTokenArrayRef    mxCellLink;     /// Formula for linked cell.
    XclTokenArrayRef    mxSrcRange;     /// Formula for source data range.
    sal_uInt16          mnEntryCount;   /// Number of entries in source range.
};

// ----------------------------------------------------------------------------

#if EXC_EXP_OCX_CTRL

/** Represents an OBJ record for an OCX form control. */
class XclExpOcxControlObj : public XclObj, public XclExpControlHelper
{
public:
    explicit            XclExpOcxControlObj(
                            const XclExpRoot& rRoot,
                            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape,
                            const String& rClassName,
                            sal_uInt32 nStrmStart, sal_uInt32 nStrmSize );

private:
    virtual void        WriteSubRecs( XclExpStream& rStrm );

private:
    String              maClassName;        /// Class name of the control.
    sal_uInt32          mnStrmStart;        /// Start position in 'Ctls' stream.
    sal_uInt32          mnStrmSize;         /// Size in 'Ctls' stream.
};

#else

/** Represents an OBJ record for an TBX form control. */
class XclExpTbxControlObj : public XclObj, public XclExpControlHelper
{
public:
    explicit            XclExpTbxControlObj(
                            const XclExpRoot& rRoot,
                            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );

    /** Sets the name of a macro attached to this control.
        @return  true = The passed event descriptor was valid, macro name has been found. */
    bool                SetMacroLink( const ::com::sun::star::script::ScriptEventDescriptor& rEvent );

private:
    virtual void        WriteSubRecs( XclExpStream& rStrm );

    /** Writes an ftMacro subrecord containing a macro link, or nothing, if no macro present. */
    void                WriteMacroSubRec( XclExpStream& rStrm );
    /** Writes a subrecord containing a cell link, or nothing, if no link present. */
    void                WriteCellLinkSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId );
    /** Writes the ftSbs sub structure containing scrollbar data. */
    void                WriteSbs( XclExpStream& rStrm );

private:
    ScfInt16Vec         maMultiSel;     /// Indexes of all selected entries in a multi selection.
    XclTokenArrayRef    mxMacroLink;    /// Token array containing a link to an attached macro.
    XclTbxEventType     meEventType;    /// Type of supported macro event.
    sal_Int32           mnHeight;       /// Height of the control.
    sal_uInt16          mnState;        /// Checked/unchecked state.
    sal_Int16           mnLineCount;    /// Combobox dropdown line count.
    sal_Int16           mnSelEntry;     /// Selected entry in combobox (1-based).
    sal_uInt16          mnScrollValue;  /// Scrollbar: Current value.
    sal_uInt16          mnScrollMin;    /// Scrollbar: Minimum value.
    sal_uInt16          mnScrollMax;    /// Scrollbar: Maximum value.
    sal_uInt16          mnScrollStep;   /// Scrollbar: Single step.
    sal_uInt16          mnScrollPage;   /// Scrollbar: Page step.
    bool                mbFlatButton;   /// False = 3D button style; True = Flat button style.
    bool                mbFlatBorder;   /// False = 3D border style; True = Flat border style.
    bool                mbMultiSel;     /// true = Multi selection in listbox.
    bool                mbScrollHor;    /// Scrollbar: true = horizontal.
};

#endif

// ----------------------------------------------------------------------------

class XclExpChart;

/** A chart object. This is the drawing object wrapper for the chart data. */
class XclExpChartObj : public XclObj, protected XclExpRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > XShapeRef;

public:
    explicit            XclExpChartObj( const XclExpRoot& rRoot, XShapeRef xShape );
    virtual             ~XclExpChartObj();

    /** Writes the OBJ record and the entire chart substream. */
    virtual void        Save( XclExpStream& rStrm );

private:
    typedef ScfRef< XclExpChart > XclExpChartRef;
    XclExpChartRef      mxChart;        /// The chart itself (BOF/EOF substream data).
};

// ============================================================================

/** Represents a NOTE record containing the relevant data of a cell note.

    NOTE records differ significantly in various BIFF versions. This class
    encapsulates all needed actions for each supported BIFF version.
    BIFF5/BIFF7: Stores the note text and generates a single or multiple NOTE
    records on saving.
    BIFF8: Creates the Escher object containing the drawing information and the
    note text.
 */
class XclExpNote : public XclExpRecord
{
public:
    /** Constructs a NOTE record from the passed note object and/or the text.
        @descr  The additional text will be separated from the note text with
            an empty line.
        @param rScPos  The Calc cell address of the note.
        @param pScNote  The Calc note object. May be 0 to create a note from rAddText only.
        @param rAddText  Additional text appended to the note text. */
    explicit            XclExpNote(
                            const XclExpRoot& rRoot,
                            const ScAddress& rScPos,
                            const ScPostIt* pScNote,
                            const String& rAddText );

    /** Writes the NOTE record, if the respective Escher object is present. */
    virtual void        Save( XclExpStream& rStrm );

    void                WriteXml( sal_Int32 nAuthorId, XclExpXmlStream& rStrm );

    const XclExpString& GetAuthor() const { return maAuthor; }
private:
    /** Writes the body of the NOTE record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    XclExpString        maAuthor;       /// Name of the author.
    String              maOrigNoteText; /// Original main text of the note.
    ByteString          maNoteText;     /// Main text of the note (<=BIFF7).
    ScAddress           maScPos;        /// Calc cell address of the note.
    sal_uInt16          mnObjId;        /// Escher object ID (BIFF8).
    bool                mbVisible;      /// true = permanently visible.
};

// ============================================================================

class XclExpComments : public XclExpRecord
{
public:
    typedef XclExpRecordList< XclExpNote >
                        XclExpNoteList;

                        XclExpComments( SCTAB nTab, XclExpNoteList& rNotes );

    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    SCTAB               mnTab;
    XclExpNoteList&     mrNotes;
};

// object manager =============================================================

class XclExpObjectManager : protected XclExpRoot
{
public:
    explicit            XclExpObjectManager( const XclExpRoot& rRoot );
    virtual             ~XclExpObjectManager();

    inline XclEscherEx& GetEx() { return *mxEx; }
    inline SvStream&    GetStrm() { return maDffStrm; }

    void                AddSdrPage();

private:
    SvMemoryStream      maDffStrm;
    ::std::auto_ptr< XclEscherEx > mxEx;
};

// ============================================================================

#endif


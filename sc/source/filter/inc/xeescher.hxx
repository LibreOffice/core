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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XEESCHER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XEESCHER_HXX

#include <vcl/graph.hxx>
#include <filter/msfilter/escherex.hxx>
#include "xcl97rec.hxx"
#include "xlescher.hxx"
#include "xlformula.hxx"
#include <svx/sdtaitm.hxx>
#include <rtl/ustring.hxx>
#include <memory>

class ScPostIt;

namespace utl { class TempFile; }
namespace com { namespace sun { namespace star { namespace chart { class XChartDocument; } } } }

namespace com { namespace sun { namespace star {
    namespace script { struct ScriptEventDescriptor; }
} } }

// DFF client anchor ==========================================================

/** Base class for DFF client anchor atoms used in spreadsheets. */
class XclExpDffAnchorBase : public EscherExClientAnchor_Base, protected XclExpRoot
{
public:
    /** Constructs a dummy client anchor. */
    explicit            XclExpDffAnchorBase( const XclExpRoot& rRoot, sal_uInt16 nFlags = 0 );

    /** Sets the flags according to the passed SdrObject. */
    void                SetFlags( const SdrObject& rSdrObj );
    /** Sets the anchor position and flags according to the passed SdrObject. */
    void                SetSdrObject( const SdrObject& rSdrObj );

    /** Writes the DFF client anchor structure with the current anchor position. */
    void                WriteDffData( EscherEx& rEscherEx ) const;

    /** Called from SVX DFF converter.
        @param rRect  The object anchor rectangle to be exported (in twips). */
    virtual void        WriteData( EscherEx& rEscherEx, const tools::Rectangle& rRect ) override;

private:
    virtual void        ImplSetFlags( const SdrObject& rSdrObj );
    virtual void        ImplCalcAnchorRect( const tools::Rectangle& rRect, MapUnit eMapUnit );

protected:  // for access in derived classes
    XclObjAnchor        maAnchor;       /// The client anchor data.
    sal_uInt16          mnFlags;        /// Flags for DFF stream export.
};

/** Represents the position (anchor) of an object in a Calc sheet. */
class XclExpDffSheetAnchor : public XclExpDffAnchorBase
{
public:
    explicit            XclExpDffSheetAnchor( const XclExpRoot& rRoot );

private:
    virtual void        ImplSetFlags( const SdrObject& rSdrObj ) override;
    virtual void        ImplCalcAnchorRect( const tools::Rectangle& rRect, MapUnit eMapUnit ) override;

private:
    SCTAB const         mnScTab;        /// Calc sheet index.
};

/** Represents the position (anchor) of a shape in an embedded draw page. */
class XclExpDffEmbeddedAnchor : public XclExpDffAnchorBase
{
public:
    explicit            XclExpDffEmbeddedAnchor( const XclExpRoot& rRoot,
                            const Size& rPageSize, sal_Int32 nScaleX, sal_Int32 nScaleY );

private:
    virtual void        ImplSetFlags( const SdrObject& rSdrObj ) override;
    virtual void        ImplCalcAnchorRect( const tools::Rectangle& rRect, MapUnit eMapUnit ) override;

private:
    Size const          maPageSize;
    sal_Int32 const     mnScaleX;
    sal_Int32 const     mnScaleY;
};

/** Represents the position (anchor) of a note object. */
class XclExpDffNoteAnchor : public XclExpDffAnchorBase
{
public:
    explicit            XclExpDffNoteAnchor( const XclExpRoot& rRoot, const tools::Rectangle& rRect );
};

/** Represents the position (anchor) of a cell dropdown object. */
class XclExpDffDropDownAnchor : public XclExpDffAnchorBase
{
public:
    explicit            XclExpDffDropDownAnchor( const XclExpRoot& rRoot, const ScAddress& rScPos );
};

// MSODRAWING* records ========================================================

/** Base class for records holding DFF stream fragments. */
class XclExpMsoDrawingBase : public XclExpRecord
{
public:
    explicit            XclExpMsoDrawingBase( XclEscherEx& rEscherEx, sal_uInt16 nRecId );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

protected:
    XclEscherEx&        mrEscherEx;         /// Reference to the DFF converter containing the DFF stream.
    sal_uInt32 const    mnFragmentKey;      /// The key of the DFF stream fragment to be written by this record.
};

/** The MSODRAWINGGROUP record contains the DGGCONTAINER with global DFF data
    such as the picture container.
 */
class XclExpMsoDrawingGroup : public XclExpMsoDrawingBase
{
public:
    explicit            XclExpMsoDrawingGroup( XclEscherEx& rEscherEx );
};

/** One or more MSODRAWING records contain the DFF stream data for a drawing
    shape.
 */
class XclExpMsoDrawing : public XclExpMsoDrawingBase
{
public:
    explicit            XclExpMsoDrawing( XclEscherEx& rEscherEx );
};

/** Provides export of bitmap data to an IMGDATA record. */
class XclExpImgData : public XclExpRecordBase
{
public:
    explicit            XclExpImgData( const Graphic& rGraphic, sal_uInt16 nRecId );

    /** Writes the BITMAP record. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    Graphic const       maGraphic;      /// The VCL graphic.
    sal_uInt16 const    mnRecId;        /// Record identifier for the IMGDATA record.
};

/** Helper class for form controls to manage spreadsheet links . */
class XclExpControlHelper : protected XclExpRoot
{
public:
    explicit            XclExpControlHelper( const XclExpRoot& rRoot );
    virtual             ~XclExpControlHelper() override;

protected:
    /** Tries to get spreadsheet cell link and source range link from the passed shape. */
    void                ConvertSheetLinks(
                            css::uno::Reference< css::drawing::XShape > const & xShape );

    /** Returns the Excel token array of the cell link, or 0, if no link present. */
    const XclTokenArray* GetCellLinkTokArr() const { return mxCellLink.get(); }
    /** Returns the Excel token array of the source range, or 0, if no link present. */
    const XclTokenArray* GetSourceRangeTokArr() const { return mxSrcRange.get(); }
    /** Returns the number of entries in the source range, or 0, if no source set. */
    sal_uInt16   GetSourceEntryCount() const { return mnEntryCount; }

    /** Writes a formula with special style only valid in OBJ records. */
    static void         WriteFormula( XclExpStream& rStrm, const XclTokenArray& rTokArr );
    /** Writes a formula subrecord with special style only valid in OBJ records. */
    static void         WriteFormulaSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId, const XclTokenArray& rTokArr );

private:
    XclTokenArrayRef    mxCellLink;     /// Formula for linked cell.
    XclTokenArrayRef    mxSrcRange;     /// Formula for source data range.
    sal_uInt16          mnEntryCount;   /// Number of entries in source range.
};

class XclMacroHelper : public XclExpControlHelper
{
    XclTokenArrayRef    mxMacroLink;    /// Token array containing a link to an attached macro.

public:
    explicit            XclMacroHelper( const XclExpRoot& rRoot );
    virtual             ~XclMacroHelper() override;
    /** Writes an ftMacro subrecord containing a macro link, or nothing, if no macro present. */
    void                WriteMacroSubRec( XclExpStream& rStrm  );
    /** Sets the name of a macro for object of passed type
        @return  true = The passed event descriptor was valid, macro name has been found. */
    bool                SetMacroLink( const css::script::ScriptEventDescriptor& rEvent,  const XclTbxEventType& nEventType );

    /** Sets the name of a macro
        @return  true = The passed macro name has been found. */
    bool                SetMacroLink( const OUString& rMacro );
};

class XclExpShapeObj : public XclObjAny, public XclMacroHelper
{
public:
    explicit            XclExpShapeObj( XclExpObjectManager& rRoot, css::uno::Reference< css::drawing::XShape > const & xShape, ScDocument* pDoc );
    virtual             ~XclExpShapeObj() override;
private:
    virtual void        WriteSubRecs( XclExpStream& rStrm ) override;
};

//delete for exporting OCX
//#if EXC_EXP_OCX_CTRL

/** Represents an OBJ record for an OCX form control. */
class XclExpOcxControlObj : public XclObj, public XclExpControlHelper
{
public:
    explicit            XclExpOcxControlObj(
                            XclExpObjectManager& rObjMgr,
                            css::uno::Reference< css::drawing::XShape > const & xShape,
                            const tools::Rectangle* pChildAnchor,
                            const OUString& rClassName,
                            sal_uInt32 nStrmStart, sal_uInt32 nStrmSize );

private:
    virtual void        WriteSubRecs( XclExpStream& rStrm ) override;

private:
    OUString const      maClassName;        /// Class name of the control.
    sal_uInt32          mnStrmStart;        /// Start position in 'Ctls' stream.
    sal_uInt32          mnStrmSize;         /// Size in 'Ctls' stream.
};

//#else

/** Represents an OBJ record for an TBX form control. */
class XclExpTbxControlObj : public XclObj, public XclMacroHelper
{
public:
    explicit            XclExpTbxControlObj(
                            XclExpObjectManager& rObjMgr,
                            css::uno::Reference< css::drawing::XShape > const & xShape,
                            const tools::Rectangle* pChildAnchor );

    /** Sets the name of a macro attached to this control.
        @return  true = The passed event descriptor was valid, macro name has been found. */
    bool                SetMacroLink( const css::script::ScriptEventDescriptor& rEvent );

private:
    virtual void        WriteSubRecs( XclExpStream& rStrm ) override;

    /** Writes a subrecord containing a cell link, or nothing, if no link present. */
    void                WriteCellLinkSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId );
    /** Writes the ftSbs sub structure containing scrollbar data. */
    void                WriteSbs( XclExpStream& rStrm );

private:
    ScfInt16Vec         maMultiSel;     /// Indexes of all selected entries in a multi selection.
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

//#endif

class XclExpChart;

/** A chart object. This is the drawing object wrapper for the chart data. */
class XclExpChartObj : public XclObj, protected XclExpRoot
{
public:
    explicit            XclExpChartObj(
                            XclExpObjectManager& rObjMgr,
                            css::uno::Reference< css::drawing::XShape > const & xShape,
                            const tools::Rectangle* pChildAnchor );
    virtual             ~XclExpChartObj() override;

    /** Writes the OBJ record and the entire chart substream. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

    const css::uno::Reference<css::chart::XChartDocument>& GetChartDoc() const;

private:
    typedef std::shared_ptr< XclExpChart > XclExpChartRef;
    XclExpChartRef                                    mxChart;        /// The chart itself (BOF/EOF substream data).
    css::uno::Reference< css::drawing::XShape >       mxShape;
    css::uno::Reference< css::chart::XChartDocument > mxChartDoc;
};

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
                            const OUString& rAddText );

    /** Writes the NOTE record, if the respective Escher object is present. */
    virtual void        Save( XclExpStream& rStrm ) override;

    void                WriteXml( sal_Int32 nAuthorId, XclExpXmlStream& rStrm );

    const XclExpString& GetAuthor() const { return maAuthor; }
private:
    /** Writes the body of the NOTE record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclExpString        maAuthor;       /// Name of the author.
    OString             maNoteText;     /// Main text of the note (<=BIFF7).
    XclExpStringRef     mpNoteContents; /// Text and formatting data (OOXML)
    ScAddress           maScPos;        /// Calc cell address of the note.
    sal_uInt16          mnObjId;        /// Escher object ID (BIFF8).
    bool                mbVisible;      /// true = permanently visible.
    SdrTextHorzAdjust   meTHA;          /// text horizontal adjust
    SdrTextVertAdjust   meTVA;          /// text vertical adjust
    bool                mbAutoScale;    /// Auto scale text
    bool                mbLocked;       /// Position & Size locked
    bool                mbAutoFill;     /// Auto Fill Style
    bool                mbColHidden;    /// Column containing the comment is hidden
    bool                mbRowHidden;    /// Row containing the comment is hidden
    tools::Rectangle           maCommentFrom;  /// From and From Offset
    tools::Rectangle           maCommentTo;    /// To and To Offsets
};

class XclExpComments : public XclExpRecord
{
public:
                        XclExpComments( SCTAB nTab, XclExpRecordList< XclExpNote >& rNotes );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    SCTAB const         mnTab;
    XclExpRecordList< XclExpNote >& mrNotes;
};

// object manager =============================================================

class XclExpObjectManager : public XclExpRoot
{
public:
    explicit            XclExpObjectManager( const XclExpRoot& rRoot );
    virtual             ~XclExpObjectManager() override;

    /** Creates a new DFF client anchor object. Caller takes ownership! May be
        overwritten in derived  classes. */
    virtual XclExpDffAnchorBase* CreateDffAnchor() const;

    /** Creates and returns the MSODRAWINGGROUP record containing global DFF
        data in the DGGCONTAINER. */
    std::shared_ptr< XclExpRecordBase > CreateDrawingGroup();

    /** Initializes the object manager for a new sheet. */
    void                StartSheet();

    /** Processes a drawing page and returns the record block containing all
        related records (MSODRAWING, OBJ, TXO, charts, etc.). */
    std::shared_ptr< XclExpRecordBase > ProcessDrawing( const SdrPage* pSdrPage );
    /** Processes a collection of UNO shapes and returns the record block
        containing all related records (MSODRAWING, OBJ, TXO, charts, etc.). */
    std::shared_ptr< XclExpRecordBase > ProcessDrawing( const css::uno::Reference< css::drawing::XShapes >& rxShapes );

    /** Finalizes the object manager after conversion of all sheets. */
    void                EndDocument();

    XclEscherEx& GetEscherEx() { return *mxEscherEx; }
    XclExpMsoDrawing*   GetMsodrawingPerSheet();
    bool                HasObj() const;
    sal_uInt16          AddObj( std::unique_ptr<XclObj> pObjRec );
    std::unique_ptr<XclObj> RemoveLastObj();

protected:
    explicit            XclExpObjectManager( const XclExpObjectManager& rParent );

private:
    void                InitStream( bool bTempFile );

private:
    std::shared_ptr< ::utl::TempFile > mxTempFile;
    std::shared_ptr< SvStream >  mxDffStrm;
    std::shared_ptr< XclEscherEx > mxEscherEx;
    std::shared_ptr< XclExpObjList > mxObjList;
};

class XclExpEmbeddedObjectManager : public XclExpObjectManager
{
public:
    explicit            XclExpEmbeddedObjectManager(
                            const XclExpObjectManager& rParent,
                            const Size& rPageSize,
                            sal_Int32 nScaleX, sal_Int32 nScaleY );

    /** Creates a new DFF client anchor object for embedded objects according
        to the scaling data passed to the constructor. Caller takes ownership! */
    virtual XclExpDffAnchorBase* CreateDffAnchor() const override;

private:
    Size const                maPageSize;
    sal_Int32 const           mnScaleX;
    sal_Int32 const           mnScaleY;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

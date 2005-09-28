/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xiescher.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 12:00:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XIESCHER_HXX
#define SC_XIESCHER_HXX

#include <vector>
#include <list>
#include <map>

#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif

#ifndef SC_XLESCHER_HXX
#include "xlescher.hxx"
#endif
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace script { struct ScriptEventDescriptor; }
} } }

class ScfProgressBar;
class ScfPropertySet;

// Text box data ==============================================================

class XclImpTxoData : protected XclImpRoot
{
public:
    explicit            XclImpTxoData( const XclImpRoot& rRoot );

    /** Reads a TXO record and following CONTINUE records for string and formatting data. */
    void                ReadTxo( XclImpStream& rStrm );

    /** Returns the text orientation. */
    inline sal_uInt16   GetOrientation() const { return maData.mnOrient; }
    /** Returns the string data, if there is any. */
    inline const XclImpString* GetString() const { return mxString.get(); }

    /** Sets the text to the passed SdrObject, if it can take text. */
    void                ProcessSdrObject( SdrObject& rSdrObj ) const;

private:
    XclTxoData          maData;         /// Data from the TXO record.
    XclImpStringRef     mxString;       /// Plain or rich string.
};

typedef ScfRef< XclImpTxoData > XclImpTxoDataRef;

// Escher objects =============================================================

class XclImpDrawObjBase;
typedef ScfRef< XclImpDrawObjBase > XclImpDrawObjRef;

/** Base class for drawing objects (OBJ records). */
class XclImpDrawObjBase : protected XclImpRoot, ScfNoCopy
{
public:
    explicit            XclImpDrawObjBase( const XclImpRoot& rRoot );
    virtual             ~XclImpDrawObjBase();

    /** Reads the FTCMO subrecord (common object data) in an OBJ record, returns a new object. */
    static XclImpDrawObjRef ReadObjCmo( XclImpStream& rStrm );
    /** Reads the contents of the specified subrecord of an OBJ record from stream. */
    virtual void        ReadSubRecord( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize );
    /** Reads the client anchor from an msofbtClientAnchor Escher record. */
    Rectangle           ReadClientAnchor( SvStream& rEscherStrm, const DffRecordHeader& rHeader );

    /** Sets common object data from FTCMO subrecord. */
    void                SetObjData( sal_uInt16 nObjType, sal_uInt16 nObjId, sal_uInt16 nObjFlags );
    /** Sets shape data from Escher stream. */
    void                SetShapeData( sal_uInt32 nShapeId, sal_uInt32 nShapeFlags, sal_uInt32 nShapeBlipId );
    /** Sets whether this is an area object (then its width and height must be greater than 0). */
    inline void         SetAreaObj( bool bAreaObj ) { mbAreaObj = bAreaObj; }

    /** If set, the SdrObject will not be created, processed, or inserted into the draw page. */
    inline void         SetInvalid() { mbValid = false; }
    /** If set, the SdrObject will not be processed, but created and inserted into the draw page. */
    inline void         SetSkipProcessSdr() { mbProcSdr = false; }
    /** If set, the SdrObject will be created or processed, but not be inserted into the draw page. */
    inline void         SetSkipInsertSdr() { mbInsSdr = false; }

    /** Returns the Calc sheet index of this object. */
    inline SCTAB        GetScTab() const { return maObjId.mnScTab; }
    /** Returns the sheet index and Excel object identifier from OBJ record. */
    inline const XclObjId& GetObjId() const { return maObjId; }
    /** Returns the Excel object type from OBJ record. */
    inline sal_uInt16   GetObjType() const { return mnObjType; }

    /** Returns the shape identifier used in the Escher stream. */
    inline sal_uInt32   GetShapeId() const { return mnShapeId; }
    /** Returns the shape flags from the Escher stream. */
    inline sal_uInt32   GetShapeFlags() const { return mnShapeFlags; }
    /** Returns the BLIP identifier for the meta file. */
    inline sal_uInt32   GetShapeBlipId() const { return mnShapeBlipId; }

    /** Returns true, if the object is valid and will be processed.. */
    inline bool         IsValid() const { return mbValid; }
    /** Returns true, if the SdrObject will be processed (ProcessSdrObject() will be called). */
    inline bool         IsProcessSdr() const { return mbProcSdr; }
    /** Returns true, if the SdrObject will be created or processed, but not be inserted into the draw page. */
    inline bool         IsInsertSdr() const { return mbInsSdr; }
    /** Returns true, if Escher object is printable. */
    inline bool         IsPrintable() const { return ::get_flag( mnObjFlags, EXC_OBJ_CMO_PRINTABLE ); }

    /** Returns true, if the passed size is valid for this object. */
    bool                IsValidSize( const Rectangle& rAnchorRect ) const;
    /** Returns the area in the sheet used by this object. */
    ScRange             GetUsedArea() const;

    /** Returns the needed size on the progress bar (calls virtual DoGetProgressSize() function). */
    sal_uInt32          GetProgressSize() const;
    /** Additional processing for the passed SdrObject (calls virtual DoProcessSdrObj() function). */
    void                ProcessSdrObject( SdrObject& rSdrObj ) const;

protected:
    /** Derived classes may return a progress bar size different from 1. */
    virtual sal_uInt32  DoGetProgressSize() const;
    /** Derived classes may perform additional processing for the passed SdrObject. */
    virtual void        DoProcessSdrObj( SdrObject& rSdrObj ) const;

    /** Creates an Escher anchor from the passed position (used for sheet charts). */
    void                CreateEscherAnchor( const Rectangle& rAnchorRect );

private:
    typedef ScfRef< XclEscherAnchor > XclEscherAnchorRef;

    XclEscherAnchorRef  mxAnchor;       /// The position of the object in the containing sheet.
    XclObjId            maObjId;        /// Sheet index and object identifier.
    sal_uInt16          mnObjType;      /// The Excel object type from OBJ record.
    sal_uInt16          mnObjFlags;     /// Additional flags from OBJ record.
    sal_uInt32          mnShapeId;      /// Shape ID from Escher stream.
    sal_uInt32          mnShapeFlags;   /// Shape flags from Escher stream.
    sal_uInt32          mnShapeBlipId;  /// The BLIP identifier (meta file).
    bool                mbValid;        /// true = Object is valid, do processing and insertion.
    bool                mbAreaObj;      /// true = Width and height must be greater than 0.
    bool                mbProcSdr;      /// true = Process the SdrObject.
    bool                mbInsSdr;       /// true = Insert the SdrObject into draw page.
};

// ----------------------------------------------------------------------------

/** A simple drawing object, e.g. line, rectangle, textbox, or bitmap. */
class XclImpDrawingObj : public XclImpDrawObjBase
{
public:
    /** @param bAreaObj  true = Width and height of the object must be greater than 0. */
    explicit            XclImpDrawingObj( const XclImpRoot& rRoot, bool bAreaObj );

    /** Stores the passed textbox data from a TXO record. */
    inline void         SetTxoData( XclImpTxoDataRef xTxoData ) { mxTxoData = xTxoData; }

    /** Returns the text orientation from the contained textbox data. */
    inline sal_uInt16   GetOrientation() const { return mxTxoData.is() ? mxTxoData->GetOrientation() : EXC_TXO_TEXTROT_NONE; }
    /** Returns the string from the contained textbox data. */
    inline const XclImpString* GetString() const { return mxTxoData.is() ? mxTxoData->GetString() : 0; }

protected:
    /** Inserts the contained text data at the passed object. */
    virtual void        DoProcessSdrObj( SdrObject& rSdrObj ) const;

private:
    XclImpTxoDataRef    mxTxoData;      /// Textbox data from TXO record.
};

// ----------------------------------------------------------------------------

/** A note object, which is a specialized text box objext. */
class XclImpNoteObj : public XclImpDrawingObj
{
public:
    explicit            XclImpNoteObj( const XclImpRoot& rRoot );

    /** Sets note flags and the note position in the Calc sheet. */
    void                SetNoteData( const ScAddress& rScPos, sal_uInt16 nNoteFlags );

protected:
    /** Inserts the note into the document, sets visibility. */
    virtual void        DoProcessSdrObj( SdrObject& rSdrObj ) const;

private:
    ScAddress           maScPos;        /// Cell position of the note object.
    sal_uInt16          mnNoteFlags;    /// Flags from NOTE record.
};

// ----------------------------------------------------------------------------

/** Helper to manage controls linked to the sheet. */
class XclImpCtrlLinkHelper
{
public:
    explicit            XclImpCtrlLinkHelper( XclCtrlBindMode eBindMode );

    /** Returns the linked cell address, or 0, if not present. */
    inline const ScAddress* GetCellLink() const { return mxCellLink.get(); }
    /** Returns the value binding mode for linked cells. */
    inline XclCtrlBindMode GetBindingMode() const { return meBindMode; }
    /** Returns the linked source cell range, or 0, if not present. */
    inline const ScRange* GetSourceRange() const { return mxSrcRange.get(); }

protected:
    /** Reads the formula for the linked cell from the current position of the stream. */
    void                ReadCellLinkFormula( XclImpStream& rStrm );
    /** Reads the formula for the source range from the current position of the stream. */
    void                ReadSrcRangeFormula( XclImpStream& rStrm );

private:
    ScfRef< ScAddress > mxCellLink;     /// Linked cell in the Calc document.
    ScfRef< ScRange >   mxSrcRange;     /// Source data range in the Calc document.
    XclCtrlBindMode     meBindMode;     /// Value binding mode.
};

// ----------------------------------------------------------------------------

/** An old form control object (does not use the OLE mechanism, but is a "simple" drawing object). */
class XclImpTbxControlObj : public XclImpDrawingObj, public XclImpCtrlLinkHelper
{
public:
    explicit            XclImpTbxControlObj( const XclImpRoot& rRoot );

    /** Reads the contents of the specified subrecord of an OBJ record from stream. */
    virtual void        ReadSubRecord( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize );

    /** Returns the complete component service name for this control. */
    ::rtl::OUString     GetServiceName() const;

    /** Sets form control specific properties. */
    void                WriteToPropertySet( ScfPropertySet& rPropSet ) const;
    /** Tries to fill the passed descriptor with imported macro data.
        @return  true = Control is associated with a macro, rEvent contains valid data. */
    bool                FillMacroDescriptor(
                            ::com::sun::star::script::ScriptEventDescriptor& rEvent ) const;

private:
    /** Reads the contents of the ftCbls sub structure in an OBJ record. */
    void                ReadCbls( XclImpStream& rStrm );
    /** Reads the contents of the ftCblsFmla sub structure in an OBJ record. */
    void                ReadCblsFmla( XclImpStream& rStrm );
    /** Reads the contents of the ftLbsData sub structure in an OBJ record. */
    void                ReadLbsData( XclImpStream& rStrm );
    /** Reads the contents of the ftSbs sub structure in an OBJ record. */
    void                ReadSbs( XclImpStream& rStrm );
    /** Reads the contents of the ftGboData sub structure in an OBJ record. */
    void                ReadGboData( XclImpStream& rStrm );
    /** Reads the contents of the ftMacro sub structure in an OBJ record. */
    void                ReadMacro( XclImpStream& rStrm );

private:
    ScfInt16Vec         maMultiSel;     /// Indexes of all selected entries in a multi selection.
    String              maMacroName;    /// Name of an attached macro.
    sal_uInt16          mnState;        /// Checked/unchecked state.
    sal_Int16           mnSelEntry;     /// Index of selected entry (1-based).
    sal_Int16           mnSelType;      /// Selection type.
    sal_Int16           mnLineCount;    /// Combobox dropdown line count.
    sal_Int16           mnScrollValue;  /// Scrollbar: Current value.
    sal_Int16           mnScrollMin;    /// Scrollbar: Minimum value.
    sal_Int16           mnScrollMax;    /// Scrollbar: Maximum value.
    sal_Int16           mnScrollStep;   /// Scrollbar: Single step.
    sal_Int16           mnScrollPage;   /// Scrollbar: Page step.
    bool                mbFlatButton;   /// False = 3D button style; True = Flat button style.
    bool                mbFlatBorder;   /// False = 3D border style; True = Flat border style.
    bool                mbScrollHor;    /// Scrollbar: true = horizontal.
};

// ----------------------------------------------------------------------------

/** A common Escher OLE object, or an OLE form control. */
class XclImpOleObj : public XclImpDrawObjBase, public XclImpCtrlLinkHelper
{
public:
    explicit            XclImpOleObj( const XclImpRoot& rRoot );

    /** Reads the contents of the specified subrecord of an OBJ record from stream. */
    virtual void        ReadSubRecord( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize );

    /** Returns true, if this object is a form control, and false, if it is a common OLE object. */
    inline bool         IsControl() const { return mbControl; }
    /** Sets the internal name for a form control. */
    inline void         SetControlName( const String& rName ) { maCtrlName = rName; }

    /** Returns the OLE storage name used in the Excel document. */
    inline const String& GetStorageName() const { return maStorageName; }
    /** Returns the position in Ctrl stream for additional form control data. */
    inline sal_uInt32   GetCtrlStreamPos() const { return mnCtrlStrmPos; }
    /** Returns the internal name for a form control. */
    inline const String& GetControlName() const { return maCtrlName; }

    /** Sets form control specific properties. */
    void                WriteToPropertySet( ScfPropertySet& rPropSet ) const;

protected:
    /** Overloaded to do additional processing on the SdrObject. */
    virtual void        DoProcessSdrObj( SdrObject& rSdrObj ) const;

private:
    /** Reads the contents of the ftPioGrbit sub structure in an OBJ record. */
    void                ReadPioGrbit( XclImpStream& rStrm );
    /** Reads the contents of the ftPictFmla sub structure in an OBJ record. */
    void                ReadPictFmla( XclImpStream& rStrm, sal_uInt16 nRecSize );

private:
    String              maStorageName;  /// Name of the OLE storage for this object.
    String              maCtrlName;     /// Internal name of a form control.
    sal_uInt32          mnCtrlStrmPos;  /// Position in Ctrl stream for controls.
    bool                mbAsSymbol;     /// true = Show as symbol.
    bool                mbLinked;       /// true = Linked; false = Embedded.
    bool                mbControl;      /// true = Form control, false = OLE object.
};

// ----------------------------------------------------------------------------

class XclImpChart;

/** A chart object. This is the drawing object wrapper for the chart data. */
class XclImpChartObj : public XclImpDrawObjBase
{
public:
    /** @param bOwnTab  True = chart is on an own sheet; false = chart is an embedded object. */
    explicit            XclImpChartObj( const XclImpRoot& rRoot, bool bOwnTab );

    /** Reads the complete chart substream (BOF/EOF block).
        @descr  The passed stream must be located in the BOF record of the chart substream. */
    void                ReadChartSubStream( XclImpStream& rStrm );

    /** Returns the embedded chart. */
    inline const XclImpChart* GetChart() const { return mxChart.get(); }
    /** Returns the object anchor of a sheet chart (chart fills one page). */
    inline const Rectangle& GetAnchorRect() const { return maAnchorRect; }

protected:
    /** Returns the needed size on the progress bar. */
    virtual sal_uInt32  DoGetProgressSize() const;

private:
    /** Calculates the anchor position of a sheet chart. */
    void                CalcTabChartAnchor();

private:
    typedef ScfRef< XclImpChart > XclImpChartRef;

    XclImpChartRef      mxChart;        /// The chart itself.
    Rectangle           maAnchorRect;   /// Anchor position of sheet charts.
    bool                mbOwnTab;       /// true = own sheet; false = embedded object.
};

// Escher stream conversion ===================================================

/** The solver container collects all connector rules for connected objects. */
class XclImpSolverContainer : public SvxMSDffSolverContainer
{
public:
    /** Reads the entire solver container. Stream must point to begin of container header. */
    void                ReadSolverContainer( SvStream& rEscherStrm );

    /** Inserts a new pointer to an SdrObject by the related shape ID. */
    void                InsertSdrObjectInfo( const XclImpDrawObjBase& rDrawObj, SdrObject* pSdrObj );
    /** Removes a pointer to an SdrObject by the related shape ID. */
    void                RemoveSdrObjectInfo( const XclImpDrawObjBase& rDrawObj );

    /** Inserts the SdrObject pointers into all connector rules. */
    void                UpdateConnectorRules();
    /** Removes all contained connector rules. */
    void                RemoveConnectorRules();

private:
    /** Returns the first connector rule from the internal list. */
    SvxMSDffConnectorRule* GetFirstRule();
    /** Returns the next connector rule from the internal list. */
    SvxMSDffConnectorRule* GetNextRule();
    /** Updates the data of a connected shape in a connector rule. */
    void                UpdateConnection( sal_uInt32 nShapeId, SdrObject*& rpSdrObj, sal_uInt32* pnShapeFlags = 0 );

private:
    /** Stores data about an SdrObject processed during import. */
    struct XclImpSdrInfo
    {
        SdrObject*          mpSdrObj;       /// Pointer to an SdrObject.
        sal_uInt32          mnShapeFlags;   /// Shape flags from escher stream.
        inline explicit     XclImpSdrInfo() : mpSdrObj( 0 ), mnShapeFlags( 0 ) {}
        inline void         Set( SdrObject* pSdrObj, sal_uInt32 nShapeFlags )
                                { mpSdrObj = pSdrObj; mnShapeFlags = nShapeFlags; }
    };
    typedef ::std::map< sal_uInt32, XclImpSdrInfo > XclImpSdrInfoMap;

    XclImpSdrInfoMap    maSdrInfoMap;   /// Maps shape IDs to SdrObjects.
};

// ----------------------------------------------------------------------------

class XclImpObjectManager;
class XclImpOcxConverter;

/** Derived from SvxMSDffManager, contains core implementation of Escher stream import. */
class XclImpDffManager : public SvxMSDffManager, protected XclImpRoot
{
public:
    explicit            XclImpDffManager(
                            const XclImpRoot& rRoot,
                            XclImpObjectManager& rObjManager,
                            SvStream& rEscherStrm );
    virtual             ~XclImpDffManager();

    /** Initializes the internal progress bar with the passed size and starts it. */
    void                StartProgressBar( sal_uInt32 nProgressSize );
    /** Processes the Escher stream, converts all objects. */
    void                ProcessEscherStream( SvStream& rEscherStrm );
    /** Processes a chart from an Excel chart sheet, converts it to a chart object. */
    void                ProcessTabChart( const XclImpChartObj& rChartObj );

    /** Returns the used area in the sheet with the passed index. */
    ScRange             GetUsedArea( SCTAB nScTab ) const;

protected:
    /** Reads the client anchor from the Escher stream and sets it at the correct Escher object. */
    virtual void        ProcessClientAnchor2(
                            SvStream& rEscherStrm,
                            DffRecordHeader& rHeader,
                            void* pClientData,
                            DffObjData& rObjData );
    /** Processes an Escher object, reads properties from Escher stream. */
    virtual SdrObject*  ProcessObj(
                            SvStream& rEscherStrm,
                            DffObjData& rObjData,
                            void* pClientData,
                            Rectangle& rTextRect,
                            SdrObject* pOldSdrObj = 0 );
    /** Returns the BLIP stream position, based on the passed Escher stream position. */
    virtual ULONG       Calc_nBLIPPos( ULONG nOrgVal, ULONG nStreamPos ) const;
    /** Returns a color from the Excel color palette. */
    virtual FASTBOOL    GetColorFromPalette( USHORT nIndex, Color& rColor ) const;

private:
    /** Reads a string property from the passed Escher stream. */
    String              ReadStringProperty( SvStream& rEscherStrm, sal_uInt32 nPropId ) const;

    /** Processes a drawing group container (global drawing data). */
    void                ProcessDggContainer( SvStream& rEscherStrm, const DffRecordHeader& rDggHeader );
    /** Processes a drawing container (all drawing data of a sheet). */
    void                ProcessDgContainer( SvStream& rEscherStrm, const DffRecordHeader& rDgHeader );
    /** Processes the global shape group container (all shapes of a sheet). */
    void                ProcessShGrContainer( SvStream& rEscherStrm, const DffRecordHeader& rShGrHeader );
    /** Processes the solver container (connectors of a sheet). */
    void                ProcessSolverContainer( SvStream& rEscherStrm, const DffRecordHeader& rSolverHeader );
    /** Processes a shape or shape group container (one top-level shape). */
    void                ProcessShContainer( SvStream& rEscherStrm, const DffRecordHeader& rShHeader );

    /** Returns the OCX converter (OCX form controls converter). */
    XclImpOcxConverter& GetOcxConverter();

    /** Inserts the passed SdrObject into the document. This function takes ownership of pSdrObj! */
    void                InsertSdrObject( const XclImpDrawObjBase& rDrawObj, SdrObject* pSdrObj );

    /** Tries to create a custom SdrObject for specific object types. */
    SdrObject*          CreateCustomSdrObject( const XclImpDrawObjBase& rDrawObj, const Rectangle& rAnchorRect );
    /** Creates the SdrObject for the passed Excel OLE object. */
    SdrObject*          CreateSdrObject( const XclImpOleObj& rOleObj, const Rectangle& rAnchorRect );
    /** Creates the SdrObject for the passed Excel textbox control object. */
    SdrObject*          CreateSdrObject( const XclImpTbxControlObj& rTbxCtrlObj, const Rectangle& rAnchorRect );
    /** Creates the SdrObject for the passed Excel chart object. */
    SdrObject*          CreateSdrObject( const XclImpChartObj& rChartObj, const Rectangle& rAnchorRect );

    /** Updates the used area of a sheet with the position and size of the passed object. */
    void                UpdateUsedArea( const XclImpDrawObjBase& rDrawObj );

private:
    typedef ::std::map< SCTAB, ScRange >    ScRangeMap;
    typedef ScfRef< ScfProgressBar >        ScfProgressBarRef;
    typedef ScfRef< XclImpOcxConverter >    XclImpOcxConvRef;

    XclImpObjectManager& mrObjManager;      /// The Excel object manager.
    XclImpSolverContainer maSolverCont;     /// The solver container for connector rules.
    ScRangeMap          maUsedAreaMap;      /// Used ranges for all sheets.
    ScfProgressBarRef   mxProgress;         /// The progress bar used in ProcessObj().
    XclImpOcxConvRef    mxOcxConverter;     /// The form controls converter.
    sal_uInt32          mnOleImpFlags;      /// Application OLE import settings.
};

// The object manager =========================================================

/** Stores all drawing and OLE objects and additional data related to these objects. */
class XclImpObjectManager : protected XclImpRoot
{
public:
    explicit            XclImpObjectManager( const XclImpRoot& rRoot );
    virtual             ~XclImpObjectManager();

    // *** Read Excel records *** ---------------------------------------------

    /** Reads the MSODRAWINGGROUP record. */
    void                ReadMsoDrawingGroup( XclImpStream& rStrm );
    /** Reads the MSODRAWING record. */
    void                ReadMsoDrawing( XclImpStream& rStrm );
    /** Reads the MSODRAWINGSELECTION record. */
    void                ReadMsoDrawingSelection( XclImpStream& rStrm );

    /** Reads the OBJ record. */
    void                ReadObj( XclImpStream& rStrm );
    /** Reads the TXO record. */
    void                ReadTxo( XclImpStream& rStrm );
    /** Reads the NOTE record. */
    void                ReadNote( XclImpStream& rStrm );

    /** Inserts a new chart object and reads the chart substream (BOF/EOF block).
        @descr  Used to import sheet charts, which do not have a corresponding OBJ record. */
    void                ReadTabChart( XclImpStream& rStrm );

    // *** Drawing objects *** ------------------------------------------------

    /** Finds the OBJ record data related to the Escher shape at the passed position. */
    XclImpDrawObjRef    FindDrawObj( const DffRecordHeader& rHeader ) const;
    /** Finds the OBJ record data specified by the passed object identifier. */
    XclImpDrawObjRef    FindDrawObj( const XclObjId& rObjId ) const;
    /** Finds the TXO record data related to the Escher shape at the passed position. */
    XclImpTxoDataRef    FindTxoData( const DffRecordHeader& rHeader ) const;

    /** Sets the object with the passed identification to be ignored on import. */
    void                SetInvalidObj( SCTAB nScTab, sal_uInt16 nObjId );

    // *** Drawing object conversion *** --------------------------------------

    /** Returns the DFF manager (Escher stream converter). Don't call before the Escher stream is read. */
    XclImpDffManager&   GetDffManager();
    /** Inserts all objects into the Calc document. */
    void                ConvertObjects();

    /** Returns the used area in the sheet with the passed index. */
    ScRange             GetUsedArea( SCTAB nScTab ) const;

    // ------------------------------------------------------------------------
private:
    /** Reads contents of an Escher record and append data to internal Escher stream. */
    void                ReadEscherRecord( XclImpStream& rStrm );
    /** Reads a complete chart substream (BOF/EOF block) for an embedded chart object.
        @descr  The passed stream must be located in the BOF record of the chart substream. */
    void                ReadChartSubStream( XclImpStream& rStrm, XclImpChartObj& rChartObj );

    /** Returns the size of the progress bar shown while processing all objects. */
    sal_uInt32          GetProgressSize() const;

    // ------------------------------------------------------------------------
private:
    typedef ::std::map< ULONG, XclImpDrawObjRef >       XclImpObjMap;
    typedef ::std::map< XclObjId, XclImpDrawObjRef >    XclImpObjMapById;
    typedef ::std::map< ULONG, XclImpTxoDataRef >       XclImpTxoMap;
    typedef ScfRef< XclImpChartObj >                    XclImpChartObjRef;
    typedef ::std::list< XclImpChartObjRef >            XclImpChartObjList;
    typedef ScfRef< XclImpDffManager >                  XclImpDffMgrRef;
    typedef ::std::vector< XclObjId >                   XclObjIdVec;

    SvMemoryStream      maEscherStrm;       /// Copy of Escher stream in memory.
    XclImpObjMap        maObjMap;           /// Maps drawing objects to Escher stream position.
    XclImpObjMapById    maObjMapId;         /// Maps drawing objects to sheet index and object ID.
    XclImpTxoMap        maTxoMap;           /// Maps TXO textbox data to sheet index and object ID.
    XclImpChartObjList  maTabCharts;        /// Charts imported from Excel chart sheets.
    XclImpDffMgrRef     mxDffManager;       /// The Escher stream converter.
    XclObjIdVec         maInvalidObjs;      /// All Escher objects to skip.
};

// Escher property set helper =================================================

/** This class reads an Escher property set (msofbtOPT record).
    @descr  It can return separate property values or an item set which contains
    items translated from these properties. */
class XclImpEscherPropSet
{
public:
    explicit            XclImpEscherPropSet( const XclImpDffManager& rDffManager );

    /** Reads an Escher property set from the stream.
        @descr  The stream must point to the start of an Escher record containing properties. */
    void                Read( XclImpStream& rStrm );

    /** Returns the specified property or the default value, if not extant. */
    sal_uInt32          GetPropertyValue( sal_uInt16 nPropId, sal_uInt32 nDefault = 0 ) const;

    /** Translates the properties and fills the item set. */
    void                FillToItemSet( SfxItemSet& rItemSet ) const;

private:
    typedef ::std::auto_ptr< SvMemoryStream > SvMemoryStreamPtr;

    DffPropertyReader   maPropReader;   /// Reads the properties from an SvStream.
    SvMemoryStreamPtr   mxMemStrm;      /// Helper stream.
};

XclImpStream& operator>>( XclImpStream& rStrm, XclImpEscherPropSet& rPropSet );

// ============================================================================

#endif


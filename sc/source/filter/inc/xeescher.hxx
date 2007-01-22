/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xeescher.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-22 13:21:02 $
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

#ifndef SC_XEESCHER_HXX
#define SC_XEESCHER_HXX

#ifndef SC_XLESCHER_HXX
#include "xlescher.hxx"
#endif

#include "xcl97rec.hxx"

namespace com { namespace sun { namespace star {
    namespace script { struct ScriptEventDescriptor; }
} } }

// ============================================================================

/** Helper class for form controils to manage spreadsheet links . */
class XclExpControlObjHelper : protected XclExpRoot
{
public:
    explicit            XclExpControlObjHelper( const XclExpRoot& rRoot );
    virtual             ~XclExpControlObjHelper();

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
class XclExpOcxControlObj : public XclObj, public XclExpControlObjHelper
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
class XclExpTbxControlObj : public XclObj, public XclExpControlObjHelper
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
    sal_Int32           mnHeight;       /// Height of the control.
    sal_uInt16          mnState;        /// Checked/unchecked state.
    sal_Int16           mnLineCount;    /// Combobox dropdown line count.
    sal_Int16           mnSelEntry;     /// Selected entry in combobox (1-based).
    sal_Int16           mnScrollValue;  /// Scrollbar: Current value.
    sal_Int16           mnScrollMin;    /// Scrollbar: Minimum value.
    sal_Int16           mnScrollMax;    /// Scrollbar: Maximum value.
    sal_Int16           mnScrollStep;   /// Scrollbar: Single step.
    sal_Int16           mnScrollPage;   /// Scrollbar: Page step.
    bool                mbFlatButton;   /// False = 3D button style; True = Flat button style.
    bool                mbFlatBorder;   /// False = 3D border style; True = Flat border style.
    bool                mbMultiSel;     /// true = Multi selection in listbox.
    bool                mbScrollHor;    /// Scrollbar: true = horizontal.
};

#endif

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

private:
    /** Writes the body of the NOTE record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    XclExpString        maAuthor;       /// Name of the author.
    ByteString          maNoteText;     /// Main text of the note (<=BIFF7).
    ScAddress           maScPos;        /// Calc cell address of the note.
    sal_uInt16          mnObjId;        /// Escher object ID (BIFF8).
    bool                mbVisible;      /// true = permanently visible.
};

// ============================================================================

#endif


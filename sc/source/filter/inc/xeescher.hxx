/*************************************************************************
 *
 *  $RCSfile: xeescher.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:09:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

class XclExpTokenArray;

/** Helper to manage controls linked to the sheet. */
class XclExpCtrlLinkHelper : protected XclExpRoot
{
public:
    explicit            XclExpCtrlLinkHelper( const XclExpRoot& rRoot );
    virtual             ~XclExpCtrlLinkHelper();

    /** Sets the address of the control's linked cell. */
    void                SetCellLink( const ScAddress& rCellLink );
    /** Sets the address of the control's linked source cell range. */
    void                SetSourceRange( const ScRange& rSrcRange );

protected:
    /** Returns the Excel token array of the cell link, or 0, if no link present. */
    inline const XclExpTokenArray* GetCellLinkTokArr() const { return mxCellLink.get(); }
    /** Returns the Excel token array of the source range, or 0, if no link present. */
    inline const XclExpTokenArray* GetSourceRangeTokArr() const { return mxSrcRange.get(); }
    /** Returns the number of entries in the source range, or 0, if no source set. */
    inline sal_uInt16   GetSourceEntryCount() const { return mnEntryCount; }

    /** Writes a formula with special style only valid in OBJ records. */
    void                WriteFormula( XclExpStream& rStrm, const XclExpTokenArray& rTokArr ) const;
    /** Writes a formula subrecord with special style only valid in OBJ records. */
    void                WriteFormulaSubRec( XclExpStream& rStrm, sal_uInt16 nSubRecId, const XclExpTokenArray& rTokArr ) const;

private:
    XclExpTokenArrayRef mxCellLink;     /// Formula for linked cell.
    XclExpTokenArrayRef mxSrcRange;     /// Formula for source data range.
    sal_uInt16          mnEntryCount;   /// Number of entries in source range.
};

// ----------------------------------------------------------------------------

#if EXC_EXP_OCX_CTRL

/** Represents an OBJ record for an OCX form control. */
class XclExpObjOcxCtrl : public XclObj, public XclExpCtrlLinkHelper
{
public:
    explicit            XclExpObjOcxCtrl(
                            const XclExpRoot& rRoot,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShape >& rxShape,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel >& rxCtrlModel,
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
class XclExpObjTbxCtrl : public XclObj, public XclExpCtrlLinkHelper
{
public:
    explicit            XclExpObjTbxCtrl(
                            const XclExpRoot& rRoot,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShape >& rxShape,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel >& rxCtrlModel );

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
    XclExpTokenArrayRef mxMacroLink;    /// Token array containing a link to an attached macro.
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


/*************************************************************************
 *
 *  $RCSfile: scextopt.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 13:20:17 $
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
#ifndef SC_SCEXTOPT_HXX
#define SC_SCEXTOPT_HXX

#include <memory>

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

// ============================================================================

/** Extended settings for the document, used in import/export filters. */
struct ScExtDocSettings
{
    ScRange             maOleSize;          /// Visible range if embedded.
    String              maGlobCodeName;     /// Global codename (VBA module name).
    double              mfTabBarWidth;      /// Width of the tabbar, relative to frame window width (0.0 ... 1.0).
    sal_uInt32          mnLinkCnt;          /// Recursive counter for loading external documents.
    SCTAB               mnDisplTab;         /// Index of displayed sheet.
    bool                mbWinProtected;     /// true = Window properties are protected.
    bool                mbEncrypted;        /// true = Imported file was encrypted.

    explicit            ScExtDocSettings();
};

// ============================================================================

/** Enumerates possible positions of panes in split sheets. */
enum ScExtPanePos
{
    SCEXT_PANE_TOPLEFT,         /// Single, top, left, or top-left pane.
    SCEXT_PANE_TOPRIGHT,        /// Right, or top-right pane.
    SCEXT_PANE_BOTTOMLEFT,      /// Bottom, or bottom-left pane.
    SCEXT_PANE_BOTTOMRIGHT      /// Bottom-right pane.
};

// ----------------------------------------------------------------------------

/** Extended settings for a sheet, used in import/export filters. */
struct ScExtTabSettings
{
    ScRange             maUsedArea;         /// Used area in the sheet (columns/rows only).
    ScRangeList         maSelection;        /// Selected cell ranges (columns/rows only).
    ScAddress           maCursor;           /// The cursor position (column/row only).
    ScAddress           maFirstVis;         /// Top-left visible cell (column/row only).
    ScAddress           maSecondVis;        /// Top-left visible cell in add. panes (column/row only).
    ScAddress           maFreezePos;        /// Position of frozen panes (column/row only).
    Point               maSplitPos;         /// Position of split.
    ScExtPanePos        meActivePane;       /// Active (focused) pane.
    Color               maGridColor;        /// Grid color.
    long                mnNormalZoom;       /// Zoom in percent for normal view.
    long                mnPageZoom;         /// Zoom in percent for pagebreak preview.
    bool                mbSelected;         /// true = Sheet is selected.
    bool                mbFrozenPanes;      /// true = Frozen panes; false = Normal splits.
    bool                mbPageMode;         /// true = Pagebreak mode; false = Normal view mode.

    explicit            ScExtTabSettings();
};

// ============================================================================

struct ScExtDocOptionsImpl;

/** Extended options held by an ScDocument containing additional settings for filters.

    This object is owned by a Calc document. It contains global document settings
    (struct ScExtDocSettings), settings for all sheets in the document
    (struct ScExtTabSettings), and a list of codenames used for VBA import/export.
 */
class ScExtDocOptions
{
public:
    explicit            ScExtDocOptions();
                        ScExtDocOptions( const ScExtDocOptions& rSrc );
                        ~ScExtDocOptions();

    ScExtDocOptions&    operator=( const ScExtDocOptions& rSrc );

    /** Returns true, if the data needs to be copied to the view data after import. */
    bool                IsChanged() const;
    /** If set to true, the data will be copied to the view data after import. */
    void                SetChanged( bool bChanged );

    /** Returns read access to the global document settings. */
    const ScExtDocSettings& GetDocSettings() const;
    /** Returns read/write access to the global document settings. */
    ScExtDocSettings&   GetDocSettings();

    /** Returns read access to the settings of a sheet, if extant; otherwise 0. */
    const ScExtTabSettings* GetTabSettings( SCTAB nTab ) const;
    /** Returns read/write access to the settings of a sheet, may create a new struct. */
    ScExtTabSettings&   GetOrCreateTabSettings( SCTAB nTab );

    /** Returns the number of sheet codenames. */
    size_t              GetCodeNameCount() const;
    /** Returns the specified codename (empty string = no codename). */
    const String&       GetCodeName( size_t nIdx ) const;
    /** Appends a codename for a sheet. */
    void                AppendCodeName( const String& rCodeName );

private:
    ::std::auto_ptr< ScExtDocOptionsImpl > mxImpl;
};

// ============================================================================

#endif


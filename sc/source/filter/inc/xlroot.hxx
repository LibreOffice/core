/*************************************************************************
 *
 *  $RCSfile: xlroot.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:31:09 $
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

// ============================================================================

#ifndef SC_XLROOT_HXX
#define SC_XLROOT_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif


// Global data ================================================================

class ScEditEngineDefaulter;
class ScHeaderEditEngine;
class EditEngine;
class XclTracer;

struct RootData;//!

/** Stores global buffers and data needed elsewhere in the Excel filters. */
struct XclRootData
{
    typedef ::std::auto_ptr< ScEditEngineDefaulter >    ScEditEngineDefaulterPtr;
    typedef ::std::auto_ptr< ScHeaderEditEngine >       ScHeaderEditEnginePtr;
    typedef ::std::auto_ptr< EditEngine >               EditEnginePtr;
    typedef ::std::auto_ptr< XclTracer >                XclTracerPtr;

    XclBiff                     meBiff;         /// Current BIFF version.
    ScDocument&                 mrDoc;          /// The source or destination document.
    String                      maDocUrl;       /// Document URL of imported/exported file.
    String                      maBasePath;     /// Base path of imported/exported file (path of maDocUrl).
    CharSet                     meCharSet;      /// Character set to import/export byte strings.
    LanguageType                meDocLang;      /// Document language (import: from file, export: from system).
    LanguageType                meUILang;       /// UI language (import: from file, export: from system).
    ScAddress                   maScMaxPos;     /// Highest Calc cell position.
    ScAddress                   maXclMaxPos;    /// Highest Excel cell position.
    long                        mnCharWidth;    /// Width of '0' in default font (twips).
    sal_uInt16                  mnScTab;        /// Current Calc sheet index.
    bool                        mbTruncated;    /// Flag for the table truncated warning box.

    ScEditEngineDefaulterPtr    mpEditEngine;   /// Edit engine for rich strings etc.
    ScHeaderEditEnginePtr       mpHFEditEngine; /// Edit engine for header/footer.
    EditEnginePtr               mpDrawEditEng;  /// Edit engine for text boxes.

    XclTracerPtr                mpTracer;       /// Filter tracer.

    ::std::auto_ptr< RootData > mpRDP;//!

#ifdef DBG_UTIL
    sal_Int32                   mnObjCnt;       /// Object counter for mem leak tests.
#endif

    explicit                    XclRootData(
                                    XclBiff eBiff,
                                    ScDocument& rDocument,
                                    const String& rDocUrl,
                                    CharSet eCharSet );
    virtual                     ~XclRootData();
};


// ----------------------------------------------------------------------------

class SfxObjectShell;
class ScModelObj;
class SfxPrinter;
class SvNumberFormatter;
class ScDocumentPool;
class ScStyleSheetPool;
class ScRangeName;
class SvStorage;
struct XclFontData;

/** Access to global data for a filter object (imported or exported document) from other classes. */
class XclRoot
{
private:
    mutable XclRootData&        mrData;     /// Reference to the global data struct.

public:
                                XclRoot( const XclRoot& rRoot );
    virtual                     ~XclRoot();

    XclRoot&                    operator=( const XclRoot& rRoot );

    RootData*                   mpRD;//!

    /** Returns this root instance - for code readability in derived classes. */
    inline const XclRoot&       GetRoot() const { return *this; }
    /** Returns the current BIFF version of the importer/exporter. */
    inline XclBiff              GetBiff() const { return mrData.meBiff; }
    /** Returns the document language, i.e. for number formats. */
    inline LanguageType         GetDocLanguage() const { return mrData.meDocLang; }
    /** Returns the UI language. */
    inline LanguageType         GetUILanguage() const { return mrData.meUILang; }
    /** Returns the current Calc sheet index. */
    inline sal_uInt16           GetScTab() const { return mrData.mnScTab; }
    /** Returns whether the "some cells have been cut" warning box should show. */
    inline bool                 IsTruncated() const { return mrData.mbTruncated; }

    /** Returns the document URL of the imported/exported file. */
    inline const String&        GetDocUrl() const { return mrData.maDocUrl; }
    /** Returns the base path of the imported/exported file. */
    inline const String&        GetBasePath() const { return mrData.maBasePath; }
    /** Returns the character set to import/export byte strings. */
    inline CharSet              GetCharSet() const { return mrData.meCharSet; }
    /** Returns the width of the '0' character (default font) for the current printer (twips). */
    inline long                 GetCharWidth() const { return mrData.mnCharWidth; }

    /** Returns the destination document (import) or source document (export). */
    inline ScDocument&          GetDoc() const { return mrData.mrDoc; }
    /** Returns pointer to the destination document (import) or source document (export). */
    inline ScDocument*          GetDocPtr() const { return &mrData.mrDoc; }
    /** Returns the object shell of the Calc document. May be NULL (i.e. import from clipboard). */
    SfxObjectShell*             GetDocShell() const;
    /** Returns the object model of the Calc document. */
    ScModelObj*                 GetDocModelObj() const;
    /** Returns pointer to the printer of the Calc document. */
    SfxPrinter*                 GetPrinter() const;
    /** Returns the number formatter of the Calc document. */
    SvNumberFormatter&          GetFormatter() const;
    /** Returns the style sheet pool of the Calc document. */
    ScStyleSheetPool&           GetStyleSheetPool() const;
    /** Returns the defined names container of the Calc document. */
    ScRangeName&                GetNamedRanges() const;

    /** Returns the OLE2 root storage of the imported/exported file.
        @return  Pointer to root storage or 0, if the file is a simple stream. */
    SvStorage*                  GetRootStorage() const;

    /** Returns the edit engine for import/export of rich strings etc. */
    ScEditEngineDefaulter&      GetEditEngine() const;
    /** Returns the edit engine for import/export of headers/footers. */
    ScHeaderEditEngine&         GetHFEditEngine() const;
    /** Returns the edit engine for import/export of drawing text boxes. */
    EditEngine&                 GetDrawEditEngine() const;

    /** Returns the filter tracer. */
    XclTracer&                  GetTracer() const;

    /** Returns the highest possible cell address in a Calc document. */
    inline const ScAddress&     GetScMaxPos() const { return mrData.maScMaxPos; }
    /** Returns the highest possible cell address in an Excel document (using current BIFF version). */
    inline const ScAddress&     GetXclMaxPos() const { return mrData.maXclMaxPos; }

protected:
    explicit                    XclRoot( XclRootData& rRootData );

    /** Sets the BIFF version. */
    void                        SetBiff( XclBiff eBiff );
    /** Sets the document language, i.e. for number formats. */
    inline void                 SetDocLanguage( LanguageType eLang ) { mrData.meDocLang = eLang; }
    /** Sets the UI language, i.e. if it has been read from a file. */
    inline void                 SetUILanguage( LanguageType eLang ) { mrData.meUILang = eLang; }
    /** Sets the character set to import/export byte strings. */
    inline void                 SetCharSet( CharSet eCharSet ) { mrData.meCharSet = eCharSet; }
    /** Sets the width of the '0' character (default font) for the current printer (twips).
        @param rFontData  The font used for the '0' character. */
    void                        SetCharWidth( const XclFontData& rFontData );
    /** Increases the current Calc sheet index by 1. */
    inline void                 IncScTab() { ++mrData.mnScTab; }
    /** Sets the maximum possible cell address according to the current BIFF version. */
    void                        SetMaxPos();

    /** Checks if the passed cell address is valid.
        @descr  Sets the internal flag that produces a warning box, if the cell is
        outside the passed maximum position.
        @param rPos  The cell address to check.
        @param rMaxPos  Highest valid cell address.
        @return  true = cell address is valid. */
    bool                        CheckCellAddress( const ScAddress& rPos, const ScAddress rMaxPos ) const;
    /** Checks and eventually crops the cell range to passed dimensions.
        @descr  Sets the internal flag that produces a warning box, if the cell range
        contains invalid cells. If the range is partly valid, this function sets
        the warning flag, corrects the range and returns true.
        @param rRange  (In/out) The cell range to check.
        @param rMaxPos  Highest valid cell address.
        @return  true = rRange contains a valid cell range (original or cropped). */
    bool                        CheckCellRange( ScRange& rRange, const ScAddress rMaxPos ) const;
    /** Checks and eventually crops the cell ranges to passed dimensions.
        @descr  Sets the internal flag that produces a warning box, if at least one
        cell range contains invalid cells. If the range is partly valid, this function
        sets the warning flag and corrects the range. Cell ranges which do not fit
        full or partly will be removed from the list.
        @param rRangeList  (In/out) The cell range list to check.
        @param rMaxPos  Highest valid cell address. */
    void                        CheckCellRangeList( ScRangeList& rRanges, const ScAddress& rMaxPos ) const;
};


// ============================================================================

#endif


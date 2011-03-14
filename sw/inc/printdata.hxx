/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef SW_PRINTDATA_HXX
#define SW_PRINTDATA_HXX


#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vcl/print.hxx>

#include <set>
#include <map>
#include <vector>
#include <utility>

class SwPageFrm;
class SwDoc;
class SwDocShell;
class ViewShell;
class _SetGetExpFlds;
class SwViewOption;
class OutputDevice;
class SwViewOptionAdjust_Impl;
class SwWrtShell;
class SfxViewShell;

// forward declarations
class SwPrintUIOptions;
class SwRenderData;

////////////////////////////////////////////////////////////


class SwPrintData
{
    const SwPrintUIOptions *    m_pPrintUIOptions;  // not owner
    const SwRenderData *        m_pRenderData;      // not owner

public:

    sal_Bool bPrintGraphic, bPrintTable, bPrintDraw, bPrintControl, bPrintPageBackground,
             bPrintBlackFont,
             //#i81434# - printing of hidden text
             bPrintHiddenText, bPrintTextPlaceholder,
             bPrintLeftPages, bPrintRightPages, bPrintReverse, bPrintProspect,
             bPrintProspectRTL,
             bPrintSingleJobs, bPaperFromSetup,
             // Print empty pages
             bPrintEmptyPages,

             // #i56195# no field update while printing mail merge documents
             bUpdateFieldsInPrinting,
             bModified;

    sal_Int16           nPrintPostIts;
    rtl::OUString       sFaxName;

    SwPrintData()
    {
        m_pPrintUIOptions       = NULL;
        m_pRenderData        = NULL;

        bPrintGraphic           =
        bPrintTable             =
        bPrintDraw              =
        bPrintControl           =
        bPrintLeftPages         =
        bPrintRightPages        =
        bPrintPageBackground    =
        bPrintEmptyPages        =
        bUpdateFieldsInPrinting = sal_True;

        bPaperFromSetup         =
        bPrintReverse           =
        bPrintProspect          =
        bPrintProspectRTL       =
        bPrintSingleJobs        =
        bModified               =
        bPrintBlackFont         =
        bPrintHiddenText        =
        bPrintTextPlaceholder   = sal_False;

        nPrintPostIts           = 0;
    }

    virtual ~SwPrintData() {}

    sal_Bool operator==(const SwPrintData& rData)const
    {
        return
        bPrintGraphic       ==   rData.bPrintGraphic        &&
        bPrintTable         ==   rData.bPrintTable          &&
        bPrintDraw          ==   rData.bPrintDraw           &&
        bPrintControl       ==   rData.bPrintControl        &&
        bPrintPageBackground==   rData.bPrintPageBackground &&
        bPrintBlackFont     ==   rData.bPrintBlackFont      &&
        bPrintLeftPages     ==   rData.bPrintLeftPages      &&
        bPrintRightPages    ==   rData.bPrintRightPages     &&
        bPrintReverse       ==   rData.bPrintReverse        &&
        bPrintProspect      ==   rData.bPrintProspect       &&
        bPrintProspectRTL   ==   rData.bPrintProspectRTL    &&
        bPrintSingleJobs    ==   rData.bPrintSingleJobs     &&
        bPaperFromSetup     ==   rData.bPaperFromSetup      &&
        bPrintEmptyPages    ==   rData.bPrintEmptyPages     &&
        bUpdateFieldsInPrinting == rData.bUpdateFieldsInPrinting &&
        nPrintPostIts       ==   rData.nPrintPostIts        &&
        sFaxName            ==   rData.sFaxName             &&
        bPrintHiddenText    ==   rData.bPrintHiddenText     &&
        bPrintTextPlaceholder   ==   rData.bPrintTextPlaceholder;
    }

    // Note: in the context where this class ist used the pointers should always be valid
    // during the lifetime of this object
    const SwPrintUIOptions &    GetPrintUIOptions() const       { return *m_pPrintUIOptions; }
    const SwRenderData &        GetRenderData() const           { return *m_pRenderData; }
    void  SetPrintUIOptions( const SwPrintUIOptions *pOpt )     { m_pPrintUIOptions = pOpt; }
    void  SetRenderData( const SwRenderData *pData )            { m_pRenderData = pData; }

    sal_Bool IsPrintGraphic() const             { return bPrintGraphic; }
    sal_Bool IsPrintTable() const               { return bPrintTable; }
    sal_Bool IsPrintDraw() const                { return bPrintDraw; }
    sal_Bool IsPrintControl() const             { return bPrintControl; }
    sal_Bool IsPrintLeftPage() const            { return bPrintLeftPages; }
    sal_Bool IsPrintRightPage() const           { return bPrintRightPages; }
    sal_Bool IsPrintReverse() const             { return bPrintReverse; }
    sal_Bool IsPaperFromSetup() const           { return bPaperFromSetup; }
    sal_Bool IsPrintEmptyPages() const          { return bPrintEmptyPages; }
    sal_Bool IsPrintProspect() const            { return bPrintProspect; }
    sal_Bool IsPrintProspectRTL() const         { return bPrintProspectRTL; }
    sal_Bool IsPrintPageBackground() const      { return bPrintPageBackground; }
    sal_Bool IsPrintBlackFont() const           { return bPrintBlackFont; }
    sal_Bool IsPrintSingleJobs() const          { return bPrintSingleJobs; }
    sal_Int16 GetPrintPostIts() const           { return nPrintPostIts; }
    const rtl::OUString GetFaxName() const      { return sFaxName; }
    sal_Bool IsPrintHiddenText() const          { return bPrintHiddenText; }
    sal_Bool IsPrintTextPlaceholder() const     { return bPrintTextPlaceholder; }

    void SetPrintGraphic( sal_Bool b )              { doSetModified(); bPrintGraphic = b; }
    void SetPrintTable( sal_Bool b )                { doSetModified(); bPrintTable = b; }
    void SetPrintDraw( sal_Bool b )                 { doSetModified(); bPrintDraw = b; }
    void SetPrintControl( sal_Bool b )              { doSetModified(); bPrintControl = b; }
    void SetPrintLeftPage( sal_Bool b )             { doSetModified(); bPrintLeftPages = b; }
    void SetPrintRightPage( sal_Bool b )            { doSetModified(); bPrintRightPages = b; }
    void SetPrintReverse( sal_Bool b )              { doSetModified(); bPrintReverse = b; }
    void SetPaperFromSetup( sal_Bool b )            { doSetModified(); bPaperFromSetup = b; }
    void SetPrintEmptyPages( sal_Bool b )           { doSetModified(); bPrintEmptyPages = b; }
    void SetPrintPostIts( sal_Int16 n )             { doSetModified(); nPrintPostIts = n; }
    void SetPrintProspect( sal_Bool b )             { doSetModified(); bPrintProspect = b; }
    void SetPrintProspect_RTL( sal_Bool b )         { doSetModified(); bPrintProspectRTL = b; }
    void SetPrintPageBackground( sal_Bool b )       { doSetModified(); bPrintPageBackground = b; }
    void SetPrintBlackFont( sal_Bool b )            { doSetModified(); bPrintBlackFont = b; }
    void SetPrintSingleJobs( sal_Bool b )           { doSetModified(); bPrintSingleJobs = b; }
    void SetFaxName( const rtl::OUString& rSet )    { sFaxName = rSet; }
    void SetPrintHiddenText( sal_Bool b )           { doSetModified(); bPrintHiddenText = b; }
    void SetPrintTextPlaceholder( sal_Bool b )      { doSetModified(); bPrintTextPlaceholder = b; }

    virtual void doSetModified () { bModified = sal_True;}
};


////////////////////////////////////////////////////////////


class SwPrintUIOptions : public vcl::PrinterOptionsHelper
{
    OutputDevice* m_pLast;
    const SwPrintData & m_rDefaultPrintData;

public:
    SwPrintUIOptions( bool bWeb, bool bSwSrcView, bool bHasSelection, bool bHasPostIts, const SwPrintData &rDefaultPrintData );
    virtual ~SwPrintUIOptions();

    bool processPropertiesAndCheckFormat( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& i_rNewProp );

    bool IsPrintFormControls() const            { return getBoolValue( "PrintControls",        m_rDefaultPrintData.bPrintControl ); }
    bool IsPrintPageBackground() const          { return getBoolValue( "PrintPageBackground",  m_rDefaultPrintData.bPrintPageBackground ); }
    bool IsPrintProspect() const                { return getBoolValue( "PrintProspect",        m_rDefaultPrintData.bPrintProspect ); }
    bool IsPrintProspectRTL() const             { return getIntValue( "PrintProspectRTL",      m_rDefaultPrintData.bPrintProspectRTL ) ? true : false; }
    bool IsPrintTextPlaceholders() const        { return getBoolValue( "PrintTextPlaceholder", m_rDefaultPrintData.bPrintTextPlaceholder ); }
    bool IsPrintHiddenText() const              { return getBoolValue( "PrintHiddenText",      m_rDefaultPrintData.bPrintHiddenText ); }
    bool IsPrintWithBlackTextColor() const      { return getBoolValue( "PrintBlackFonts",      m_rDefaultPrintData.bPrintBlackFont ); }
    sal_Int16 GetPrintPostItsType() const       { return static_cast< sal_Int16 >(getIntValue( "PrintAnnotationMode", m_rDefaultPrintData.nPrintPostIts )); }
    bool IsPaperFromSetup() const               { return getBoolValue( "PrintPaperFromSetup",  m_rDefaultPrintData.bPaperFromSetup ); }
    bool IsPrintReverse() const                 { return false; /*handled by print dialog now*/ }

    bool IsPrintLeftPages() const;
    bool IsPrintRightPages() const;
    bool IsPrintEmptyPages( bool bIsPDFExport ) const;
    bool IsPrintTables() const;
    bool IsPrintGraphics() const;
    bool IsPrintDrawings() const;
};


////////////////////////////////////////////////////////////

// A class that stores temporary data that is needed for rendering the document.
// Usually this data is created when 'getRendererCount' is called and
// and it is used in the 'render' function of that same interface
class SwRenderData
{
    // pages valid for printing (according to the current settings)
    // and their respective start frames (see getRendererCount in unotxdoc.cxx)
    // This set of pages does NOT depend on the 'PageRange' that is used as a printing option!
    std::set< sal_Int32 >                       m_aValidPages;          // the set of possible pages (see StringRangeEnumerator::getRangesFromString )
    std::map< sal_Int32, const SwPageFrm * >    m_aValidStartFrames;    // the map of start frames for those pages

    // printer paper tray to use for each of the m_aValidPages above
    std::map< sal_Int32, sal_Int32 >            m_aPrinterPaperTrays;

    // vector of pages and their order to be printed (duplicates and any order allowed!)
    // (see 'render' in unotxdoc.cxx)
    std::vector< sal_Int32 >                    m_aPagesToPrint;

    std::vector< const SwPageFrm * >            m_aPostItStartFrames;

    // for prospect printing: the pairs of pages to be printed together on a single prospect page.
    // -1 indicates a half page to be left empty.
    std::vector< std::pair< sal_Int32, sal_Int32 > >    m_aPagePairs;

    rtl::OUString               m_aPageRange;

    // the view options to be applied for printing
    SwViewOptionAdjust_Impl *   m_pViewOptionAdjust;

    SwPrintData *               m_pPrtOptions;

public:

    // PostIt relevant data
    _SetGetExpFlds *    m_pPostItFields;    // an array of "_SetGetExpFld *" sorted by page and line numbers
    SwDoc *             m_pPostItDoc;
    ViewShell *         m_pPostItShell;

public:
    SwRenderData();
    ~SwRenderData();


    bool HasPostItData() const  { return m_pPostItShell != 0 && m_pPostItDoc != 0; }
    void CreatePostItData( SwDoc *pDoc, const SwViewOption *pViewOpt, OutputDevice *pOutDev );
    void DeletePostItData();

    bool IsViewOptionAdjust() const  { return m_pViewOptionAdjust != 0; }
    bool NeedNewViewOptionAdjust( const SwWrtShell& ) const;
    void ViewOptionAdjustStart( SwWrtShell &rSh, const SwViewOption &rViewOptions );
    void ViewOptionAdjust( SwPrintData const*const pPrtOptions );
    void ViewOptionAdjustStop();

    bool HasSwPrtOptions() const    { return m_pPrtOptions != 0; }
    void SetSwPrtOptions(SwPrintData *const pOpt)   { m_pPrtOptions = pOpt; }
    SwPrintData const*  GetSwPrtOptions() const     { return m_pPrtOptions; }
    SwPrintData &       GetSwPrtOptionsRef()        { return *m_pPrtOptions; }
    void MakeSwPrtOptions( SwPrintData & rOptions, const SwDocShell *pDocShell,
            const SwPrintUIOptions *pOpt, const SwRenderData *pData, bool bIsPDFExport );


    typedef std::map< sal_Int32, const SwPageFrm * >            ValidStartFramesMap_t;
    typedef std::vector< std::pair< sal_Int32, sal_Int32 > >    PagePairsVec_t;

    std::set< sal_Int32 > &             GetValidPagesSet()          { return m_aValidPages; }
    const std::set< sal_Int32 > &       GetValidPagesSet() const    { return m_aValidPages; }
    ValidStartFramesMap_t &             GetValidStartFrames()       { return m_aValidStartFrames; }
    const ValidStartFramesMap_t &       GetValidStartFrames() const { return m_aValidStartFrames; }

    // a map for printer paper tray numbers to use for each document page
    // a value of -1 for the tray means that there is no specific tray defined
    std::map< sal_Int32, sal_Int32 >&        GetPrinterPaperTrays()          { return m_aPrinterPaperTrays; }
    const std::map< sal_Int32, sal_Int32 >&  GetPrinterPaperTrays() const    { return m_aPrinterPaperTrays; }

    // used for 'normal' printing
    // A page value of 0 as entry indicates that this page is not from the document but
    // from the post-it document. (See also GetPostItStartFrame below)
    std::vector< sal_Int32 > &          GetPagesToPrint()           { return m_aPagesToPrint; }
    const std::vector< sal_Int32 > &    GetPagesToPrint() const     { return m_aPagesToPrint; }

    // used for 'normal' printing with post-its
    // - if the map entry will be NULL then the respective page to be printed is from
    // the document. In that case use the value from GetPagesToPrint at the same index to
    // get the phys. page number to be printed, and then retrieve the start frame to use
    // from GetValidStartFrms.
    // - If the entry is not NULL it is the start frame of the page from the post-it document
    // that is to be printed
    std::vector< const SwPageFrm * > &          GetPostItStartFrames()          { return m_aPostItStartFrames; }
    const std::vector< const SwPageFrm * > &    GetPostItStartFrames() const    { return m_aPostItStartFrames; }

    // used for prospect printing only
    PagePairsVec_t &                    GetPagePairsForProspectPrinting()           { return m_aPagePairs; }
    const PagePairsVec_t &              GetPagePairsForProspectPrinting() const     { return m_aPagePairs; }

    rtl::OUString   GetPageRange() const                            { return m_aPageRange; }
    void            SetPageRange( const rtl::OUString &rRange )     { m_aPageRange = rRange; }
};


////////////////////////////////////////////////////////////

// last remnants of swprtopt.hxx:
#define POSTITS_NONE    0
#define POSTITS_ONLY    1
#define POSTITS_ENDDOC  2
#define POSTITS_ENDPAGE 3

namespace sw {

void InitPrintOptionsFromApplication(SwPrintData & o_rData, bool const bWeb);

} // namespace sw

#endif  // SW_PRINTDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

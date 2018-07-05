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
#ifndef INCLUDED_SW_INC_PRINTDATA_HXX
#define INCLUDED_SW_INC_PRINTDATA_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vcl/print.hxx>
#include <sfx2/objsh.hxx>

#include <set>
#include <map>
#include <vector>
#include <utility>
#include <memory>

class SwDoc;
class SwDocShell;
class SetGetExpFields;
class SwViewOption;
class SwViewOptionAdjust_Impl;
class SwViewShell;
class SwRenderData;

/** this must match the definitions in css::text::NotePrintMode */
enum class SwPostItMode
{
    NONE    = 0,
    Only    = 1,
    EndDoc  = 2,
    EndPage = 3,
    InMargins = 4
};


class SwPrintData
{
    const SwRenderData *        m_pRenderData;      // not owner

public:

    bool m_bPrintGraphic, m_bPrintTable, m_bPrintDraw, m_bPrintControl, m_bPrintPageBackground,
             m_bPrintBlackFont,
             //#i81434# - printing of hidden text
             m_bPrintHiddenText, m_bPrintTextPlaceholder,
             m_bPrintLeftPages, m_bPrintRightPages, m_bPrintReverse, m_bPrintProspect,
             m_bPrintProspectRTL,
             m_bPrintSingleJobs, m_bPaperFromSetup,
             /// Print empty pages
             m_bPrintEmptyPages,

             /// #i56195# no field update while printing mail merge documents
             m_bUpdateFieldsInPrinting;

    SwPostItMode    m_nPrintPostIts;
    OUString       m_sFaxName;

    SwPrintData()
    {
        m_pRenderData        = nullptr;

        m_bPrintGraphic           =
        m_bPrintTable             =
        m_bPrintDraw              =
        m_bPrintControl           =
        m_bPrintLeftPages         =
        m_bPrintRightPages        =
        m_bPrintPageBackground    =
        m_bPrintEmptyPages        =
        m_bUpdateFieldsInPrinting = true;

        m_bPaperFromSetup         =
        m_bPrintReverse           =
        m_bPrintProspect          =
        m_bPrintProspectRTL       =
        m_bPrintSingleJobs        =
        m_bPrintBlackFont         =
        m_bPrintHiddenText        =
        m_bPrintTextPlaceholder   = false;

        m_nPrintPostIts           = SwPostItMode::NONE;
    }

    virtual ~SwPrintData() {}

    SwPrintData(SwPrintData const &) = default;
    SwPrintData(SwPrintData &&) = default;
    SwPrintData & operator =(SwPrintData const &) = default;
    SwPrintData & operator =(SwPrintData &&) = default;

    bool operator==(const SwPrintData& rData)const
    {
        return
        m_bPrintGraphic       ==   rData.m_bPrintGraphic        &&
        m_bPrintTable         ==   rData.m_bPrintTable          &&
        m_bPrintDraw          ==   rData.m_bPrintDraw           &&
        m_bPrintControl       ==   rData.m_bPrintControl        &&
        m_bPrintPageBackground==   rData.m_bPrintPageBackground &&
        m_bPrintBlackFont     ==   rData.m_bPrintBlackFont      &&
        m_bPrintLeftPages     ==   rData.m_bPrintLeftPages      &&
        m_bPrintRightPages    ==   rData.m_bPrintRightPages     &&
        m_bPrintReverse       ==   rData.m_bPrintReverse        &&
        m_bPrintProspect      ==   rData.m_bPrintProspect       &&
        m_bPrintProspectRTL   ==   rData.m_bPrintProspectRTL    &&
        m_bPrintSingleJobs    ==   rData.m_bPrintSingleJobs     &&
        m_bPaperFromSetup     ==   rData.m_bPaperFromSetup      &&
        m_bPrintEmptyPages    ==   rData.m_bPrintEmptyPages     &&
        m_bUpdateFieldsInPrinting == rData.m_bUpdateFieldsInPrinting &&
        m_nPrintPostIts       ==   rData.m_nPrintPostIts        &&
        m_sFaxName            ==   rData.m_sFaxName             &&
        m_bPrintHiddenText    ==   rData.m_bPrintHiddenText     &&
        m_bPrintTextPlaceholder   ==   rData.m_bPrintTextPlaceholder;
    }

    /** Note: in the context where this class is used the pointers should always be valid
       during the lifetime of this object */
    const SwRenderData &        GetRenderData() const           { return *m_pRenderData; }
    void  SetRenderData( const SwRenderData *pData )            { m_pRenderData = pData; }

    bool IsPrintGraphic() const             { return m_bPrintGraphic; }
    bool IsPrintTable() const               { return m_bPrintTable; }
    bool IsPrintDraw() const                { return m_bPrintDraw; }
    bool IsPrintControl() const             { return m_bPrintControl; }
    bool IsPrintLeftPage() const            { return m_bPrintLeftPages; }
    bool IsPrintRightPage() const           { return m_bPrintRightPages; }
    bool IsPrintReverse() const             { return m_bPrintReverse; }
    bool IsPaperFromSetup() const           { return m_bPaperFromSetup; }
    bool IsPrintEmptyPages() const          { return m_bPrintEmptyPages; }
    bool IsPrintProspect() const            { return m_bPrintProspect; }
    bool IsPrintProspectRTL() const         { return m_bPrintProspectRTL; }
    bool IsPrintPageBackground() const      { return m_bPrintPageBackground; }
    bool IsPrintBlackFont() const           { return m_bPrintBlackFont; }
    bool IsPrintSingleJobs() const          { return m_bPrintSingleJobs; }
    SwPostItMode GetPrintPostIts() const           { return m_nPrintPostIts; }
    const OUString& GetFaxName() const      { return m_sFaxName; }
    bool IsPrintHiddenText() const          { return m_bPrintHiddenText; }
    bool IsPrintTextPlaceholder() const     { return m_bPrintTextPlaceholder; }

    void SetPrintGraphic( bool b )              { doSetModified(); m_bPrintGraphic = b; }
    void SetPrintTable( bool b )                { doSetModified(); m_bPrintTable = b; }
    void SetPrintDraw( bool b )                 { doSetModified(); m_bPrintDraw = b; }
    void SetPrintControl( bool b )              { doSetModified(); m_bPrintControl = b; }
    void SetPrintLeftPage( bool b )             { doSetModified(); m_bPrintLeftPages = b; }
    void SetPrintRightPage( bool b )            { doSetModified(); m_bPrintRightPages = b; }
    void SetPrintReverse( bool b )              { doSetModified(); m_bPrintReverse = b; }
    void SetPaperFromSetup( bool b )            { doSetModified(); m_bPaperFromSetup = b; }
    void SetPrintEmptyPages( bool b )           { doSetModified(); m_bPrintEmptyPages = b; }
    void SetPrintPostIts( SwPostItMode n )      { doSetModified(); m_nPrintPostIts = n; }
    void SetPrintProspect( bool b )             { doSetModified(); m_bPrintProspect = b; }
    void SetPrintProspect_RTL( bool b )         { doSetModified(); m_bPrintProspectRTL = b; }
    void SetPrintPageBackground( bool b )       { doSetModified(); m_bPrintPageBackground = b; }
    void SetPrintBlackFont( bool b )            { doSetModified(); m_bPrintBlackFont = b; }
    void SetPrintSingleJobs( bool b )           { doSetModified(); m_bPrintSingleJobs = b; }
    void SetFaxName( const OUString& rSet )     { m_sFaxName = rSet; }
    void SetPrintHiddenText( bool b )           { doSetModified(); m_bPrintHiddenText = b; }
    void SetPrintTextPlaceholder( bool b )      { doSetModified(); m_bPrintTextPlaceholder = b; }

    virtual void doSetModified () {}
};

class SwPrintUIOptions : public vcl::PrinterOptionsHelper
{
    VclPtr< OutputDevice > m_pLast;
    const SwPrintData & m_rDefaultPrintData;

public:
    SwPrintUIOptions( sal_uInt16 nCurrentPage, bool bWeb, bool bSwSrcView, bool bHasSelection, bool bHasPostIts, const SwPrintData &rDefaultPrintData );
    virtual ~SwPrintUIOptions();

    bool processPropertiesAndCheckFormat( const css::uno::Sequence< css::beans::PropertyValue >& i_rNewProp );

    bool IsPrintFormControls() const            { return getBoolValue( "PrintControls",        m_rDefaultPrintData.m_bPrintControl ); }
    bool IsPrintPageBackground() const          { return getBoolValue( "PrintPageBackground",  m_rDefaultPrintData.m_bPrintPageBackground ); }
    bool IsPrintProspect() const                { return getBoolValue( "PrintProspect",        m_rDefaultPrintData.m_bPrintProspect ); }
    bool IsPrintProspectRTL() const             { return getIntValue( "PrintProspectRTL",      m_rDefaultPrintData.m_bPrintProspectRTL ? 1 : 0 ) != 0; }
    bool IsPrintTextPlaceholders() const        { return getBoolValue( "PrintTextPlaceholder", m_rDefaultPrintData.m_bPrintTextPlaceholder ); }
    bool IsPrintHiddenText() const              { return getBoolValue( "PrintHiddenText",      m_rDefaultPrintData.m_bPrintHiddenText ); }
    bool IsPrintWithBlackTextColor() const      { return getBoolValue( "PrintBlackFonts",      m_rDefaultPrintData.m_bPrintBlackFont ); }
    SwPostItMode GetPrintPostItsType() const       { return static_cast< SwPostItMode >(getIntValue( "PrintAnnotationMode", static_cast<sal_uInt16>(m_rDefaultPrintData.m_nPrintPostIts) )); }
    bool IsPaperFromSetup() const               { return getBoolValue( "PrintPaperFromSetup",  m_rDefaultPrintData.m_bPaperFromSetup ); }

    bool IsPrintLeftPages() const;
    bool IsPrintRightPages() const;
    bool IsPrintEmptyPages( bool bIsPDFExport ) const;
    bool IsPrintGraphics() const;
    bool IsPrintDrawings() const;
};

/** A class that stores temporary data that is needed for rendering the document.
   Usually this data is created when 'getRendererCount' is called and
   and it is used in the 'render' function of that same interface */
class SwRenderData
{
    /** pages valid for printing (according to the current settings)
     This set of pages does NOT depend on the 'PageRange' that is used as a printing option! */
    std::set< sal_Int32 >                       m_aValidPages;          ///< the set of possible pages (see StringRangeEnumerator::getRangesFromString )

    /// printer paper tray to use for each of the m_aValidPages above
    std::map< sal_Int32, sal_Int32 >            m_aPrinterPaperTrays;

    /** vector of pages and their order to be printed (duplicates and any order allowed!)
       (see 'render' in unotxdoc.cxx)
       negative entry indicates the page to be printed is from the post-it doc */
    std::vector< sal_Int32 >                    m_aPagesToPrint;

    /** for prospect printing: the pairs of pages to be printed together on a single prospect page.
       -1 indicates a half page to be left empty. */
    std::vector< std::pair< sal_Int32, sal_Int32 > >    m_aPagePairs;

    OUString               m_aPageRange;

    /** temp print document -- must live longer than m_pViewOptionAdjust!
       also this is a Lock and not a Ref because Ref does not delete the doc */
    SfxObjectShellLock m_xTempDocShell;

    /// the view options to be applied for printing
    std::unique_ptr<SwViewOptionAdjust_Impl> m_pViewOptionAdjust;

    std::unique_ptr<SwPrintData>    m_pPrtOptions;

public:

    // PostIt relevant data
    /// an array of "SetGetExpField *" sorted by page and line numbers
    std::unique_ptr<SetGetExpFields> m_pPostItFields;
    /// this contains a SwDoc with the post-it content
    std::unique_ptr<SwViewShell>      m_pPostItShell;

public:
    SwRenderData();
    ~SwRenderData();

    bool HasPostItData() const  { return m_pPostItShell != nullptr; }
    void CreatePostItData( SwDoc *pDoc, const SwViewOption *pViewOpt, OutputDevice *pOutDev );
    void DeletePostItData();

    SfxObjectShellLock const& GetTempDocShell() const { return m_xTempDocShell;}
    void SetTempDocShell(SfxObjectShellLock const&);

    bool IsViewOptionAdjust() const  { return m_pViewOptionAdjust != nullptr; }
    bool NeedNewViewOptionAdjust( const SwViewShell& ) const;
    void ViewOptionAdjustStart( SwViewShell &rSh, const SwViewOption &rViewOptions);
    void ViewOptionAdjust( SwPrintData const* const pPrtOptions, bool setShowPlaceHoldersInPDF );
    void ViewOptionAdjustStop();
    void ViewOptionAdjustCrashPreventionKludge();

    bool HasSwPrtOptions() const    { return m_pPrtOptions != nullptr; }
    SwPrintData const*  GetSwPrtOptions() const { return m_pPrtOptions.get(); }
    void MakeSwPrtOptions( SwDocShell const*const pDocShell,
            SwPrintUIOptions const*const pOpt, bool const bIsPDFExport );

    typedef std::vector< std::pair< sal_Int32, sal_Int32 > >    PagePairsVec_t;

    std::set< sal_Int32 > &             GetValidPagesSet()          { return m_aValidPages; }
    const std::set< sal_Int32 > &       GetValidPagesSet() const    { return m_aValidPages; }

    /** a map for printer paper tray numbers to use for each document page
       a value of -1 for the tray means that there is no specific tray defined */
    std::map< sal_Int32, sal_Int32 >&        GetPrinterPaperTrays()          { return m_aPrinterPaperTrays; }
    const std::map< sal_Int32, sal_Int32 >&  GetPrinterPaperTrays() const    { return m_aPrinterPaperTrays; }

    /** used for 'normal' printing
       A page value of 0 as entry indicates that this page is not from the document but
       from the post-it document. (See also GetPostItStartFrame below) */
    std::vector< sal_Int32 > &          GetPagesToPrint()           { return m_aPagesToPrint; }
    const std::vector< sal_Int32 > &    GetPagesToPrint() const     { return m_aPagesToPrint; }

    /// used for prospect printing only
    PagePairsVec_t &                    GetPagePairsForProspectPrinting()           { return m_aPagePairs; }
    const PagePairsVec_t &              GetPagePairsForProspectPrinting() const     { return m_aPagePairs; }

    const OUString& GetPageRange() const                            { return m_aPageRange; }
    void            SetPageRange( const OUString &rRange )     { m_aPageRange = rRange; }
};

namespace sw {

void InitPrintOptionsFromApplication(SwPrintData & o_rData, bool const bWeb);

} ///< namespace sw

#endif // INCLUDED_SW_INC_PRINTDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

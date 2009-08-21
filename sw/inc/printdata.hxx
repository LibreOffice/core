/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: printdata.hxx,v $
 * $Revision: 1.11 $
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
#ifndef _SW_PRINTDATA_HXX
#define _SW_PRINTDATA_HXX


#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vcl/print.hxx>

#include <set>
#include <map>
#include <vector>
#include <utility>

class SwPageFrm;
class SwDoc;
class ViewShell;
class _SetGetExpFlds;
class SwViewOption;
class OutputDevice;


////////////////////////////////////////////////////////////

class SwPrintUIOptions : public vcl::PrinterOptionsHelper
{
    OutputDevice* m_pLast;

    // pages valid for printing (according to the current settings)
    // and their respective start frames (see getRendererCount in unotxdoc.cxx)
    // This set of pages does NOT depend on the 'PageRange' that is used as a printing option!
    std::set< sal_Int32 >                       m_aValidPages;       // the set of possible pages (see StringRangeEnumerator::getRangesFromString )
    std::map< sal_Int32, const SwPageFrm * >    m_aValidStartFrms;    // the map of start frames for those pages

    // vector of pages and their order to be printed (duplicates and any order allowed!)
    // (see 'render' in unotxdoc.cxx)
    std::vector< sal_Int32 >                            m_aPagesToPrint;

    std::vector< const SwPageFrm * >                    m_aPostItStartFrame;

    // for prospect printing: the pairs of pages to be printed together on a single prospect page.
    // -1 indicates a half page to be left empty.
    std::vector< std::pair< sal_Int32, sal_Int32 > >    m_aPagePairs;

    rtl::OUString       m_PageRange;

public:

    // PostIt relevant data
    _SetGetExpFlds *    m_pPostItFields;    // an array of "_SetGetExpFld *" sorted by page and line numbers
    SwDoc *             m_pPostItDoc;
    ViewShell *         m_pPostItShell;


public:
    SwPrintUIOptions( BOOL bWeb = FALSE );
    ~SwPrintUIOptions();

    bool processPropertiesAndCheckFormat( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& i_rNewProp );

    bool IsPrintLeftPages() const;
    bool IsPrintRightPages() const;
    bool IsPrintEmptyPages( bool bIsPDFExport ) const;
    bool IsPrintTables() const;
    bool IsPrintGraphics() const;
    bool IsPrintDrawings() const;

    bool HasPostItData() const  { return m_pPostItShell != 0 && m_pPostItDoc != 0 && m_pPostItShell != 0; }
    void CreatePostItData( SwDoc *pDoc, const SwViewOption *pViewOpt, OutputDevice *pOutDev );
    void DeletePostItData();

    typedef std::map< sal_Int32, const SwPageFrm * >            ValidStartFramesMap_t;
    typedef std::vector< std::pair< sal_Int32, sal_Int32 > >    PagePairsVec_t;

    std::set< sal_Int32 > &             GetValidPagesSet()          { return m_aValidPages; }
    const std::set< sal_Int32 > &       GetValidPagesSet() const    { return m_aValidPages; }
    ValidStartFramesMap_t &             GetValidStartFrms()         { return m_aValidStartFrms; }
    const ValidStartFramesMap_t &       GetValidStartFrms() const   { return m_aValidStartFrms; }

    // used for 'normal' printing
    // A page value of 0 as entry indicates that this page is not from the document but
    // from the post-it document. (See also GetPostItStartFrame below)
    std::vector< sal_Int32 > &          GetPagesToPrint()           { return m_aPagesToPrint; }
    const std::vector< sal_Int32 > &    GetPagesToPrint() const     { return m_aPagesToPrint; }

    // used for 'normal' printing with post-its
    // - if the vector entry will be NULL then the respective page to be printed is from
    // the document. In that case use the value from GetPagesToPrint at the same index to
    // get the phys. page number to be printed, and then retrieve the start frame to use
    // from GetValidStartFrms.
    // - If the entry is not NULL it is the start frame of the page from the post-it document
    // that is to be printed
    std::vector< const SwPageFrm * > &           GetPostItStartFrame()              { return m_aPostItStartFrame; }
    const std::vector< const SwPageFrm * > &     GetPostItStartFrame() const        { return m_aPostItStartFrame; }

    // used for prospect printing only
    PagePairsVec_t &                    GetPagePairsForProspectPrinting()           { return m_aPagePairs; }
    const PagePairsVec_t &              GetPagePairsForProspectPrinting() const     { return m_aPagePairs; }

    rtl::OUString   GetPageRange() const                            { return m_PageRange; }
    void            SetPageRange( const rtl::OUString &rRange )     { m_PageRange = rRange; }
};


////////////////////////////////////////////////////////////


class SwPrintData
{
    const SwPrintUIOptions *  m_pPrintUIOptions;

public:

    sal_Bool bPrintGraphic, bPrintTable, bPrintDraw, bPrintControl, bPrintPageBackground,
             bPrintBlackFont,
             //#i81434# - printing of hidden text
             bPrintHiddenText, bPrintTextPlaceholder,
             bPrintLeftPage, bPrintRightPage, bPrintReverse, bPrintProspect,
             bPrintProspect_RTL,
             bPrintSingleJobs, bPaperFromSetup,
             // --> FME 2005-12-13 #b6354161# Print empty pages
             bPrintEmptyPages,
             // <--
             // #i56195# no field update while printing mail merge documents
             bUpdateFieldsInPrinting,
             bModified;

    sal_Int16           nPrintPostIts;
    rtl::OUString       sFaxName;

    SwPrintData()
    {
        m_pPrintUIOptions       = NULL;

        bPrintGraphic           =
        bPrintTable             =
        bPrintDraw              =
        bPrintControl           =
        bPrintLeftPage          =
        bPrintRightPage         =
        bPrintPageBackground    =
        bPrintEmptyPages        =
        bUpdateFieldsInPrinting = sal_True;

        bPaperFromSetup         =
        bPrintReverse           =
        bPrintProspect          =
        bPrintProspect_RTL      =
        bPrintSingleJobs        =
        bModified               =
        bPrintBlackFont         =
        bPrintHiddenText       =
        bPrintTextPlaceholder   = sal_False;

        nPrintPostIts           = 0;
    }

    virtual ~SwPrintData() {}

    sal_Bool operator==(const SwPrintData& rData)const
    {
        return
        bPrintGraphic       ==   rData.bPrintGraphic       &&
        bPrintTable         ==   rData.bPrintTable         &&
        bPrintDraw          ==   rData.bPrintDraw          &&
        bPrintControl       ==   rData.bPrintControl       &&
        bPrintPageBackground==   rData.bPrintPageBackground&&
        bPrintBlackFont     ==   rData.bPrintBlackFont     &&
        bPrintLeftPage      ==   rData.bPrintLeftPage      &&
        bPrintRightPage     ==   rData.bPrintRightPage     &&
        bPrintReverse       ==   rData.bPrintReverse       &&
        bPrintProspect      ==   rData.bPrintProspect      &&
        bPrintProspect_RTL  ==   rData.bPrintProspect_RTL  &&
        bPrintSingleJobs    ==   rData.bPrintSingleJobs    &&
        bPaperFromSetup     ==   rData.bPaperFromSetup     &&
        bPrintEmptyPages    ==   rData.bPrintEmptyPages   &&
        bUpdateFieldsInPrinting == rData.bUpdateFieldsInPrinting &&
        nPrintPostIts       ==   rData.nPrintPostIts       &&
        sFaxName            ==   rData.sFaxName         &&
        bPrintHiddenText       ==   rData.bPrintHiddenText &&
        bPrintTextPlaceholder   ==   rData.bPrintTextPlaceholder;
    }

    // Note:  int the conntext where this class ist used the pointer should always be valid
    // during the lifetime of this object
    const SwPrintUIOptions &    GetPrintUIOptions() const       { return *m_pPrintUIOptions; }
    void SetPrintUIOptions( const SwPrintUIOptions *pOpt )      { m_pPrintUIOptions = pOpt; }

    sal_Bool IsPrintGraphic()   const { return bPrintGraphic; }
    sal_Bool IsPrintTable()     const { return bPrintTable; }
    sal_Bool IsPrintDraw()      const { return bPrintDraw; }
    sal_Bool IsPrintControl()   const { return bPrintControl; }
    sal_Bool IsPrintLeftPage()  const { return bPrintLeftPage; }
    sal_Bool IsPrintRightPage() const { return bPrintRightPage; }
    sal_Bool IsPrintReverse()   const { return bPrintReverse; }
    sal_Bool IsPaperFromSetup() const { return bPaperFromSetup; }
    sal_Bool IsPrintEmptyPages() const{ return bPrintEmptyPages; }
    sal_Bool IsPrintProspect()  const { return bPrintProspect; }
    sal_Bool IsPrintProspect_RTL()  const { return bPrintProspect_RTL; }
    sal_Bool IsPrintPageBackground() const { return bPrintPageBackground; }
    sal_Bool IsPrintBlackFont() const { return bPrintBlackFont;}
    sal_Bool IsPrintSingleJobs() const { return bPrintSingleJobs;}
    sal_Int16 GetPrintPostIts() const { return nPrintPostIts; }
    const rtl::OUString     GetFaxName() const{return sFaxName;}
    sal_Bool IsPrintHiddenText() const {return bPrintHiddenText;}
    sal_Bool IsPrintTextPlaceholder() const {return bPrintTextPlaceholder;}

    void SetPrintGraphic  ( sal_Bool b ) { doSetModified(); bPrintGraphic = b;}
    void SetPrintTable    ( sal_Bool b ) { doSetModified(); bPrintTable = b;}
    void SetPrintDraw     ( sal_Bool b ) { doSetModified(); bPrintDraw = b;}
    void SetPrintControl  ( sal_Bool b ) { doSetModified(); bPrintControl = b; }
    void SetPrintLeftPage ( sal_Bool b ) { doSetModified(); bPrintLeftPage = b;}
    void SetPrintRightPage( sal_Bool b ) { doSetModified(); bPrintRightPage = b;}
    void SetPrintReverse  ( sal_Bool b ) { doSetModified(); bPrintReverse = b;}
    void SetPaperFromSetup( sal_Bool b ) { doSetModified(); bPaperFromSetup = b;}
    void SetPrintEmptyPages(sal_Bool b ) { doSetModified(); bPrintEmptyPages = b;}
    void SetPrintPostIts    ( sal_Int16 n){ doSetModified(); nPrintPostIts = n; }
    void SetPrintProspect   ( sal_Bool b ) { doSetModified(); bPrintProspect = b; }
    void SetPrintPageBackground(sal_Bool b){ doSetModified(); bPrintPageBackground = b;}
    void SetPrintBlackFont(sal_Bool b){ doSetModified(); bPrintBlackFont = b;}
    void SetPrintSingleJobs(sal_Bool b){ doSetModified(); bPrintSingleJobs = b;}
    void SetFaxName(const rtl::OUString& rSet){sFaxName = rSet;}
    void SetPrintHiddenText(sal_Bool b){ doSetModified(); bPrintHiddenText = b;}
    void SetPrintTextPlaceholder(sal_Bool b){ doSetModified(); bPrintTextPlaceholder = b;}

    virtual void doSetModified () { bModified = sal_True;}
};

////////////////////////////////////////////////////////////

#endif  //_SW_PRINTDATA_HXX


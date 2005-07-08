/*************************************************************************
 *
 *  $RCSfile: EnhancedPDFExportHelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 11:00:19 $
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

#ifndef _ENHANCEDPDFEXPORTHELPER_HXX
#define _ENHANCEDPDFEXPORTHELPER_HXX

#ifndef _VCL_PDFEXTOUTDEVDATA_HXX
#include <vcl/pdfextoutdevdata.hxx>
#endif
#ifndef _SWRECT_HXX
#include <swrect.hxx>
#endif

#include <map>
#include <vector>

class vcl::PDFExtOutDevData;
class OutputDevice;
class SwFrm;
class SwLinePortion;
class SwTxtPainter;
class SwEditShell;
class rtl::OUString;
class MultiSelection;
class SwTxtNode;


/*
 * Mapping of OOo elements to tagged pdf elements:
 *
 * OOo element                      tagged pdf element
 * -----------                      ------------------
 *
 * Grouping elements:
 *
 * SwRootFrm                        Document
 * SwSection                        Sect
 * SwSection (TOC)                  TOC
 * SwSection (Index)                Index
 * SwTxtNode in TOC                 TOCI
 *
 * Block-Level Structure Elements:
 *
 * SwTxtNode                        P
 * SwTxtNode with Outline           H1 - H6
 * SwTxtNode with NumRule           L, LI, LBody
 * SwTable                          Table
 * SwRowFrm                         TR
 * SwCellFrm in Headline row        TH
 * SwCellFrm                        TD
 *
 * Illustration elements:
 *
 * SwFlyFrame                       Division
 * SwFlyFrm with Math OLE Object    Formula
 * SwFlyFrm with SwNoTxtFrm         Figure
 *
 * Inline-Level Structure Elements:
 *
 * SwTxtPortion                     Span
 * SwFtnPortion                     Link
 * SwFldPortion (RefField)          Link
 * SwFldPortion (AuthorityField)    BibEntry
 *
 */

struct Frm_Info
{
    const SwFrm& mrFrm;
    Frm_Info( const SwFrm& rFrm ) : mrFrm( rFrm ) {};
};

struct Por_Info
{
    const SwLinePortion& mrPor;
    const SwTxtPainter& mrTxtPainter;
    Por_Info( const SwLinePortion& rPor, const SwTxtPainter& rTxtPainer )
            : mrPor( rPor ), mrTxtPainter( rTxtPainer ) {};
};


/*************************************************************************
 *                class SwTaggedPDFHelper
 * Analyses a given frame during painting and generates the appropriate
 * structure elements.
 *************************************************************************/

class SwTaggedPDFHelper
{
    private:

    // This will be incremented for each BeginTag() call.
    // It denotes the number of tags to close during EndStructureElements();
    BYTE nEndStructureElement;

    //  If an already existing tag is reopened for follows of flow frames,
    // this value stores the tag id which has to be restored.
    sal_Int32 nRestoreCurrentTag;

    vcl::PDFExtOutDevData* mpPDFExtOutDevData;
    const Frm_Info* mpFrmInfo;
    const Por_Info* mpPorInfo;

    void BeginTag( vcl::PDFWriter::StructElement );
    void EndTag();

    void SetAttributes( vcl::PDFWriter::StructElement eType );

    // These functions are called by the c'tor, d'tor
    void BeginBlockStructureElements();
    void BeginInlineStructureElements();
    void EndStructureElements();

    bool CheckReopenTag();
    bool CheckRestoreTag() const;

    public:

    // pFrmInfo != 0 => BeginBlockStructureElement
    // pPorInfo != 0 => BeginInlineStructureElement
    // pFrmInfo, pPorInfo = 0 => BeginNonStructureElement
    SwTaggedPDFHelper( const Frm_Info* pFrmInfo, const Por_Info* pPorInfo,
                       OutputDevice& rOut );
    ~SwTaggedPDFHelper();
};

/*************************************************************************
 *                class SwEnhancedPDFExportHelper
 * Analyses the document structure and export Notes, Hyperlinks, References,
 * and Outline. Link ids created during pdf export are stored in
 * aReferenceIdMap and aHyperlinkIdMap, in order to use them during
 * tagged pdf output. Therefore the SwEnhancedPDFExportHelper is used
 * before painting. Unfortunately links from the EditEngine into the
 * Writer document require to be exported after they have been painted.
 * Therefore SwEnhancedPDFExportHelper also has to be used after the
 * painting process, the parameter bEditEngineOnly indicated that only
 * the bookmarks from the EditEngine have to be processed.
 *************************************************************************/

typedef std::pair< SwRect, sal_Int32 > IdMapEntry;
typedef std::vector< IdMapEntry > LinkIdMap;
typedef std::map< void*, sal_Int32 > FrmTagIdMap;

class SwEnhancedPDFExportHelper
{
    private:

    SwEditShell& mrSh;
    OutputDevice& mrOut;

    MultiSelection* pPageRange;

    bool mbSkipEmptyPages;
    bool mbEditEngineOnly;

    static LinkIdMap aLinkIdMap;
    static FrmTagIdMap aFrmTagIdMap;

    void EnhancedPDFExport();
    sal_Int32 CalcOutputPageNum( const SwRect& rRect ) const;

    void MakeHeaderFooterLinks( vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                const SwTxtNode& rTNd, const SwRect& rLinkRect,
                                sal_Int32 nDestId, const String& rURL, bool bIntern ) const;

    public:

    SwEnhancedPDFExportHelper( SwEditShell& rSh,
                               OutputDevice& rOut,
                               const rtl::OUString& rPageRange,
                               bool bSkipEmptyPages,
                               bool bEditEngineOnly );

    ~SwEnhancedPDFExportHelper();

    static LinkIdMap& GetLinkIdMap() { return aLinkIdMap; }
    static FrmTagIdMap& GetFrmTagIdMap() { return aFrmTagIdMap; }
};

#endif

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Miklos Vajna.
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

#ifndef _RTFEXPORT_HXX_
#define _RTFEXPORT_HXX_

#include <set>
#include "rtfattributeoutput.hxx"
#include "wrtww8.hxx"

#include <rtl/ustring.hxx>

#include <cstdio>
#include <map>

class RtfExportFilter;
class RtfSdrExport;
typedef std::map<sal_uInt16,Color> RtfColorTbl;
typedef std::map<sal_uInt16,rtl::OString> RtfStyleTbl;
typedef std::map<String,sal_uInt16> RtfRedlineTbl;
class SwNode;
class SwEndNode;
class SwTableNode;
class SwTxtNode;
class SwGrfNode;
class SwOLENode;
class SwSectionNode;
class SwNumRuleTbl;

namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
} } }

/// The class that does all the actual RTF export-related work.
class RtfExport : public MSWordExportBase
{
    /// Pointer to the filter that owns us.
    RtfExportFilter *m_pFilter;
    Writer* m_pWriter;

    /// Attribute output for document.
    RtfAttributeOutput *m_pAttrOutput;

    /// Sections/headers/footers
    MSWordSections *m_pSections;

    RtfSdrExport *m_pSdrExport;

public:
    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const;

    /// Access to the Rtf Sdr exporter.
    virtual RtfSdrExport& SdrExporter() const;

    /// Hack, unfortunately necessary at some places for now.
    virtual bool HackIsWW8OrHigher() const;

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich );

    virtual void AppendBookmarks( const SwTxtNode& rNode, xub_StrLen nAktPos, xub_StrLen nLen );

    virtual void AppendBookmark( const rtl::OUString& rName, bool bSkip = false );

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t()*/ ) { /* no-op for rtf, most probably should not even be in MSWordExportBase */ }
    virtual void WriteChar( sal_Unicode );

    /// Write the numbering table.
    virtual void WriteNumbering();

    /// Write the revision table.
    virtual void WriteRevTab();

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt, sal_uInt8 nBreakCode );

    /// Write the field
    virtual void OutputField( const SwField* pFld, ww::eField eFldType,
            const String& rFldCmd, sal_uInt8 nMode = nsFieldFlags::WRITEFIELD_ALL );

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark );
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark );

    virtual void DoComboBox(const rtl::OUString &rName,
                    const rtl::OUString &rHelp,
                    const rtl::OUString &ToolTip,
                    const rtl::OUString &rSelected,
                    com::sun::star::uno::Sequence<rtl::OUString> &rListItems);

    virtual void DoFormText(const SwInputField * pFld);

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar );

protected:
    /// Format-dependant part of the actual export.
    virtual void ExportDocument_Impl();

    virtual void SectionBreaksAndFrames( const SwTxtNode& /*rNode*/ ) {}

    /// Get ready for a new section.
    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFmtPageDesc* pNewPgDescFmt = 0,
                                     const SwPageDesc* pNewPgDesc = 0 );

    /// Return value indicates if an inherited outline numbering is suppressed.
    virtual bool DisallowInheritingOutlineNumbering(const SwFmt &rFmt);

    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& );

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& );

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum );

public:
    /// Pass the pDocument, pCurrentPam and pOriginalPam to the base class.
    RtfExport( RtfExportFilter *pFilter, SwDoc *pDocument,
            SwPaM *pCurrentPam, SwPaM *pOriginalPam, Writer* pWriter );

    /// Destructor.
    virtual ~RtfExport();

#if defined(UNX)
    static const sal_Char sNewLine; // \012 or \015
#else
    static const sal_Char __FAR_DATA sNewLine[]; // \015\012
#endif

    rtl_TextEncoding eDefaultEncoding;
    rtl_TextEncoding eCurrentEncoding;
    /// This is used by OutputFlyFrame_Impl() to control the written syntax
    bool bRTFFlySyntax;

    sal_Bool m_bOutStyleTab : 1;
    SvStream& Strm();
    SvStream& OutULong( sal_uLong nVal );
    SvStream& OutLong( long nVal );
    void OutUnicode(const sal_Char *pToken, const String &rContent);
    void OutDateTime(const sal_Char* pStr, const util::DateTime& rDT );
    static rtl::OString OutChar(sal_Unicode c, int *pUCMode, rtl_TextEncoding eDestEnc);
    static rtl::OString OutString(const String &rStr, rtl_TextEncoding eDestEnc);
    static rtl::OString OutHex(sal_uLong nHex, sal_uInt8 nLen);
    void OutPageDescription( const SwPageDesc& rPgDsc, sal_Bool bWriteReset, sal_Bool bCheckForFirstPage );

    sal_uInt16 GetColor( const Color& rColor ) const;
    void InsColor( const Color& rCol );
    void InsColorLine( const SvxBoxItem& rBox );
    void OutColorTable();
    sal_uInt16 GetRedline( const String& rAuthor );
    const String* GetRedline( sal_uInt16 nId );

    void InsStyle( sal_uInt16 nId, const rtl::OString& rStyle );
    rtl::OString* GetStyle( sal_uInt16 nId );

private:
    /// No copying.
    RtfExport( const RtfExport& );

    /// No copying.
    RtfExport& operator=( const RtfExport& );

    void WriteFonts();
    void WriteStyles();
    void WriteMainText();
    void WriteInfo();
    /// Writes the writer-specific \pgdsctbl group.
    void WritePageDescTable();
    /// This is necessary to have the numbering table ready before the main text is being processed.
    void BuildNumbering();
    void WriteHeaderFooter(const SfxPoolItem& rItem, bool bHeader);
    void WriteHeaderFooter(const SwFrmFmt& rFmt, bool bHeader, const sal_Char* pStr);

    RtfColorTbl m_aColTbl;
    RtfStyleTbl m_aStyTbl;
    RtfRedlineTbl m_aRedlineTbl;
};

#endif // _RTFEXPORT_HXX_
/* vi:set shiftwidth=4 expandtab: */

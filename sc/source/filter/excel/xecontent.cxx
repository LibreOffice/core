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

#include <memory>
#include <xecontent.hxx>

#include <vector>
#include <algorithm>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>
#include <formula/grammar.hxx>
#include <scitems.hxx>
#include <editeng/flditem.hxx>
#include <document.hxx>
#include <validat.hxx>
#include <unonames.hxx>
#include <convuno.hxx>
#include <rangenam.hxx>
#include <tokenarray.hxx>
#include <stlpool.hxx>
#include <patattr.hxx>
#include <fapihelper.hxx>
#include <xehelper.hxx>
#include <xestyle.hxx>
#include <xename.hxx>
#include <xlcontent.hxx>
#include <xltools.hxx>
#include <xeformula.hxx>
#include <rtl/uuid.h>
#include <sal/log.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/relationship.hxx>

using namespace ::oox;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::sheet::XAreaLinks;
using ::com::sun::star::sheet::XAreaLink;

// Shared string table ========================================================

/** A single string entry in the hash table. */
struct XclExpHashEntry
{
    const XclExpString* mpString;       /// Pointer to the string (no ownership).
    sal_uInt32          mnSstIndex;     /// The SST index of this string.
    explicit     XclExpHashEntry( const XclExpString* pString, sal_uInt32 nSstIndex ) :
                            mpString( pString ), mnSstIndex( nSstIndex ) {}
};

/** Function object for strict weak ordering. */
struct XclExpHashEntrySWO
{
    bool         operator()( const XclExpHashEntry& rLeft, const XclExpHashEntry& rRight ) const
                            { return *rLeft.mpString < *rRight.mpString; }
};

/** Implementation of the SST export.
    @descr  Stores all passed strings in a hash table and prevents repeated
    insertion of equal strings. */
class XclExpSstImpl
{
public:
    explicit            XclExpSstImpl();

    /** Inserts the passed string, if not already inserted, and returns the unique SST index. */
    sal_uInt32          Insert( XclExpStringRef xString );

    /** Writes the complete SST and EXTSST records. */
    void                Save( XclExpStream& rStrm );
    void                SaveXml( XclExpXmlStream& rStrm );

private:
    typedef ::std::vector< XclExpHashEntry >    XclExpHashVec;
    typedef ::std::vector< XclExpHashVec >      XclExpHashTab;

    std::vector< XclExpStringRef > maStringVector;   /// List of unique strings (in SST ID order).
    XclExpHashTab       maHashTab;      /// Hashed table that manages string pointers.
    sal_uInt32          mnTotal;        /// Total count of strings (including doubles).
    sal_uInt32          mnSize;         /// Size of the SST (count of unique strings).
};

const sal_uInt32 EXC_SST_HASHTABLE_SIZE = 2048;

XclExpSstImpl::XclExpSstImpl() :
    maHashTab( EXC_SST_HASHTABLE_SIZE ),
    mnTotal( 0 ),
    mnSize( 0 )
{
}

sal_uInt32 XclExpSstImpl::Insert( XclExpStringRef xString )
{
    OSL_ENSURE( xString.get(), "XclExpSstImpl::Insert - empty pointer not allowed" );
    if( !xString.get() )
        xString.reset( new XclExpString );

    ++mnTotal;
    sal_uInt32 nSstIndex = 0;

    // calculate hash value in range [0,EXC_SST_HASHTABLE_SIZE)
    sal_uInt16 nHash = xString->GetHash();
    nHash = (nHash ^ (nHash / EXC_SST_HASHTABLE_SIZE)) % EXC_SST_HASHTABLE_SIZE;

    XclExpHashVec& rVec = maHashTab[ nHash ];
    XclExpHashEntry aEntry( xString.get(), mnSize );
    XclExpHashVec::iterator aIt = ::std::lower_bound( rVec.begin(), rVec.end(), aEntry, XclExpHashEntrySWO() );
    if( (aIt == rVec.end()) || (*aIt->mpString != *xString) )
    {
        nSstIndex = mnSize;
        maStringVector.push_back( xString );
        rVec.insert( aIt, aEntry );
        ++mnSize;
    }
    else
    {
        nSstIndex = aIt->mnSstIndex;
    }

    return nSstIndex;
}

void XclExpSstImpl::Save( XclExpStream& rStrm )
{
    if( maStringVector.empty() )
        return;

    SvMemoryStream aExtSst( 8192 );

    sal_uInt32 nBucket = mnSize;
    while( nBucket > 0x0100 )
        nBucket /= 2;

    sal_uInt16 nPerBucket = llimit_cast< sal_uInt16 >( nBucket, 8 );
    sal_uInt16 nBucketIndex = 0;

    // *** write the SST record ***

    rStrm.StartRecord( EXC_ID_SST, 8 );

    rStrm << mnTotal << mnSize;
    for (auto const& elem : maStringVector)
    {
        if( !nBucketIndex )
        {
            // write bucket info before string to get correct record position
            sal_uInt32 nStrmPos = static_cast< sal_uInt32 >( rStrm.GetSvStreamPos() );
            sal_uInt16 nRecPos = rStrm.GetRawRecPos() + 4;
            aExtSst.WriteUInt32( nStrmPos )             // stream position
                   .WriteUInt16( nRecPos )              // position from start of SST or CONTINUE
                   .WriteUInt16( 0 );     // reserved
        }

        rStrm << *elem;

        if( ++nBucketIndex == nPerBucket )
            nBucketIndex = 0;
    }

    rStrm.EndRecord();

    // *** write the EXTSST record ***

    rStrm.StartRecord( EXC_ID_EXTSST, 0 );

    rStrm << nPerBucket;
    rStrm.SetSliceSize( 8 );    // size of one bucket info
    aExtSst.Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( aExtSst );

    rStrm.EndRecord();
}

void XclExpSstImpl::SaveXml( XclExpXmlStream& rStrm )
{
    if( maStringVector.empty() )
        return;

    sax_fastparser::FSHelperPtr pSst = rStrm.CreateOutputStream(
            "xl/sharedStrings.xml",
            "sharedStrings.xml",
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml",
            OUStringToOString(oox::getRelationship(Relationship::SHAREDSTRINGS), RTL_TEXTENCODING_UTF8).getStr());
    rStrm.PushStream( pSst );

    pSst->startElement( XML_sst,
            XML_xmlns, XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls))).getStr(),
            XML_count, OString::number(  mnTotal ).getStr(),
            XML_uniqueCount, OString::number(  mnSize ).getStr(),
            FSEND );

    for (auto const& elem : maStringVector)
    {
        pSst->startElement( XML_si, FSEND );
        elem->WriteXml( rStrm );
        pSst->endElement( XML_si );
    }

    pSst->endElement( XML_sst );

    rStrm.PopStream();
}

XclExpSst::XclExpSst() :
    mxImpl( new XclExpSstImpl )
{
}

XclExpSst::~XclExpSst()
{
}

sal_uInt32 XclExpSst::Insert( const XclExpStringRef& xString )
{
    return mxImpl->Insert( xString );
}

void XclExpSst::Save( XclExpStream& rStrm )
{
    mxImpl->Save( rStrm );
}

void XclExpSst::SaveXml( XclExpXmlStream& rStrm )
{
    mxImpl->SaveXml( rStrm );
}

// Merged cells ===============================================================

XclExpMergedcells::XclExpMergedcells( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

void XclExpMergedcells::AppendRange( const ScRange& rRange, sal_uInt32 nBaseXFId )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        maMergedRanges.push_back( rRange );
        maBaseXFIds.push_back( nBaseXFId );
    }
}

sal_uInt32 XclExpMergedcells::GetBaseXFId( const ScAddress& rPos ) const
{
    OSL_ENSURE( maBaseXFIds.size() == maMergedRanges.size(), "XclExpMergedcells::GetBaseXFId - invalid lists" );
    ScfUInt32Vec::const_iterator aIt = maBaseXFIds.begin();
    ScRangeList& rNCRanges = const_cast< ScRangeList& >( maMergedRanges );
    for ( size_t i = 0, nRanges = rNCRanges.size(); i < nRanges; ++i, ++aIt )
    {
        const ScRange & rScRange = rNCRanges[ i ];
        if( rScRange.In( rPos ) )
            return *aIt;
    }
    return EXC_XFID_NOTFOUND;
}

void XclExpMergedcells::Save( XclExpStream& rStrm )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        XclRangeList aXclRanges;
        GetAddressConverter().ConvertRangeList( aXclRanges, maMergedRanges, true );
        size_t nFirstRange = 0;
        size_t nRemainingRanges = aXclRanges.size();
        while( nRemainingRanges > 0 )
        {
            size_t nRangeCount = ::std::min< size_t >( nRemainingRanges, EXC_MERGEDCELLS_MAXCOUNT );
            rStrm.StartRecord( EXC_ID_MERGEDCELLS, 2 + 8 * nRangeCount );
            aXclRanges.WriteSubList( rStrm, nFirstRange, nRangeCount );
            rStrm.EndRecord();
            nFirstRange += nRangeCount;
            nRemainingRanges -= nRangeCount;
        }
    }
}

void XclExpMergedcells::SaveXml( XclExpXmlStream& rStrm )
{
    size_t nCount = maMergedRanges.size();
    if( !nCount )
        return;
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_mergeCells,
            XML_count,  OString::number(  nCount ).getStr(),
            FSEND );
    for( size_t i = 0; i < nCount; ++i )
    {
        const ScRange & rRange = maMergedRanges[ i ];
        rWorksheet->singleElement( XML_mergeCell,
                    XML_ref,    XclXmlUtils::ToOString( rRange ).getStr(),
                    FSEND );
    }
    rWorksheet->endElement( XML_mergeCells );
}

// Hyperlinks =================================================================

XclExpHyperlink::XclExpHyperlink( const XclExpRoot& rRoot, const SvxURLField& rUrlField, const ScAddress& rScPos ) :
    XclExpRecord( EXC_ID_HLINK ),
    maScPos( rScPos ),
    mxVarData( new SvMemoryStream ),
    mnFlags( 0 )
{
    const OUString& rUrl = rUrlField.GetURL();
    const OUString& rRepr = rUrlField.GetRepresentation();
    INetURLObject aUrlObj( rUrl );
    const INetProtocol eProtocol = aUrlObj.GetProtocol();
    bool bWithRepr = !rRepr.isEmpty();
    XclExpStream aXclStrm( *mxVarData, rRoot );         // using in raw write mode.

    // description
    if( bWithRepr )
    {
        XclExpString aDescr( rRepr, XclStrFlags::ForceUnicode, 255 );
        aXclStrm << sal_uInt32( aDescr.Len() + 1 );     // string length + 1 trailing zero word
        aDescr.WriteBuffer( aXclStrm );                 // NO flags
        aXclStrm << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_DESCR;
        m_Repr = rRepr;
    }

    // file link or URL
    if( eProtocol == INetProtocol::File || eProtocol == INetProtocol::Smb )
    {
        sal_uInt16 nLevel;
        bool bRel;
        /* TODO: should we differentiate between BIFF and OOXML and write IURI
         * encoded for OOXML? */
        OUString aFileName( BuildFileName( nLevel, bRel, rUrl, rRoot, false ) );

        if( eProtocol == INetProtocol::Smb )
        {
            // #n382718# (and #n261623#) Convert smb notation to '\\'
            aFileName = aUrlObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            aFileName = aFileName.copy(4); // skip the 'smb:' part
            aFileName = aFileName.replace('/', '\\');
        }

        if( !bRel )
            mnFlags |= EXC_HLINK_ABS;
        mnFlags |= EXC_HLINK_BODY;

        OString aAsciiLink(OUStringToOString(aFileName,
            rRoot.GetTextEncoding()));
        XclExpString aLink( aFileName, XclStrFlags::ForceUnicode, 255 );
        aXclStrm    << XclTools::maGuidFileMoniker
                    << nLevel
                    << sal_uInt32( aAsciiLink.getLength() + 1 );      // string length + 1 trailing zero byte
        aXclStrm.Write( aAsciiLink.getStr(), aAsciiLink.getLength() );
        aXclStrm    << sal_uInt8( 0 )
                    << sal_uInt32( 0xDEADFFFF );
        aXclStrm.WriteZeroBytes( 20 );
        aXclStrm    << sal_uInt32( aLink.GetBufferSize() + 6 )
                    << sal_uInt32( aLink.GetBufferSize() )      // byte count, not string length
                    << sal_uInt16( 0x0003 );
        aLink.WriteBuffer( aXclStrm );                          // NO flags

        if (m_Repr.isEmpty())
            m_Repr = aFileName;

        msTarget = XclXmlUtils::ToOUString( aLink );

        if( bRel )
        {
            for( int i = 0; i < nLevel; ++i )
                msTarget = "../" + msTarget;
        }
        else
        {
            // ooxml expects the file:/// part appended ( or at least
            // ms2007 does, ms2010 is more tolerant )
            msTarget = "file:///" + msTarget;
        }
    }
    else if( eProtocol != INetProtocol::NotValid )
    {
        XclExpString aUrl( aUrlObj.GetURLNoMark(), XclStrFlags::ForceUnicode, 255 );
        aXclStrm    << XclTools::maGuidUrlMoniker
                    << sal_uInt32( aUrl.GetBufferSize() + 2 );  // byte count + 1 trailing zero word
        aUrl.WriteBuffer( aXclStrm );                           // NO flags
        aXclStrm    << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_BODY | EXC_HLINK_ABS;
        if (m_Repr.isEmpty())
            m_Repr = rUrl;

        msTarget = XclXmlUtils::ToOUString( aUrl );
    }
    else if( !rUrl.isEmpty() && rUrl[0] == '#' )     // hack for #89066#
    {
        OUString aTextMark( rUrl.copy( 1 ) );

        sal_Int32 nSepPos = aTextMark.lastIndexOf( '.' );
        if(nSepPos != -1)
            aTextMark = aTextMark.replaceAt( nSepPos, 1, "!" );
        else
            nSepPos = aTextMark.lastIndexOf( '!' );

        if(nSepPos != -1)
        {
            OUString aSheetName( aTextMark.copy(0, nSepPos));

            if ( aSheetName.indexOf(' ') != -1 && aSheetName[0] != '\'')
            {
                aTextMark = "'" + aTextMark.replaceAt(nSepPos, 0, "'");
            }
        }

        mxTextMark.reset( new XclExpString( aTextMark, XclStrFlags::ForceUnicode, 255 ) );
    }

    // text mark
    if( !mxTextMark.get() && aUrlObj.HasMark() )
        mxTextMark.reset( new XclExpString( aUrlObj.GetMark(), XclStrFlags::ForceUnicode, 255 ) );

    if( mxTextMark.get() )
    {
        aXclStrm    << sal_uInt32( mxTextMark->Len() + 1 );  // string length + 1 trailing zero word
        mxTextMark->WriteBuffer( aXclStrm );                 // NO flags
        aXclStrm    << sal_uInt16( 0 );

        mnFlags |= EXC_HLINK_MARK;
    }

    SetRecSize( 32 + mxVarData->Tell() );
}

XclExpHyperlink::~XclExpHyperlink()
{
}

OUString XclExpHyperlink::BuildFileName(
        sal_uInt16& rnLevel, bool& rbRel, const OUString& rUrl, const XclExpRoot& rRoot, bool bEncoded )
{
    INetURLObject aURLObject( rUrl );
    OUString aDosName(bEncoded ? aURLObject.GetMainURL(INetURLObject::DecodeMechanism::ToIUri)
                               : aURLObject.getFSysPath(FSysStyle::Dos));
    rnLevel = 0;
    rbRel = rRoot.IsRelUrl();

    if( rbRel )
    {
        // try to convert to relative file name
        OUString aTmpName( aDosName );
        aDosName = INetURLObject::GetRelURL( rRoot.GetBasePath(), rUrl,
            INetURLObject::EncodeMechanism::WasEncoded,
            (bEncoded ? INetURLObject::DecodeMechanism::ToIUri : INetURLObject::DecodeMechanism::WithCharset));

        if (aDosName.startsWith(INET_FILE_SCHEME))
        {
            // not converted to rel -> back to old, return absolute flag
            aDosName = aTmpName;
            rbRel = false;
        }
        else if (aDosName.startsWith("./"))
        {
            aDosName = aDosName.copy(2);
        }
        else
        {
            while (aDosName.startsWith("../"))
            {
                aDosName = aDosName.copy(3);
                ++rnLevel;
            }
        }
    }
    return aDosName;
}

void XclExpHyperlink::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nXclCol = static_cast< sal_uInt16 >( maScPos.Col() );
    sal_uInt16 nXclRow = static_cast< sal_uInt16 >( maScPos.Row() );
    rStrm   << nXclRow << nXclRow << nXclCol << nXclCol;
    WriteEmbeddedData( rStrm );
}

void XclExpHyperlink::WriteEmbeddedData( XclExpStream& rStrm )
{
    rStrm << XclTools::maGuidStdLink
            << sal_uInt32( 2 )
            << mnFlags;

    mxVarData->Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( *mxVarData );
}

void XclExpHyperlink::SaveXml( XclExpXmlStream& rStrm )
{
    OUString sId = !msTarget.isEmpty() ? rStrm.addRelation( rStrm.GetCurrentStream()->getOutputStream(),
            oox::getRelationship(Relationship::HYPERLINK),
            msTarget, true ) : OUString();
    rStrm.GetCurrentStream()->singleElement( XML_hyperlink,
            XML_ref,                XclXmlUtils::ToOString( maScPos ).getStr(),
            FSNS( XML_r, XML_id ),  !sId.isEmpty()
                                       ? XclXmlUtils::ToOString( sId ).getStr()
                                       : nullptr,
            XML_location,           mxTextMark.get() != nullptr
                                        ? XclXmlUtils::ToOString( *mxTextMark ).getStr()
                                        : nullptr,
            // OOXTODO: XML_tooltip,    from record HLinkTooltip 800h wzTooltip
            XML_display,            XclXmlUtils::ToOString(m_Repr).getStr(),
            FSEND );
}

// Label ranges ===============================================================

XclExpLabelranges::XclExpLabelranges( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
    SCTAB nScTab = GetCurrScTab();
    // row label ranges
    FillRangeList( maRowRanges, rRoot.GetDoc().GetRowNameRangesRef(), nScTab );
    // row labels only over 1 column (restriction of Excel97/2000/XP)
    for ( size_t i = 0, nRanges = maRowRanges.size(); i < nRanges; ++i )
    {
        ScRange & rScRange = maRowRanges[ i ];
        if( rScRange.aStart.Col() != rScRange.aEnd.Col() )
            rScRange.aEnd.SetCol( rScRange.aStart.Col() );
    }
    // col label ranges
    FillRangeList( maColRanges, rRoot.GetDoc().GetColNameRangesRef(), nScTab );
}

void XclExpLabelranges::FillRangeList( ScRangeList& rScRanges,
        const ScRangePairListRef& xLabelRangesRef, SCTAB nScTab )
{
    for ( size_t i = 0, nPairs = xLabelRangesRef->size(); i < nPairs; ++i )
    {
        const ScRangePair & rRangePair = (*xLabelRangesRef)[i];
        const ScRange& rScRange = rRangePair.GetRange( 0 );
        if( rScRange.aStart.Tab() == nScTab )
            rScRanges.push_back( rScRange );
    }
}

void XclExpLabelranges::Save( XclExpStream& rStrm )
{
    XclExpAddressConverter& rAddrConv = GetAddressConverter();
    XclRangeList aRowXclRanges, aColXclRanges;
    rAddrConv.ConvertRangeList( aRowXclRanges, maRowRanges, false );
    rAddrConv.ConvertRangeList( aColXclRanges, maColRanges, false );
    if( !aRowXclRanges.empty() || !aColXclRanges.empty() )
    {
        rStrm.StartRecord( EXC_ID_LABELRANGES, 4 + 8 * (aRowXclRanges.size() + aColXclRanges.size()) );
        rStrm << aRowXclRanges << aColXclRanges;
        rStrm.EndRecord();
    }
}

// Conditional formatting  ====================================================

/** Represents a CF record that contains one condition of a conditional format. */
class XclExpCFImpl : protected XclExpRoot
{
public:
    explicit            XclExpCFImpl( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormatEntry, sal_Int32 nPriority );

    /** Writes the body of the CF record. */
    void                WriteBody( XclExpStream& rStrm );
    void                SaveXml( XclExpXmlStream& rStrm );

private:
    const ScCondFormatEntry& mrFormatEntry; /// Calc conditional format entry.
    XclFontData         maFontData;         /// Font formatting attributes.
    XclExpCellBorder    maBorder;           /// Border formatting attributes.
    XclExpCellArea      maArea;             /// Pattern formatting attributes.
    XclTokenArrayRef    mxTokArr1;          /// Formula for first condition.
    XclTokenArrayRef    mxTokArr2;          /// Formula for second condition.
    sal_uInt32          mnFontColorId;      /// Font color ID.
    sal_uInt8           mnType;             /// Type of the condition (cell/formula).
    sal_uInt8           mnOperator;         /// Comparison operator for cell type.
    sal_Int32           mnPriority;         /// Priority of this entry; needed for oox export
    bool                mbFontUsed;         /// true = Any font attribute used.
    bool                mbHeightUsed;       /// true = Font height used.
    bool                mbWeightUsed;       /// true = Font weight used.
    bool                mbColorUsed;        /// true = Font color used.
    bool                mbUnderlUsed;       /// true = Font underline type used.
    bool                mbItalicUsed;       /// true = Font posture used.
    bool                mbStrikeUsed;       /// true = Font strikeout used.
    bool                mbBorderUsed;       /// true = Border attribute used.
    bool                mbPattUsed;         /// true = Pattern attribute used.
    bool                mbFormula2;
};

XclExpCFImpl::XclExpCFImpl( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormatEntry, sal_Int32 nPriority ) :
    XclExpRoot( rRoot ),
    mrFormatEntry( rFormatEntry ),
    mnFontColorId( 0 ),
    mnType( EXC_CF_TYPE_CELL ),
    mnOperator( EXC_CF_CMP_NONE ),
    mnPriority( nPriority ),
    mbFontUsed( false ),
    mbHeightUsed( false ),
    mbWeightUsed( false ),
    mbColorUsed( false ),
    mbUnderlUsed( false ),
    mbItalicUsed( false ),
    mbStrikeUsed( false ),
    mbBorderUsed( false ),
    mbPattUsed( false ),
    mbFormula2(false)
{
    /*  Get formatting attributes here, and not in WriteBody(). This is needed to
        correctly insert all colors into the palette. */

    if( SfxStyleSheetBase* pStyleSheet = GetDoc().GetStyleSheetPool()->Find( mrFormatEntry.GetStyle(), SfxStyleFamily::Para ) )
    {
        const SfxItemSet& rItemSet = pStyleSheet->GetItemSet();

        // font
        mbHeightUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_HEIGHT,     true );
        mbWeightUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_WEIGHT,     true );
        mbColorUsed  = ScfTools::CheckItem( rItemSet, ATTR_FONT_COLOR,      true );
        mbUnderlUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_UNDERLINE,  true );
        mbItalicUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_POSTURE,    true );
        mbStrikeUsed = ScfTools::CheckItem( rItemSet, ATTR_FONT_CROSSEDOUT, true );
        mbFontUsed = mbHeightUsed || mbWeightUsed || mbColorUsed || mbUnderlUsed || mbItalicUsed || mbStrikeUsed;
        if( mbFontUsed )
        {
            vcl::Font aFont;
            ScPatternAttr::GetFont( aFont, rItemSet, SC_AUTOCOL_RAW );
            maFontData.FillFromVclFont( aFont );
            mnFontColorId = GetPalette().InsertColor( maFontData.maColor, EXC_COLOR_CELLTEXT );
        }

        // border
        mbBorderUsed = ScfTools::CheckItem( rItemSet, ATTR_BORDER, true );
        if( mbBorderUsed )
            maBorder.FillFromItemSet( rItemSet, GetPalette(), GetBiff() );

        // pattern
        mbPattUsed = ScfTools::CheckItem( rItemSet, ATTR_BACKGROUND, true );
        if( mbPattUsed )
            maArea.FillFromItemSet( rItemSet, GetPalette(), true );
    }

    // *** mode and comparison operator ***

    switch( rFormatEntry.GetOperation() )
    {
        case ScConditionMode::NONE:
            mnType = EXC_CF_TYPE_NONE;
        break;
        case ScConditionMode::Between:
            mnOperator = EXC_CF_CMP_BETWEEN;
            mbFormula2 = true;
        break;
        case ScConditionMode::NotBetween:
            mnOperator = EXC_CF_CMP_NOT_BETWEEN;
            mbFormula2 = true;
        break;
        case ScConditionMode::Equal:
            mnOperator = EXC_CF_CMP_EQUAL;
        break;
        case ScConditionMode::NotEqual:
            mnOperator = EXC_CF_CMP_NOT_EQUAL;
        break;
        case ScConditionMode::Greater:
            mnOperator = EXC_CF_CMP_GREATER;
        break;
        case ScConditionMode::Less:
            mnOperator = EXC_CF_CMP_LESS;
        break;
        case ScConditionMode::EqGreater:
            mnOperator = EXC_CF_CMP_GREATER_EQUAL;
        break;
        case ScConditionMode::EqLess:
            mnOperator = EXC_CF_CMP_LESS_EQUAL;
        break;
        case ScConditionMode::Direct:
            mnType = EXC_CF_TYPE_FMLA;
        break;
        default:
            mnType = EXC_CF_TYPE_NONE;
            OSL_FAIL( "XclExpCF::WriteBody - unknown condition type" );
    }
}

void XclExpCFImpl::WriteBody( XclExpStream& rStrm )
{

    // *** formulas ***

    XclExpFormulaCompiler& rFmlaComp = GetFormulaCompiler();

    std::unique_ptr< ScTokenArray > xScTokArr( mrFormatEntry.CreateFlatCopiedTokenArray( 0 ) );
    mxTokArr1 = rFmlaComp.CreateFormula( EXC_FMLATYPE_CONDFMT, *xScTokArr );

    if (mbFormula2)
    {
        xScTokArr.reset( mrFormatEntry.CreateFlatCopiedTokenArray( 1 ) );
        mxTokArr2 = rFmlaComp.CreateFormula( EXC_FMLATYPE_CONDFMT, *xScTokArr );
    }

    // *** mode and comparison operator ***

    rStrm << mnType << mnOperator;

    // *** formula sizes ***

    sal_uInt16 nFmlaSize1 = mxTokArr1.get() ? mxTokArr1->GetSize() : 0;
    sal_uInt16 nFmlaSize2 = mxTokArr2.get() ? mxTokArr2->GetSize() : 0;
    rStrm << nFmlaSize1 << nFmlaSize2;

    // *** formatting blocks ***

    if( mbFontUsed || mbBorderUsed || mbPattUsed )
    {
        sal_uInt32 nFlags = EXC_CF_ALLDEFAULT;

        ::set_flag( nFlags, EXC_CF_BLOCK_FONT,   mbFontUsed );
        ::set_flag( nFlags, EXC_CF_BLOCK_BORDER, mbBorderUsed );
        ::set_flag( nFlags, EXC_CF_BLOCK_AREA,   mbPattUsed );

        // attributes used -> set flags to 0.
        ::set_flag( nFlags, EXC_CF_BORDER_ALL, !mbBorderUsed );
        ::set_flag( nFlags, EXC_CF_AREA_ALL,   !mbPattUsed );

        rStrm << nFlags << sal_uInt16( 0 );

        if( mbFontUsed )
        {
            // font height, 0xFFFFFFFF indicates unused
            sal_uInt32 nHeight = mbHeightUsed ? maFontData.mnHeight : 0xFFFFFFFF;
            // font style: italic and strikeout
            sal_uInt32 nStyle = 0;
            ::set_flag( nStyle, EXC_CF_FONT_STYLE,     maFontData.mbItalic );
            ::set_flag( nStyle, EXC_CF_FONT_STRIKEOUT, maFontData.mbStrikeout );
            // font color, 0xFFFFFFFF indicates unused
            sal_uInt32 nColor = mbColorUsed ? GetPalette().GetColorIndex( mnFontColorId ) : 0xFFFFFFFF;
            // font used flags for italic, weight, and strikeout -> 0 = used, 1 = default
            sal_uInt32 nFontFlags1 = EXC_CF_FONT_ALLDEFAULT;
            ::set_flag( nFontFlags1, EXC_CF_FONT_STYLE, !(mbItalicUsed || mbWeightUsed) );
            ::set_flag( nFontFlags1, EXC_CF_FONT_STRIKEOUT, !mbStrikeUsed );
            // font used flag for underline -> 0 = used, 1 = default
            sal_uInt32 nFontFlags3 = mbUnderlUsed ? 0 : EXC_CF_FONT_UNDERL;

            rStrm.WriteZeroBytesToRecord( 64 );
            rStrm   << nHeight
                    << nStyle
                    << maFontData.mnWeight
                    << EXC_FONTESC_NONE
                    << maFontData.mnUnderline;
            rStrm.WriteZeroBytesToRecord( 3 );
            rStrm   << nColor
                    << sal_uInt32( 0 )
                    << nFontFlags1
                    << EXC_CF_FONT_ESCAPEM      // escapement never used -> set the flag
                    << nFontFlags3;
            rStrm.WriteZeroBytesToRecord( 16 );
            rStrm   << sal_uInt16( 1 );         // must be 1
        }

        if( mbBorderUsed )
        {
            sal_uInt16 nLineStyle = 0;
            sal_uInt32 nLineColor = 0;
            maBorder.SetFinalColors( GetPalette() );
            maBorder.FillToCF8( nLineStyle, nLineColor );
            rStrm << nLineStyle << nLineColor << sal_uInt16( 0 );
        }

        if( mbPattUsed )
        {
            sal_uInt16 nPattern = 0, nColor = 0;
            maArea.SetFinalColors( GetPalette() );
            maArea.FillToCF8( nPattern, nColor );
            rStrm << nPattern << nColor;
        }
    }
    else
    {
        // no data blocks at all
        rStrm << sal_uInt32( 0 ) << sal_uInt16( 0 );
    }

    // *** formulas ***

    if( mxTokArr1.get() )
        mxTokArr1->WriteArray( rStrm );
    if( mxTokArr2.get() )
        mxTokArr2->WriteArray( rStrm );
}

namespace {

const char* GetOperatorString(ScConditionMode eMode, bool& bFrmla2)
{
    const char *pRet = nullptr;
    switch(eMode)
    {
        case ScConditionMode::Equal:
            pRet = "equal";
            break;
        case ScConditionMode::Less:
            pRet = "lessThan";
            break;
        case ScConditionMode::Greater:
            pRet = "greaterThan";
            break;
        case ScConditionMode::EqLess:
            pRet = "lessThanOrEqual";
            break;
        case ScConditionMode::EqGreater:
            pRet = "greaterThanOrEqual";
            break;
        case ScConditionMode::NotEqual:
            pRet = "notEqual";
            break;
        case ScConditionMode::Between:
            bFrmla2 = true;
            pRet = "between";
            break;
        case ScConditionMode::NotBetween:
            bFrmla2 = true;
            pRet = "notBetween";
            break;
        case ScConditionMode::Duplicate:
            pRet = nullptr;
            break;
        case ScConditionMode::NotDuplicate:
            pRet = nullptr;
            break;
        case ScConditionMode::BeginsWith:
            pRet = "beginsWith";
        break;
        case ScConditionMode::EndsWith:
            pRet = "endsWith";
        break;
        case ScConditionMode::ContainsText:
            pRet = "containsText";
        break;
        case ScConditionMode::NotContainsText:
            pRet = "notContains";
        break;
        case ScConditionMode::Direct:
            break;
        case ScConditionMode::NONE:
        default:
            break;
    }
    return pRet;
}

const char* GetTypeString(ScConditionMode eMode)
{
    switch(eMode)
    {
        case ScConditionMode::Direct:
            return "expression";
        case ScConditionMode::Top10:
        case ScConditionMode::TopPercent:
        case ScConditionMode::Bottom10:
        case ScConditionMode::BottomPercent:
            return "top10";
        case ScConditionMode::AboveAverage:
        case ScConditionMode::BelowAverage:
        case ScConditionMode::AboveEqualAverage:
        case ScConditionMode::BelowEqualAverage:
            return "aboveAverage";
        case ScConditionMode::NotDuplicate:
            return "uniqueValues";
        case ScConditionMode::Duplicate:
            return "duplicateValues";
        case ScConditionMode::Error:
            return "containsErrors";
        case ScConditionMode::NoError:
            return "notContainsErrors";
        case ScConditionMode::BeginsWith:
            return "beginsWith";
        case ScConditionMode::EndsWith:
            return "endsWith";
        case ScConditionMode::ContainsText:
            return "containsText";
        case ScConditionMode::NotContainsText:
            return "notContainsText";
        default:
            return "cellIs";
    }
}

bool IsTopBottomRule(ScConditionMode eMode)
{
    switch(eMode)
    {
        case ScConditionMode::Top10:
        case ScConditionMode::Bottom10:
        case ScConditionMode::TopPercent:
        case ScConditionMode::BottomPercent:
            return true;
        default:
            break;
    }

    return false;
}

bool IsTextRule(ScConditionMode eMode)
{
    switch(eMode)
    {
        case ScConditionMode::BeginsWith:
        case ScConditionMode::EndsWith:
        case ScConditionMode::ContainsText:
        case ScConditionMode::NotContainsText:
            return true;
        default:
            break;
    }

    return false;
}

bool RequiresFormula(ScConditionMode eMode)
{
    if (IsTopBottomRule(eMode))
        return false;
    else if (IsTextRule(eMode))
        return false;

    switch (eMode)
    {
        case ScConditionMode::NoError:
        case ScConditionMode::Error:
        case ScConditionMode::Duplicate:
        case ScConditionMode::NotDuplicate:
            return false;
        default:
        break;
    }

    return true;
}

bool RequiresFixedFormula(ScConditionMode eMode)
{
    switch(eMode)
    {
        case ScConditionMode::NoError:
        case ScConditionMode::Error:
        case ScConditionMode::BeginsWith:
        case ScConditionMode::EndsWith:
        case ScConditionMode::ContainsText:
        case ScConditionMode::NotContainsText:
            return true;
        default:
        break;
    }

    return false;
}

OString GetFixedFormula(ScConditionMode eMode, const ScAddress& rAddress, const OString& rText)
{
    OStringBuffer aBuffer;
    OStringBuffer aPosBuffer = XclXmlUtils::ToOString(aBuffer, rAddress);
    OString aPos = aPosBuffer.makeStringAndClear();
    switch (eMode)
    {
        case ScConditionMode::Error:
            return OString("ISERROR(" + aPos + ")") ;
        case ScConditionMode::NoError:
            return OString("NOT(ISERROR(" + aPos + "))") ;
        case ScConditionMode::BeginsWith:
            return OString("LEFT(" + aPos + ",LEN(\"" + rText + "\"))=\"" + rText + "\"");
        case ScConditionMode::EndsWith:
            return OString("RIGHT(" + aPos +",LEN(\"" + rText + "\"))=\"" + rText + "\"");
        case ScConditionMode::ContainsText:
            return OString("NOT(ISERROR(SEARCH(\"" + rText + "\"," + aPos + ")))");
        case ScConditionMode::NotContainsText:
            return OString("ISERROR(SEARCH(\"" +  rText + "\"," + aPos + "))");
        default:
        break;
    }

    return OString("");
}

}

void XclExpCFImpl::SaveXml( XclExpXmlStream& rStrm )
{
    bool bFmla2 = false;
    ScConditionMode eOperation = mrFormatEntry.GetOperation();
    bool bAboveAverage = eOperation == ScConditionMode::AboveAverage ||
                                eOperation == ScConditionMode::AboveEqualAverage;
    bool bEqualAverage = eOperation == ScConditionMode::AboveEqualAverage ||
                                eOperation == ScConditionMode::BelowEqualAverage;
    bool bBottom = eOperation == ScConditionMode::Bottom10
        || eOperation == ScConditionMode::BottomPercent;
    bool bPercent = eOperation == ScConditionMode::TopPercent ||
        eOperation == ScConditionMode::BottomPercent;
    OString aRank("0");
    if(IsTopBottomRule(eOperation))
    {
        // position and formula grammar are not important
        // we only store a number there
        aRank = XclXmlUtils::ToOString(mrFormatEntry.GetExpression(ScAddress(0,0,0), 0));
    }
    OString aText;
    if(IsTextRule(eOperation))
    {
        // we need to write the text without quotes
        // we have to actually get the string from
        // the token array for that
        std::unique_ptr<ScTokenArray> pTokenArray(mrFormatEntry.CreateFlatCopiedTokenArray(0));
        if(pTokenArray->GetLen())
            aText = XclXmlUtils::ToOString(pTokenArray->FirstToken()->GetString().getString());
    }

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_cfRule,
            XML_type, GetTypeString( mrFormatEntry.GetOperation() ),
            XML_priority, OString::number( mnPriority + 1 ).getStr(),
            XML_operator, GetOperatorString( mrFormatEntry.GetOperation(), bFmla2 ),
            XML_aboveAverage, OString::number( int(bAboveAverage) ).getStr(),
            XML_equalAverage, OString::number( int(bEqualAverage) ).getStr(),
            XML_bottom, OString::number( int(bBottom) ).getStr(),
            XML_percent, OString::number( int(bPercent) ).getStr(),
            XML_rank, aRank.getStr(),
            XML_text, aText.getStr(),
            XML_dxfId, OString::number( GetDxfs().GetDxfId( mrFormatEntry.GetStyle() ) ).getStr(),
            FSEND );

    if (RequiresFixedFormula(eOperation))
    {
        rWorksheet->startElement( XML_formula, FSEND );
        OString aFormula = GetFixedFormula(eOperation, mrFormatEntry.GetValidSrcPos(), aText);
        rWorksheet->writeEscaped(aFormula.getStr());
        rWorksheet->endElement( XML_formula );
    }
    else if(RequiresFormula(eOperation))
    {
        rWorksheet->startElement( XML_formula, FSEND );
        std::unique_ptr<ScTokenArray> pTokenArray(mrFormatEntry.CreateFlatCopiedTokenArray(0));
        rWorksheet->writeEscaped(XclXmlUtils::ToOUString( GetCompileFormulaContext(), mrFormatEntry.GetValidSrcPos(),
                    pTokenArray.get()));
        rWorksheet->endElement( XML_formula );
        if (bFmla2)
        {
            rWorksheet->startElement( XML_formula, FSEND );
            std::unique_ptr<ScTokenArray> pTokenArray2(mrFormatEntry.CreateFlatCopiedTokenArray(1));
            rWorksheet->writeEscaped(XclXmlUtils::ToOUString( GetCompileFormulaContext(), mrFormatEntry.GetValidSrcPos(),
                        pTokenArray2.get()));
            rWorksheet->endElement( XML_formula );
        }
    }
    // OOXTODO: XML_extLst
    rWorksheet->endElement( XML_cfRule );
}

XclExpCF::XclExpCF( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormatEntry, sal_Int32 nPriority = 0 ) :
    XclExpRecord( EXC_ID_CF ),
    XclExpRoot( rRoot ),
    mxImpl( new XclExpCFImpl( rRoot, rFormatEntry, nPriority ) )
{
}

XclExpCF::~XclExpCF()
{
}

void XclExpCF::WriteBody( XclExpStream& rStrm )
{
    mxImpl->WriteBody( rStrm );
}

void XclExpCF::SaveXml( XclExpXmlStream& rStrm )
{
    mxImpl->SaveXml( rStrm );
}

XclExpDateFormat::XclExpDateFormat( const XclExpRoot& rRoot, const ScCondDateFormatEntry& rFormatEntry, sal_Int32 nPriority ):
    XclExpRecord( EXC_ID_CF ),
    XclExpRoot( rRoot ),
    mrFormatEntry(rFormatEntry),
    mnPriority(nPriority)
{
}

XclExpDateFormat::~XclExpDateFormat()
{
}

namespace {

const char* getTimePeriodString( condformat::ScCondFormatDateType eType )
{
    switch(eType)
    {
        case condformat::TODAY:
            return "today";
        case condformat::YESTERDAY:
            return "yesterday";
        case condformat::TOMORROW:
            return "yesterday";
        case condformat::THISWEEK:
            return "thisWeek";
        case condformat::LASTWEEK:
            return "lastWeek";
        case condformat::NEXTWEEK:
            return "nextWeek";
        case condformat::THISMONTH:
            return "thisMonth";
        case condformat::LASTMONTH:
            return "lastMonth";
        case condformat::NEXTMONTH:
            return "nextMonth";
        case condformat::LAST7DAYS:
            return "last7Days";
        default:
            break;
    }
    return nullptr;
}

}

void XclExpDateFormat::SaveXml( XclExpXmlStream& rStrm )
{
    // only write the supported entries into OOXML
    const char* sTimePeriod = getTimePeriodString(mrFormatEntry.GetDateType());
    if(!sTimePeriod)
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_cfRule,
            XML_type, "timePeriod",
            XML_priority, OString::number( mnPriority + 1 ).getStr(),
            XML_timePeriod, sTimePeriod,
            XML_dxfId, OString::number( GetDxfs().GetDxfId( mrFormatEntry.GetStyleName() ) ).getStr(),
            FSEND );
    rWorksheet->endElement( XML_cfRule);
}

XclExpCfvo::XclExpCfvo(const XclExpRoot& rRoot, const ScColorScaleEntry& rEntry, const ScAddress& rAddr, bool bFirst):
    XclExpRecord(),
    XclExpRoot( rRoot ),
    mrEntry(rEntry),
    maSrcPos(rAddr),
    mbFirst(bFirst)
{
}

namespace {

OString getColorScaleType( const ScColorScaleEntry& rEntry, bool bFirst )
{
    switch(rEntry.GetType())
    {
        case COLORSCALE_MIN:
            return "min";
        case COLORSCALE_MAX:
            return "max";
        case COLORSCALE_PERCENT:
            return "percent";
        case COLORSCALE_FORMULA:
            return "formula";
        case COLORSCALE_AUTO:
            if(bFirst)
                return "min";
            else
                return "max";
        case COLORSCALE_PERCENTILE:
            return "percentile";
        default:
            break;
    }

    return "num";
}

}

void XclExpCfvo::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    OString aValue;
    if(mrEntry.GetType() == COLORSCALE_FORMULA)
    {
        OUString aFormula = XclXmlUtils::ToOUString( GetCompileFormulaContext(), maSrcPos,
                mrEntry.GetFormula());
        aValue = OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8 );
    }
    else
    {
        aValue = OString::number( mrEntry.GetValue() );
    }

    rWorksheet->startElement( XML_cfvo,
            XML_type, getColorScaleType(mrEntry, mbFirst).getStr(),
            XML_val, aValue.getStr(),
            FSEND );

    rWorksheet->endElement( XML_cfvo );
}

XclExpColScaleCol::XclExpColScaleCol( const XclExpRoot& rRoot, const Color& rColor ):
    XclExpRecord(),
    XclExpRoot( rRoot ),
    mrColor( rColor )
{
}

XclExpColScaleCol::~XclExpColScaleCol()
{
}

void XclExpColScaleCol::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    rWorksheet->startElement( XML_color,
            XML_rgb, XclXmlUtils::ToOString( mrColor ).getStr(),
            FSEND );

    rWorksheet->endElement( XML_color );
}

namespace {

OString createHexStringFromDigit(sal_uInt8 nDigit)
{
    OString aString = OString::number( nDigit, 16 );
    if(aString.getLength() == 1)
        aString = aString + OString::number(0);
    return aString;
}

OString createGuidStringFromInt(sal_uInt8 nGuid[16])
{
    OStringBuffer aBuffer;
    aBuffer.append('{');
    for(size_t i = 0; i < 16; ++i)
    {
        aBuffer.append(createHexStringFromDigit(nGuid[i]));
        if(i == 3|| i == 5 || i == 7 || i == 9 )
            aBuffer.append('-');
    }
    aBuffer.append('}');
    OString aString = aBuffer.makeStringAndClear();
    return aString.toAsciiUpperCase();
}

OString generateGUIDString()
{
    sal_uInt8 nGuid[16];
    rtl_createUuid(nGuid, nullptr, true);
    return createGuidStringFromInt(nGuid);
}

}

XclExpCondfmt::XclExpCondfmt( const XclExpRoot& rRoot, const ScConditionalFormat& rCondFormat, const XclExtLstRef& xExtLst, sal_Int32& rIndex ) :
    XclExpRecord( EXC_ID_CONDFMT ),
    XclExpRoot( rRoot )
{
    const ScRangeList& aScRanges = rCondFormat.GetRange();
    GetAddressConverter().ConvertRangeList( maXclRanges, aScRanges, true );
    if( !maXclRanges.empty() )
    {
        std::vector<XclExpExtCondFormatData> aExtEntries;
        for( size_t nIndex = 0, nCount = rCondFormat.size(); nIndex < nCount; ++nIndex )
            if( const ScFormatEntry* pFormatEntry = rCondFormat.GetEntry( nIndex ) )
            {
                if(pFormatEntry->GetType() == ScFormatEntry::Type::Condition)
                    maCFList.AppendNewRecord( new XclExpCF( GetRoot(), static_cast<const ScCondFormatEntry&>(*pFormatEntry), ++rIndex ) );
                else if(pFormatEntry->GetType() == ScFormatEntry::Type::Colorscale)
                    maCFList.AppendNewRecord( new XclExpColorScale( GetRoot(), static_cast<const ScColorScaleFormat&>(*pFormatEntry), ++rIndex ) );
                else if(pFormatEntry->GetType() == ScFormatEntry::Type::Databar)
                {
                    const ScDataBarFormat& rFormat = static_cast<const ScDataBarFormat&>(*pFormatEntry);
                    XclExpExtCondFormatData aExtEntry;
                    aExtEntry.nPriority = -1;
                    aExtEntry.aGUID = generateGUIDString();
                    aExtEntry.pEntry = &rFormat;
                    aExtEntries.push_back(aExtEntry);

                    maCFList.AppendNewRecord( new XclExpDataBar( GetRoot(), rFormat, ++rIndex, aExtEntry.aGUID));
                }
                else if(pFormatEntry->GetType() == ScFormatEntry::Type::Iconset)
                {
                    // don't export iconSet entries that are not in OOXML
                    const ScIconSetFormat& rIconSet = static_cast<const ScIconSetFormat&>(*pFormatEntry);
                    bool bNeedsExt = false;
                    switch (rIconSet.GetIconSetData()->eIconSetType)
                    {
                        case IconSet_3Smilies:
                        case IconSet_3ColorSmilies:
                        case IconSet_3Stars:
                        case IconSet_3Triangles:
                        case IconSet_5Boxes:
                        {
                            bNeedsExt = true;
                        }
                        break;
                        default:
                            break;
                    }

                    bNeedsExt |= rIconSet.GetIconSetData()->mbCustom;

                    if (bNeedsExt)
                    {
                        XclExpExtCondFormatData aExtEntry;
                        aExtEntry.nPriority = ++rIndex;
                        aExtEntry.aGUID = generateGUIDString();
                        aExtEntry.pEntry = &rIconSet;
                        aExtEntries.push_back(aExtEntry);
                    }
                    else
                        maCFList.AppendNewRecord( new XclExpIconSet( GetRoot(), rIconSet, ++rIndex ) );
                }
                else if(pFormatEntry->GetType() == ScFormatEntry::Type::Date)
                    maCFList.AppendNewRecord( new XclExpDateFormat( GetRoot(), static_cast<const ScCondDateFormatEntry&>(*pFormatEntry), ++rIndex ) );
            }
        aScRanges.Format( msSeqRef, ScRefFlags::VALID, nullptr, formula::FormulaGrammar::CONV_XL_OOX, ' ', true );

        if(!aExtEntries.empty() && xExtLst.get())
        {
            XclExpExtRef pParent = xExtLst->GetItem( XclExpExtDataBarType );
            if( !pParent.get() )
            {
                xExtLst->AddRecord( XclExpExtRef(new XclExpExtCondFormat( *xExtLst )) );
                pParent = xExtLst->GetItem( XclExpExtDataBarType );
            }
            static_cast<XclExpExtCondFormat*>(xExtLst->GetItem( XclExpExtDataBarType ).get())->AddRecord(
                    std::make_shared<XclExpExtConditionalFormatting>( *pParent, aExtEntries, aScRanges));
        }
    }
}

XclExpCondfmt::~XclExpCondfmt()
{
}

bool XclExpCondfmt::IsValidForBinary() const
{
    // ccf (2 bytes): An unsigned integer that specifies the count of CF records that follow this
    // record. MUST be greater than or equal to 0x0001, and less than or equal to 0x0003.

    SAL_WARN_IF( maCFList.GetSize() > 3, "sc.filter", "More than 3 conditional filters for cell(s), won't export");

    return !maCFList.IsEmpty() && maCFList.GetSize() <= 3 && !maXclRanges.empty();
}

bool XclExpCondfmt::IsValidForXml() const
{
    return !maCFList.IsEmpty() && !maXclRanges.empty();
}

void XclExpCondfmt::Save( XclExpStream& rStrm )
{
    if( IsValidForBinary() )
    {
        XclExpRecord::Save( rStrm );
        maCFList.Save( rStrm );
    }
}

void XclExpCondfmt::WriteBody( XclExpStream& rStrm )
{
    OSL_ENSURE( !maCFList.IsEmpty(), "XclExpCondfmt::WriteBody - no CF records to write" );
    OSL_ENSURE( !maXclRanges.empty(), "XclExpCondfmt::WriteBody - no cell ranges found" );

    rStrm   << static_cast< sal_uInt16 >( maCFList.GetSize() )
            << sal_uInt16( 1 )
            << maXclRanges.GetEnclosingRange()
            << maXclRanges;
}

void XclExpCondfmt::SaveXml( XclExpXmlStream& rStrm )
{
    if( !IsValidForXml() )
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_conditionalFormatting,
            XML_sqref, XclXmlUtils::ToOString( msSeqRef ).getStr(),
            // OOXTODO: XML_pivot,
            FSEND );

    maCFList.SaveXml( rStrm );

    rWorksheet->endElement( XML_conditionalFormatting );
}

XclExpColorScale::XclExpColorScale( const XclExpRoot& rRoot, const ScColorScaleFormat& rFormat, sal_Int32 nPriority ):
    XclExpRecord(),
    XclExpRoot( rRoot ),
    mnPriority( nPriority )
{
    const ScRange & rRange = rFormat.GetRange().front();
    ScAddress aAddr = rRange.aStart;
    for(const auto& rxColorScaleEntry : rFormat)
    {
        // exact position is not important, we allow only absolute refs

        XclExpCfvoList::RecordRefType xCfvo( new XclExpCfvo( GetRoot(), *rxColorScaleEntry, aAddr ) );
        maCfvoList.AppendRecord( xCfvo );
        XclExpColScaleColList::RecordRefType xClo( new XclExpColScaleCol( GetRoot(), rxColorScaleEntry->GetColor() ) );
        maColList.AppendRecord( xClo );
    }
}

void XclExpColorScale::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    rWorksheet->startElement( XML_cfRule,
            XML_type, "colorScale",
            XML_priority, OString::number( mnPriority + 1 ).getStr(),
            FSEND );

    rWorksheet->startElement( XML_colorScale, FSEND );

    maCfvoList.SaveXml(rStrm);
    maColList.SaveXml(rStrm);

    rWorksheet->endElement( XML_colorScale );

    rWorksheet->endElement( XML_cfRule );
}

XclExpDataBar::XclExpDataBar( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, sal_Int32 nPriority, const OString& rGUID):
    XclExpRecord(),
    XclExpRoot( rRoot ),
    mrFormat( rFormat ),
    mnPriority( nPriority ),
    maGUID(rGUID)
{
    const ScRange & rRange = rFormat.GetRange().front();
    ScAddress aAddr = rRange.aStart;
    // exact position is not important, we allow only absolute refs
    mpCfvoLowerLimit.reset(
        new XclExpCfvo(GetRoot(), *mrFormat.GetDataBarData()->mpLowerLimit, aAddr, true));
    mpCfvoUpperLimit.reset(
        new XclExpCfvo(GetRoot(), *mrFormat.GetDataBarData()->mpUpperLimit, aAddr, false));

    mpCol.reset( new XclExpColScaleCol( GetRoot(), mrFormat.GetDataBarData()->maPositiveColor ) );
}

void XclExpDataBar::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    rWorksheet->startElement( XML_cfRule,
            XML_type, "dataBar",
            XML_priority, OString::number( mnPriority + 1 ).getStr(),
            FSEND );

    rWorksheet->startElement( XML_dataBar,
                                XML_showValue, OString::number(int(!mrFormat.GetDataBarData()->mbOnlyBar)),
                                XML_minLength, OString::number(sal_uInt32(mrFormat.GetDataBarData()->mnMinLength)),
                                XML_maxLength, OString::number(sal_uInt32(mrFormat.GetDataBarData()->mnMaxLength)),
            FSEND );

    mpCfvoLowerLimit->SaveXml(rStrm);
    mpCfvoUpperLimit->SaveXml(rStrm);
    mpCol->SaveXml(rStrm);

    rWorksheet->endElement( XML_dataBar );

    // extLst entries for Excel 2010 and 2013
    rWorksheet->startElement( XML_extLst, FSEND );
    rWorksheet->startElement( XML_ext,
                                FSNS( XML_xmlns, XML_x14 ), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls14Lst))).getStr(),
                                XML_uri, "{B025F937-C7B1-47D3-B67F-A62EFF666E3E}",
                                FSEND );

    rWorksheet->startElementNS( XML_x14, XML_id, FSEND );
    rWorksheet->write( maGUID.getStr() );
    rWorksheet->endElementNS( XML_x14, XML_id );

    rWorksheet->endElement( XML_ext );
    rWorksheet->endElement( XML_extLst );

    rWorksheet->endElement( XML_cfRule );
}

XclExpIconSet::XclExpIconSet( const XclExpRoot& rRoot, const ScIconSetFormat& rFormat, sal_Int32 nPriority ):
    XclExpRecord(),
    XclExpRoot( rRoot ),
    mrFormat( rFormat ),
    mnPriority( nPriority )
{
    const ScRange & rRange = rFormat.GetRange().front();
    ScAddress aAddr = rRange.aStart;
    for (auto const& itr : rFormat)
    {
        // exact position is not important, we allow only absolute refs

        XclExpCfvoList::RecordRefType xCfvo( new XclExpCfvo( GetRoot(), *itr, aAddr ) );
        maCfvoList.AppendRecord( xCfvo );
    }
}

void XclExpIconSet::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    rWorksheet->startElement( XML_cfRule,
            XML_type, "iconSet",
            XML_priority, OString::number( mnPriority + 1 ).getStr(),
            FSEND );

    const char* pIconSetName = ScIconSetFormat::getIconSetName(mrFormat.GetIconSetData()->eIconSetType);
    rWorksheet->startElement( XML_iconSet,
            XML_iconSet, pIconSetName,
            XML_showValue, mrFormat.GetIconSetData()->mbShowValue ? nullptr : "0",
            XML_reverse, mrFormat.GetIconSetData()->mbReverse ? "1" : nullptr,
            FSEND );

    maCfvoList.SaveXml( rStrm );

    rWorksheet->endElement( XML_iconSet );
    rWorksheet->endElement( XML_cfRule );
}

XclExpCondFormatBuffer::XclExpCondFormatBuffer( const XclExpRoot& rRoot, const XclExtLstRef& xExtLst ) :
    XclExpRoot( rRoot )
{
    if( const ScConditionalFormatList* pCondFmtList = GetDoc().GetCondFormList(GetCurrScTab()) )
    {
        sal_Int32 nIndex = 0;
        for( const auto& rxCondFmt : *pCondFmtList)
        {
            XclExpCondfmtList::RecordRefType xCondfmtRec( new XclExpCondfmt( GetRoot(), *rxCondFmt, xExtLst, nIndex ));
            if( xCondfmtRec->IsValidForXml() )
                maCondfmtList.AppendRecord( xCondfmtRec );
        }
    }
}

void XclExpCondFormatBuffer::Save( XclExpStream& rStrm )
{
    maCondfmtList.Save( rStrm );
}

void XclExpCondFormatBuffer::SaveXml( XclExpXmlStream& rStrm )
{
    maCondfmtList.SaveXml( rStrm );
}

// Validation =================================================================

namespace {

/** Writes a formula for the DV record. */
void lclWriteDvFormula( XclExpStream& rStrm, const XclTokenArray* pXclTokArr )
{
    sal_uInt16 nFmlaSize = pXclTokArr ? pXclTokArr->GetSize() : 0;
    rStrm << nFmlaSize << sal_uInt16( 0 );
    if( pXclTokArr )
        pXclTokArr->WriteArray( rStrm );
}

/** Writes a formula for the DV record, based on a single string. */
void lclWriteDvFormula( XclExpStream& rStrm, const XclExpString& rString )
{
    // fake a formula with a single tStr token
    rStrm   << static_cast< sal_uInt16 >( rString.GetSize() + 1 )
            << sal_uInt16( 0 )
            << EXC_TOKID_STR
            << rString;
}

const char* lcl_GetValidationType( sal_uInt32 nFlags )
{
    switch( nFlags & EXC_DV_MODE_MASK )
    {
        case EXC_DV_MODE_ANY:       return "none";
        case EXC_DV_MODE_WHOLE:     return "whole";
        case EXC_DV_MODE_DECIMAL:   return "decimal";
        case EXC_DV_MODE_LIST:      return "list";
        case EXC_DV_MODE_DATE:      return "date";
        case EXC_DV_MODE_TIME:      return "time";
        case EXC_DV_MODE_TEXTLEN:   return "textLength";
        case EXC_DV_MODE_CUSTOM:    return "custom";
    }
    return nullptr;
}

const char* lcl_GetOperatorType( sal_uInt32 nFlags )
{
    switch( nFlags & EXC_DV_COND_MASK )
    {
        case EXC_DV_COND_BETWEEN:       return "between";
        case EXC_DV_COND_NOTBETWEEN:    return "notBetween";
        case EXC_DV_COND_EQUAL:         return "equal";
        case EXC_DV_COND_NOTEQUAL:      return "notEqual";
        case EXC_DV_COND_GREATER:       return "greaterThan";
        case EXC_DV_COND_LESS:          return "lessThan";
        case EXC_DV_COND_EQGREATER:     return "greaterThanOrEqual";
        case EXC_DV_COND_EQLESS:        return "lessThanOrEqual";
    }
    return nullptr;
}

} // namespace

XclExpDV::XclExpDV( const XclExpRoot& rRoot, sal_uLong nScHandle ) :
    XclExpRecord( EXC_ID_DV ),
    XclExpRoot( rRoot ),
    mnFlags( 0 ),
    mnScHandle( nScHandle )
{
    if( const ScValidationData* pValData = GetDoc().GetValidationEntry( mnScHandle ) )
    {
        // prompt box - empty string represented by single NUL character
        OUString aTitle, aText;
        bool bShowPrompt = pValData->GetInput( aTitle, aText );
        if( !aTitle.isEmpty() )
            maPromptTitle.Assign( aTitle );
        else
            maPromptTitle.Assign( '\0' );
        if( !aText.isEmpty() )
            maPromptText.Assign( aText );
        else
            maPromptText.Assign( '\0' );

        // error box - empty string represented by single NUL character
        ScValidErrorStyle eScErrorStyle;
        bool bShowError = pValData->GetErrMsg( aTitle, aText, eScErrorStyle );
        if( !aTitle.isEmpty() )
            maErrorTitle.Assign( aTitle );
        else
            maErrorTitle.Assign( '\0' );
        if( !aText.isEmpty() )
            maErrorText.Assign( aText );
        else
            maErrorText.Assign( '\0' );

        // flags
        switch( pValData->GetDataMode() )
        {
            case SC_VALID_ANY:      mnFlags |= EXC_DV_MODE_ANY;         break;
            case SC_VALID_WHOLE:    mnFlags |= EXC_DV_MODE_WHOLE;       break;
            case SC_VALID_DECIMAL:  mnFlags |= EXC_DV_MODE_DECIMAL;     break;
            case SC_VALID_LIST:     mnFlags |= EXC_DV_MODE_LIST;        break;
            case SC_VALID_DATE:     mnFlags |= EXC_DV_MODE_DATE;        break;
            case SC_VALID_TIME:     mnFlags |= EXC_DV_MODE_TIME;        break;
            case SC_VALID_TEXTLEN:  mnFlags |= EXC_DV_MODE_TEXTLEN;     break;
            case SC_VALID_CUSTOM:   mnFlags |= EXC_DV_MODE_CUSTOM;      break;
            default:                OSL_FAIL( "XclExpDV::XclExpDV - unknown mode" );
        }

        switch( pValData->GetOperation() )
        {
            case ScConditionMode::NONE:
            case ScConditionMode::Equal:         mnFlags |= EXC_DV_COND_EQUAL;       break;
            case ScConditionMode::Less:          mnFlags |= EXC_DV_COND_LESS;        break;
            case ScConditionMode::Greater:       mnFlags |= EXC_DV_COND_GREATER;     break;
            case ScConditionMode::EqLess:        mnFlags |= EXC_DV_COND_EQLESS;      break;
            case ScConditionMode::EqGreater:     mnFlags |= EXC_DV_COND_EQGREATER;   break;
            case ScConditionMode::NotEqual:      mnFlags |= EXC_DV_COND_NOTEQUAL;    break;
            case ScConditionMode::Between:       mnFlags |= EXC_DV_COND_BETWEEN;     break;
            case ScConditionMode::NotBetween:    mnFlags |= EXC_DV_COND_NOTBETWEEN;  break;
            default:                    OSL_FAIL( "XclExpDV::XclExpDV - unknown condition" );
        }
        switch( eScErrorStyle )
        {
            case SC_VALERR_STOP:        mnFlags |= EXC_DV_ERROR_STOP;       break;
            case SC_VALERR_WARNING:     mnFlags |= EXC_DV_ERROR_WARNING;    break;
            case SC_VALERR_INFO:        mnFlags |= EXC_DV_ERROR_INFO;       break;
            case SC_VALERR_MACRO:
                // set INFO for validity with macro call, delete title
                mnFlags |= EXC_DV_ERROR_INFO;
                maErrorTitle.Assign( '\0' );    // contains macro name
            break;
            default:                    OSL_FAIL( "XclExpDV::XclExpDV - unknown error style" );
        }
        ::set_flag( mnFlags, EXC_DV_IGNOREBLANK, pValData->IsIgnoreBlank() );
        ::set_flag( mnFlags, EXC_DV_SUPPRESSDROPDOWN, pValData->GetListType() == css::sheet::TableValidationVisibility::INVISIBLE );
        ::set_flag( mnFlags, EXC_DV_SHOWPROMPT, bShowPrompt );
        ::set_flag( mnFlags, EXC_DV_SHOWERROR, bShowError );

        // formulas
        XclExpFormulaCompiler& rFmlaComp = GetFormulaCompiler();
        std::unique_ptr< ScTokenArray > xScTokArr;

        // first formula
        xScTokArr.reset( pValData->CreateFlatCopiedTokenArray( 0 ) );
        if (xScTokArr)
        {
            if( pValData->GetDataMode() == SC_VALID_LIST )
            {
                OUString aString;
                if( XclTokenArrayHelper::GetStringList( aString, *xScTokArr, '\n' ) )
                {
                    OUStringBuffer sFormulaBuf;
                    sFormulaBuf.append( '"' );
                    /*  Formula is a list of string tokens -> build the Excel string.
                        Data validity is BIFF8 only (important for the XclExpString object).
                        Excel uses the NUL character as string list separator. */
                    mxString1.reset( new XclExpString( XclStrFlags::EightBitLength ) );
                    if (!aString.isEmpty())
                    {
                        sal_Int32 nStringIx = 0;
                        for(;;)
                        {
                            const OUString aToken( aString.getToken( 0, '\n', nStringIx ) );
                            mxString1->Append( aToken );
                            sFormulaBuf.append( aToken );
                            if (nStringIx<0)
                                break;
                            mxString1->Append(OUString(u'\0'));
                            sFormulaBuf.append( ',' );
                        }
                    }
                    ::set_flag( mnFlags, EXC_DV_STRINGLIST );

                    // maximum length allowed in Excel is 255 characters, and don't end with an empty token
                    sal_uInt32 nLen = sFormulaBuf.getLength();
                    if( nLen > 256 )  // 255 + beginning quote mark
                    {
                        nLen = 256;
                        if( sFormulaBuf[nLen - 1] == ',' )
                            --nLen;
                        sFormulaBuf.truncate(nLen);
                    }

                    sFormulaBuf.append( '"' );
                    msFormula1 = sFormulaBuf.makeStringAndClear();
                }
                else
                {
                    /*  All other formulas in validation are stored like conditional
                        formatting formulas (with tRefN/tAreaN tokens as value or
                        array class). But NOT the cell references and defined names
                        in list validation - they are stored as reference class
                        tokens... Example:
                        1) Cell must be equal to A1 -> formula is =A1 -> writes tRefNV token
                        2) List is taken from A1    -> formula is =A1 -> writes tRefNR token
                        Formula compiler supports this by offering two different functions
                        CreateDataValFormula() and CreateListValFormula(). */
                    if(GetOutput() == EXC_OUTPUT_BINARY)
                        mxTokArr1 = rFmlaComp.CreateFormula( EXC_FMLATYPE_LISTVAL, *xScTokArr );
                    else
                        msFormula1 = XclXmlUtils::ToOUString( GetCompileFormulaContext(), pValData->GetSrcPos(),
                            xScTokArr.get());
                }
            }
            else
            {
                // no list validation -> convert the formula
                if(GetOutput() == EXC_OUTPUT_BINARY)
                    mxTokArr1 = rFmlaComp.CreateFormula( EXC_FMLATYPE_DATAVAL, *xScTokArr );
                else
                    msFormula1 = XclXmlUtils::ToOUString( GetCompileFormulaContext(), pValData->GetSrcPos(),
                            xScTokArr.get());
            }
        }

        // second formula
        xScTokArr.reset( pValData->CreateFlatCopiedTokenArray( 1 ) );
        if (xScTokArr)
        {
            if(GetOutput() == EXC_OUTPUT_BINARY)
                mxTokArr2 = rFmlaComp.CreateFormula( EXC_FMLATYPE_DATAVAL, *xScTokArr );
            else
                msFormula2 = XclXmlUtils::ToOUString( GetCompileFormulaContext(), pValData->GetSrcPos(),
                        xScTokArr.get());
        }
    }
    else
    {
        OSL_FAIL( "XclExpDV::XclExpDV - missing core data" );
        mnScHandle = ULONG_MAX;
    }
}

XclExpDV::~XclExpDV()
{
}

void XclExpDV::InsertCellRange( const ScRange& rRange )
{
    maScRanges.Join( rRange );
}

bool XclExpDV::Finalize()
{
    GetAddressConverter().ConvertRangeList( maXclRanges, maScRanges, true );
    return (mnScHandle != ULONG_MAX) && !maXclRanges.empty();
}

void XclExpDV::WriteBody( XclExpStream& rStrm )
{
    // flags and strings
    rStrm << mnFlags << maPromptTitle << maErrorTitle << maPromptText << maErrorText;
    // condition formulas
    if( mxString1.get() )
        lclWriteDvFormula( rStrm, *mxString1 );
    else
        lclWriteDvFormula( rStrm, mxTokArr1.get() );
    lclWriteDvFormula( rStrm, mxTokArr2.get() );
    // cell ranges
    rStrm << maXclRanges;
}

void XclExpDV::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_dataValidation,
            XML_allowBlank,         ToPsz( ::get_flag( mnFlags, EXC_DV_IGNOREBLANK ) ),
            XML_error,              XESTRING_TO_PSZ( maErrorText ),
            // OOXTODO: XML_errorStyle,
            XML_errorTitle,         XESTRING_TO_PSZ( maErrorTitle ),
            // OOXTODO: XML_imeMode,
            XML_operator,           lcl_GetOperatorType( mnFlags ),
            XML_prompt,             XESTRING_TO_PSZ( maPromptText ),
            XML_promptTitle,        XESTRING_TO_PSZ( maPromptTitle ),
            // showDropDown should have been showNoDropDown - check oox/xlsx import for details
            XML_showDropDown,       ToPsz( ::get_flag( mnFlags, EXC_DV_SUPPRESSDROPDOWN ) ),
            XML_showErrorMessage,   ToPsz( ::get_flag( mnFlags, EXC_DV_SHOWERROR ) ),
            XML_showInputMessage,   ToPsz( ::get_flag( mnFlags, EXC_DV_SHOWPROMPT ) ),
            XML_sqref,              XclXmlUtils::ToOString( maScRanges ).getStr(),
            XML_type,               lcl_GetValidationType( mnFlags ),
            FSEND );
    if( !msFormula1.isEmpty() )
    {
        rWorksheet->startElement( XML_formula1, FSEND );
        rWorksheet->writeEscaped( msFormula1 );
        rWorksheet->endElement( XML_formula1 );
    }
    if( !msFormula2.isEmpty() )
    {
        rWorksheet->startElement( XML_formula2, FSEND );
        rWorksheet->writeEscaped( msFormula2 );
        rWorksheet->endElement( XML_formula2 );
    }
    rWorksheet->endElement( XML_dataValidation );
}

XclExpDval::XclExpDval( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_DVAL, 18 ),
    XclExpRoot( rRoot )
{
}

XclExpDval::~XclExpDval()
{
}

void XclExpDval::InsertCellRange( const ScRange& rRange, sal_uLong nScHandle )
{
    if( GetBiff() == EXC_BIFF8 )
    {
        XclExpDV& rDVRec = SearchOrCreateDv( nScHandle );
        rDVRec.InsertCellRange( rRange );
    }
}

void XclExpDval::Save( XclExpStream& rStrm )
{
    // check all records
    size_t nPos = maDVList.GetSize();
    while( nPos )
    {
        --nPos;     // backwards to keep nPos valid
        XclExpDVRef xDVRec = maDVList.GetRecord( nPos );
        if( !xDVRec->Finalize() )
            maDVList.RemoveRecord( nPos );
    }

    // write the DVAL and the DV's
    if( !maDVList.IsEmpty() )
    {
        XclExpRecord::Save( rStrm );
        maDVList.Save( rStrm );
    }
}

void XclExpDval::SaveXml( XclExpXmlStream& rStrm )
{
    if( maDVList.IsEmpty() )
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_dataValidations,
            XML_count, OString::number(  maDVList.GetSize() ).getStr(),
            // OOXTODO: XML_disablePrompts,
            // OOXTODO: XML_xWindow,
            // OOXTODO: XML_yWindow,
            FSEND );
    maDVList.SaveXml( rStrm );
    rWorksheet->endElement( XML_dataValidations );
}

XclExpDV& XclExpDval::SearchOrCreateDv( sal_uLong nScHandle )
{
    // test last found record
    if( mxLastFoundDV.get() && (mxLastFoundDV->GetScHandle() == nScHandle) )
        return *mxLastFoundDV;

    // binary search
    size_t nCurrPos = 0;
    if( !maDVList.IsEmpty() )
    {
        size_t nFirstPos = 0;
        size_t nLastPos = maDVList.GetSize() - 1;
        bool bLoop = true;
        sal_uLong nCurrScHandle = ::std::numeric_limits< sal_uLong >::max();
        while( (nFirstPos <= nLastPos) && bLoop )
        {
            nCurrPos = (nFirstPos + nLastPos) / 2;
            mxLastFoundDV = maDVList.GetRecord( nCurrPos );
            nCurrScHandle = mxLastFoundDV->GetScHandle();
            if( nCurrScHandle == nScHandle )
                bLoop = false;
            else if( nCurrScHandle < nScHandle )
                nFirstPos = nCurrPos + 1;
            else if( nCurrPos )
                nLastPos = nCurrPos - 1;
            else    // special case for nLastPos = -1
                bLoop = false;
        }
        if( nCurrScHandle == nScHandle )
            return *mxLastFoundDV;
        else if( nCurrScHandle < nScHandle )
            ++nCurrPos;
    }

    // create new DV record
    mxLastFoundDV.reset( new XclExpDV( *this, nScHandle ) );
    maDVList.InsertRecord( mxLastFoundDV, nCurrPos );
    return *mxLastFoundDV;
}

void XclExpDval::WriteBody( XclExpStream& rStrm )
{
    rStrm.WriteZeroBytes( 10 );
    rStrm << EXC_DVAL_NOOBJ << static_cast< sal_uInt32 >( maDVList.GetSize() );
}

// Web Queries ================================================================

XclExpWebQuery::XclExpWebQuery(
        const OUString& rRangeName,
        const OUString& rUrl,
        const OUString& rSource,
        sal_Int32 nRefrSecs ) :
    maDestRange( rRangeName ),
    maUrl( rUrl ),
    // refresh delay time: seconds -> minutes
    mnRefresh( ulimit_cast< sal_Int16 >( (nRefrSecs + 59) / 60 ) ),
    mbEntireDoc( false )
{
    // comma separated list of HTML table names or indexes
    OUString aNewTables;
    OUString aAppendTable;
    bool bExitLoop = false;
    if (!rSource.isEmpty())
    {
        sal_Int32 nStringIx = 0;
        do
        {
            OUString aToken( rSource.getToken( 0, ';', nStringIx ) );
            mbEntireDoc = ScfTools::IsHTMLDocName( aToken );
            bExitLoop = mbEntireDoc || ScfTools::IsHTMLTablesName( aToken );
            if( !bExitLoop && ScfTools::GetHTMLNameFromName( aToken, aAppendTable ) )
                aNewTables = ScGlobal::addToken( aNewTables, aAppendTable, ',' );
        }
        while (nStringIx>0 && !bExitLoop);
    }

    if( !bExitLoop )    // neither HTML_all nor HTML_tables found
    {
        if( !aNewTables.isEmpty() )
            mxQryTables.reset( new XclExpString( aNewTables ) );
        else
            mbEntireDoc = true;
    }
}

XclExpWebQuery::~XclExpWebQuery()
{
}

void XclExpWebQuery::Save( XclExpStream& rStrm )
{
    OSL_ENSURE( !mbEntireDoc || !mxQryTables.get(), "XclExpWebQuery::Save - illegal mode" );
    sal_uInt16 nFlags;

    // QSI record
    rStrm.StartRecord( EXC_ID_QSI, 10 + maDestRange.GetSize() );
    rStrm   << EXC_QSI_DEFAULTFLAGS
            << sal_uInt16( 0x0010 )
            << sal_uInt16( 0x0012 )
            << sal_uInt32( 0x00000000 )
            << maDestRange;
    rStrm.EndRecord();

    // PARAMQRY record
    nFlags = 0;
    ::insert_value( nFlags, EXC_PQRYTYPE_WEBQUERY, 0, 3 );
    ::set_flag( nFlags, EXC_PQRY_WEBQUERY );
    ::set_flag( nFlags, EXC_PQRY_TABLES, !mbEntireDoc );
    rStrm.StartRecord( EXC_ID_PQRY, 12 );
    rStrm   << nFlags
            << sal_uInt16( 0x0000 )
            << sal_uInt16( 0x0001 );
    rStrm.WriteZeroBytes( 6 );
    rStrm.EndRecord();

    // WQSTRING record
    rStrm.StartRecord( EXC_ID_WQSTRING, maUrl.GetSize() );
    rStrm << maUrl;
    rStrm.EndRecord();

    // unknown record 0x0802
    rStrm.StartRecord( EXC_ID_0802, 16 + maDestRange.GetSize() );
    rStrm   << EXC_ID_0802;             // repeated record id ?!?
    rStrm.WriteZeroBytes( 6 );
    rStrm   << sal_uInt16( 0x0003 )
            << sal_uInt32( 0x00000000 )
            << sal_uInt16( 0x0010 )
            << maDestRange;
    rStrm.EndRecord();

    // WEBQRYSETTINGS record
    nFlags = mxQryTables.get() ? EXC_WQSETT_SPECTABLES : EXC_WQSETT_ALL;
    rStrm.StartRecord( EXC_ID_WQSETT, 28 );
    rStrm   << EXC_ID_WQSETT            // repeated record id ?!?
            << sal_uInt16( 0x0000 )
            << sal_uInt16( 0x0004 )
            << sal_uInt16( 0x0000 )
            << EXC_WQSETT_DEFAULTFLAGS
            << nFlags;
    rStrm.WriteZeroBytes( 10 );
    rStrm   << mnRefresh                // refresh delay in minutes
            << EXC_WQSETT_FORMATFULL
            << sal_uInt16( 0x0000 );
    rStrm.EndRecord();

    // WEBQRYTABLES record
    if( mxQryTables.get() )
    {
        rStrm.StartRecord( EXC_ID_WQTABLES, 4 + mxQryTables->GetSize() );
        rStrm   << EXC_ID_WQTABLES          // repeated record id ?!?
                << sal_uInt16( 0x0000 )
                << *mxQryTables;            // comma separated list of source tables
        rStrm.EndRecord();
    }
}

XclExpWebQueryBuffer::XclExpWebQueryBuffer( const XclExpRoot& rRoot )
{
    SCTAB nScTab = rRoot.GetCurrScTab();
    SfxObjectShell* pShell = rRoot.GetDocShell();
    if( !pShell ) return;
    ScfPropertySet aModelProp( pShell->GetModel() );
    if( !aModelProp.Is() ) return;

    Reference< XAreaLinks > xAreaLinks;
    aModelProp.GetProperty( xAreaLinks, SC_UNO_AREALINKS );
    if( !xAreaLinks.is() ) return;

    for( sal_Int32 nIndex = 0, nCount = xAreaLinks->getCount(); nIndex < nCount; ++nIndex )
    {
        Reference< XAreaLink > xAreaLink( xAreaLinks->getByIndex( nIndex ), UNO_QUERY );
        if( xAreaLink.is() )
        {
            CellRangeAddress aDestRange( xAreaLink->getDestArea() );
            if( static_cast< SCTAB >( aDestRange.Sheet ) == nScTab )
            {
                ScfPropertySet aLinkProp( xAreaLink );
                OUString aFilter;
                if( aLinkProp.GetProperty( aFilter, SC_UNONAME_FILTER ) &&
                    (aFilter == EXC_WEBQRY_FILTER) )
                {
                    // get properties
                    OUString /*aFilterOpt,*/ aUrl;
                    sal_Int32 nRefresh = 0;

//                  aLinkProp.GetProperty( aFilterOpt, SC_UNONAME_FILTOPT );
                    aLinkProp.GetProperty( aUrl, SC_UNONAME_LINKURL );
                    aLinkProp.GetProperty( nRefresh, SC_UNONAME_REFDELAY );

                    OUString aAbsDoc( ScGlobal::GetAbsDocName( aUrl, pShell ) );
                    INetURLObject aUrlObj( aAbsDoc );
                    OUString aWebQueryUrl( aUrlObj.getFSysPath( FSysStyle::Dos ) );
                    if( aWebQueryUrl.isEmpty() )
                        aWebQueryUrl = aAbsDoc;

                    // find range or create a new range
                    OUString aRangeName;
                    ScRange aScDestRange;
                    ScUnoConversion::FillScRange( aScDestRange, aDestRange );
                    if( const ScRangeData* pRangeData = rRoot.GetNamedRanges().findByRange( aScDestRange ) )
                    {
                        aRangeName = pRangeData->GetName();
                    }
                    else
                    {
                        XclExpFormulaCompiler& rFmlaComp = rRoot.GetFormulaCompiler();
                        XclExpNameManager& rNameMgr = rRoot.GetNameManager();

                        // create a new unique defined name containing the range
                        XclTokenArrayRef xTokArr = rFmlaComp.CreateFormula( EXC_FMLATYPE_WQUERY, aScDestRange );
                        sal_uInt16 nNameIdx = rNameMgr.InsertUniqueName( aUrlObj.getBase(), xTokArr, nScTab );
                        aRangeName = rNameMgr.GetOrigName( nNameIdx );
                    }

                    // create and store the web query record
                    if( !aRangeName.isEmpty() )
                        AppendNewRecord( new XclExpWebQuery(
                            aRangeName, aWebQueryUrl, xAreaLink->getSourceArea(), nRefresh ) );
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

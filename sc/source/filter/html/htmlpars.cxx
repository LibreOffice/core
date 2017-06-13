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
#include <sal/config.h>

#include <comphelper/string.hxx>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svtools/htmlcfg.hxx>
#include <svx/algitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/justifyitem.hxx>
#include <sfx2/objsh.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <vcl/graphicfilter.hxx>
#include <svtools/parhtml.hxx>
#include <svtools/htmlkywd.hxx>
#include <svtools/htmltokn.h>
#include <sfx2/docfile.hxx>

#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <tools/tenccvt.hxx>

#include <rtl/tencinfo.h>

#include "htmlpars.hxx"
#include "global.hxx"
#include "document.hxx"
#include "rangelst.hxx"

#include <o3tl/make_unique.hxx>
#include <orcus/css_parser.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <utility>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

ScHTMLStyles::ScHTMLStyles() : maEmpty() {}

void ScHTMLStyles::add(const char* pElemName, size_t nElemName, const char* pClassName, size_t nClassName,
                       const OUString& aProp, const OUString& aValue)
{
    if (pElemName)
    {
        OUString aElem(pElemName, nElemName, RTL_TEXTENCODING_UTF8);
        aElem = aElem.toAsciiLowerCase();
        if (pClassName)
        {
            // Both element and class names given.
            ElemsType::iterator itrElem = m_ElemProps.find(aElem);
            if (itrElem == m_ElemProps.end())
            {
                // new element
                std::pair<ElemsType::iterator, bool> r =
                    m_ElemProps.insert(std::make_pair(aElem, o3tl::make_unique<NamePropsType>()));
                if (!r.second)
                    // insertion failed.
                    return;
                itrElem = r.first;
            }

            NamePropsType *const pClsProps = itrElem->second.get();
            OUString aClass(pClassName, nClassName, RTL_TEXTENCODING_UTF8);
            aClass = aClass.toAsciiLowerCase();
            insertProp(*pClsProps, aClass, aProp, aValue);
        }
        else
        {
            // Element name only. Add it to the element global.
            insertProp(m_ElemGlobalProps, aElem, aProp, aValue);
        }
    }
    else
    {
        if (pClassName)
        {
            // Class name only. Add it to the global.
            OUString aClass(pClassName, nClassName, RTL_TEXTENCODING_UTF8);
            aClass = aClass.toAsciiLowerCase();
            insertProp(m_GlobalProps, aClass, aProp, aValue);
        }
    }
}

const OUString& ScHTMLStyles::getPropertyValue(
    const OUString& rElem, const OUString& rClass, const OUString& rPropName) const
{
    // First, look into the element-class storage.
    {
        auto const itr = m_ElemProps.find(rElem);
        if (itr != m_ElemProps.end())
        {
            const NamePropsType *const pClasses = itr->second.get();
            NamePropsType::const_iterator itr2 = pClasses->find(rClass);
            if (itr2 != pClasses->end())
            {
                const PropsType *const pProps = itr2->second.get();
                PropsType::const_iterator itr3 = pProps->find(rPropName);
                if (itr3 != pProps->end())
                    return itr3->second;
            }
        }
    }
    // Next, look into the class global storage.
    {
        auto const itr = m_GlobalProps.find(rClass);
        if (itr != m_GlobalProps.end())
        {
            const PropsType *const pProps = itr->second.get();
            PropsType::const_iterator itr2 = pProps->find(rPropName);
            if (itr2 != pProps->end())
                return itr2->second;
        }
    }
    // As the last resort, look into the element global storage.
    {
        auto const itr = m_ElemGlobalProps.find(rClass);
        if (itr != m_ElemGlobalProps.end())
        {
            const PropsType *const pProps = itr->second.get();
            PropsType::const_iterator itr2 = pProps->find(rPropName);
            if (itr2 != pProps->end())
                return itr2->second;
        }
    }

    return maEmpty; // nothing found.
}

void ScHTMLStyles::insertProp(
    NamePropsType& rStore, const OUString& aName,
    const OUString& aProp, const OUString& aValue)
{
    NamePropsType::iterator itr = rStore.find(aName);
    if (itr == rStore.end())
    {
        // new element
        std::pair<NamePropsType::iterator, bool> r =
            rStore.insert(std::make_pair(aName, o3tl::make_unique<PropsType>()));
        if (!r.second)
            // insertion failed.
            return;

        itr = r.first;
    }

    PropsType *const pProps = itr->second.get();
    pProps->insert(PropsType::value_type(aProp, aValue));
}

// BASE class for HTML parser classes

ScHTMLParser::ScHTMLParser( EditEngine* pEditEngine, ScDocument* pDoc ) :
    ScEEParser( pEditEngine ),
    mpDoc( pDoc )
{
    SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
    for( sal_uInt16 nIndex = 0; nIndex < SC_HTML_FONTSIZES; ++nIndex )
        maFontHeights[ nIndex ] = rHtmlOptions.GetFontSize( nIndex ) * 20;
}

ScHTMLParser::~ScHTMLParser()
{
}

ScHTMLLayoutParser::ScHTMLLayoutParser(
    EditEngine* pEditP, const OUString& rBaseURL, const Size& aPageSizeP,
    ScDocument* pDocP ) :
        ScHTMLParser( pEditP, pDocP ),
        aPageSize( aPageSizeP ),
        aBaseURL( rBaseURL ),
        xLockedList( new ScRangeList ),
        pTables( nullptr ),
        pLocalColOffset( new ScHTMLColOffset ),
        nFirstTableCell(0),
        nTableLevel(0),
        nTable(0),
        nMaxTable(0),
        nColCntStart(0),
        nMaxCol(0),
        nTableWidth(0),
        nColOffset(0),
        nColOffsetStart(0),
        nOffsetTolerance( SC_HTML_OFFSET_TOLERANCE_SMALL ),
        bTabInTabCell( false ),
        bFirstRow( true ),
        bInCell( false ),
        bInTitle( false )
{
    MakeColNoRef( pLocalColOffset, 0, 0, 0, 0 );
    MakeColNoRef( &maColOffset, 0, 0, 0, 0 );
}

ScHTMLLayoutParser::~ScHTMLLayoutParser()
{
    while ( !aTableStack.empty() )
    {
        ScHTMLTableStackEntry* pS = aTableStack.top();
        aTableStack.pop();

        bool found = false;
        for (ScEEParseEntry* p : maList)
        {
            if ( pS->pCellEntry == p )
            {
                found = true;
                break;
            }
        }
        if ( !found )
            delete pS->pCellEntry;
        if ( pS->pLocalColOffset != pLocalColOffset )
            delete pS->pLocalColOffset;
        delete pS;
    }
    delete pLocalColOffset;
    if ( pTables )
    {
        for( OuterMap::const_iterator it = pTables->begin(); it != pTables->end(); ++it)
            delete it->second;
        delete pTables;
    }
}

ErrCode ScHTMLLayoutParser::Read( SvStream& rStream, const OUString& rBaseURL )
{
    Link<HtmlImportInfo&,void> aOldLink = pEdit->GetHtmlImportHdl();
    pEdit->SetHtmlImportHdl( LINK( this, ScHTMLLayoutParser, HTMLImportHdl ) );

    SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
    bool bLoading = pObjSh && pObjSh->IsLoading();

    SvKeyValueIteratorRef xValues;
    SvKeyValueIterator* pAttributes = nullptr;
    if ( bLoading )
        pAttributes = pObjSh->GetHeaderAttributes();
    else
    {
        // When not loading, set up fake http headers to force the SfxHTMLParser to use UTF8
        // (used when pasting from clipboard)
        const sal_Char* pCharSet = rtl_getBestMimeCharsetFromTextEncoding( RTL_TEXTENCODING_UTF8 );
        if( pCharSet )
        {
            OUString aContentType = "text/html; charset=";
            aContentType += OUString::createFromAscii( pCharSet );

            xValues = new SvKeyValueIterator;
            xValues->Append( SvKeyValue( OOO_STRING_SVTOOLS_HTML_META_content_type, aContentType ) );
            pAttributes = xValues.get();
        }
    }

    ErrCode nErr = pEdit->Read( rStream, rBaseURL, EE_FORMAT_HTML, pAttributes );

    pEdit->SetHtmlImportHdl( aOldLink );
    // Create column width
    Adjust();
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    sal_uInt16 nCount = maColOffset.size();
    sal_uLong nOff = maColOffset[0];
    Size aSize;
    for ( sal_uInt16 j = 1; j < nCount; j++ )
    {
        aSize.Width() = maColOffset[j] - nOff;
        aSize = pDefaultDev->PixelToLogic( aSize, MapMode( MapUnit::MapTwip ) );
        maColWidths[ j-1 ] = aSize.Width();
        nOff = maColOffset[j];
    }
    return nErr;
}

const ScHTMLTable* ScHTMLLayoutParser::GetGlobalTable() const
{
    return nullptr;
}

void ScHTMLLayoutParser::NewActEntry( ScEEParseEntry* pE )
{
    ScEEParser::NewActEntry( pE );
    if ( pE )
    {
        if ( !pE->aSel.HasRange() )
        {   // Completely empty, following text ends up in the same paragraph!
            pActEntry->aSel.nStartPara = pE->aSel.nEndPara;
            pActEntry->aSel.nStartPos = pE->aSel.nEndPos;
        }
    }
    pActEntry->aSel.nEndPara = pActEntry->aSel.nStartPara;
    pActEntry->aSel.nEndPos = pActEntry->aSel.nStartPos;
}

void ScHTMLLayoutParser::EntryEnd( ScEEParseEntry* pE, const ESelection& rSel )
{
    if ( rSel.nEndPara >= pE->aSel.nStartPara )
    {
        pE->aSel.nEndPara = rSel.nEndPara;
        pE->aSel.nEndPos = rSel.nEndPos;
    }
    else if ( rSel.nStartPara == pE->aSel.nStartPara - 1 && !pE->aSel.HasRange() )
    {   // Did not attach a paragraph, but empty, do nothing
    }
    else
    {
        OSL_FAIL( "EntryEnd: EditEngine ESelection End < Start" );
    }
}

void ScHTMLLayoutParser::NextRow( HtmlImportInfo* pInfo )
{
    if ( bInCell )
        CloseEntry( pInfo );
    if ( nRowMax < ++nRowCnt )
        nRowMax = nRowCnt;
    nColCnt = nColCntStart;
    nColOffset = nColOffsetStart;
    bFirstRow = false;
}

bool ScHTMLLayoutParser::SeekOffset( ScHTMLColOffset* pOffset, sal_uInt16 nOffset,
        SCCOL* pCol, sal_uInt16 nOffsetTol )
{
    OSL_ENSURE( pOffset, "ScHTMLLayoutParser::SeekOffset - illegal call" );
    ScHTMLColOffset::const_iterator it = pOffset->find( nOffset );
    bool bFound = it != pOffset->end();
    sal_uInt16 nPos = it - pOffset->begin();
    *pCol = static_cast<SCCOL>(nPos);
    if ( bFound )
        return true;
    sal_uInt16 nCount = pOffset->size();
    if ( !nCount )
        return false;
    // nPos is the position of insertion, that's where the next higher one is (or isn't)
    if ( nPos < nCount && (((*pOffset)[nPos] - nOffsetTol) <= nOffset) )
        return true;
    // Not smaller than everything else? Then compare with the next lower one
    else if ( nPos && (((*pOffset)[nPos-1] + nOffsetTol) >= nOffset) )
    {
        (*pCol)--;
        return true;
    }
    return false;
}

void ScHTMLLayoutParser::MakeCol( ScHTMLColOffset* pOffset, sal_uInt16& nOffset,
        sal_uInt16& nWidth, sal_uInt16 nOffsetTol, sal_uInt16 nWidthTol )
{
    OSL_ENSURE( pOffset, "ScHTMLLayoutParser::MakeCol - illegal call" );
    SCCOL nPos;
    if ( SeekOffset( pOffset, nOffset, &nPos, nOffsetTol ) )
        nOffset = (sal_uInt16)(*pOffset)[nPos];
    else
        pOffset->insert( nOffset );
    if ( nWidth )
    {
        if ( SeekOffset( pOffset, nOffset + nWidth, &nPos, nWidthTol ) )
            nWidth = (sal_uInt16)(*pOffset)[nPos] - nOffset;
        else
            pOffset->insert( nOffset + nWidth );
    }
}

void ScHTMLLayoutParser::MakeColNoRef( ScHTMLColOffset* pOffset, sal_uInt16 nOffset,
        sal_uInt16 nWidth, sal_uInt16 nOffsetTol, sal_uInt16 nWidthTol )
{
    OSL_ENSURE( pOffset, "ScHTMLLayoutParser::MakeColNoRef - illegal call" );
    SCCOL nPos;
    if ( SeekOffset( pOffset, nOffset, &nPos, nOffsetTol ) )
        nOffset = (sal_uInt16)(*pOffset)[nPos];
    else
        pOffset->insert( nOffset );
    if ( nWidth )
    {
        if ( !SeekOffset( pOffset, nOffset + nWidth, &nPos, nWidthTol ) )
            pOffset->insert( nOffset + nWidth );
    }
}

void ScHTMLLayoutParser::ModifyOffset( ScHTMLColOffset* pOffset, sal_uInt16& nOldOffset,
            sal_uInt16& nNewOffset, sal_uInt16 nOffsetTol )
{
    OSL_ENSURE( pOffset, "ScHTMLLayoutParser::ModifyOffset - illegal call" );
    SCCOL nPos;
    if ( !SeekOffset( pOffset, nOldOffset, &nPos, nOffsetTol ) )
    {
        if ( SeekOffset( pOffset, nNewOffset, &nPos, nOffsetTol ) )
            nNewOffset = (sal_uInt16)(*pOffset)[nPos];
        else
            pOffset->insert( nNewOffset );
        return ;
    }
    nOldOffset = (sal_uInt16)(*pOffset)[nPos];
    SCCOL nPos2;
    if ( SeekOffset( pOffset, nNewOffset, &nPos2, nOffsetTol ) )
    {
        nNewOffset = (sal_uInt16)(*pOffset)[nPos2];
        return ;
    }
    long nDiff = nNewOffset - nOldOffset;
    if ( nDiff < 0 )
    {
        do
        {
            const_cast<sal_uLong&>((*pOffset)[nPos]) += nDiff;
        } while ( nPos-- );
    }
    else
    {
        do
        {
            const_cast<sal_uLong&>((*pOffset)[nPos]) += nDiff;
        } while ( ++nPos < (sal_uInt16)pOffset->size() );
    }
}

void ScHTMLLayoutParser::SkipLocked( ScEEParseEntry* pE, bool bJoin )
{
    if ( ValidCol(pE->nCol) )
    {   // Or else this would create a wrong value at ScAddress (chance for an infinite loop)!
        bool bBadCol = false;
        bool bAgain;
        ScRange aRange( pE->nCol, pE->nRow, 0,
            pE->nCol + pE->nColOverlap - 1, pE->nRow + pE->nRowOverlap - 1, 0 );
        do
        {
            bAgain = false;
            for ( size_t i =  0, nRanges = xLockedList->size(); i < nRanges; ++i )
            {
                ScRange* pR = (*xLockedList)[i];
                if ( pR->Intersects( aRange ) )
                {
                    pE->nCol = pR->aEnd.Col() + 1;
                    SCCOL nTmp = pE->nCol + pE->nColOverlap - 1;
                    if ( pE->nCol > MAXCOL || nTmp > MAXCOL )
                        bBadCol = true;
                    else
                    {
                        bAgain = true;
                        aRange.aStart.SetCol( pE->nCol );
                        aRange.aEnd.SetCol( nTmp );
                    }
                    break;
                }
            }
        } while ( bAgain );
        if ( bJoin && !bBadCol )
            xLockedList->Join( aRange );
    }
}

void ScHTMLLayoutParser::Adjust()
{
    xLockedList->RemoveAll();

    std::stack< ScHTMLAdjustStackEntry* > aStack;
    ScHTMLAdjustStackEntry* pS = nullptr;
    sal_uInt16 nTab = 0;
    SCCOL nLastCol = SCCOL_MAX;
    SCROW nNextRow = 0;
    SCROW nCurRow = 0;
    sal_uInt16 nPageWidth = (sal_uInt16) aPageSize.Width();
    InnerMap* pTab = nullptr;
    for (ScEEParseEntry* pE : maList)
    {
        if ( pE->nTab < nTab )
        {   // Table finished
            if ( !aStack.empty() )
            {
                pS = aStack.top();
                aStack.pop();

                nLastCol = pS->nLastCol;
                nNextRow = pS->nNextRow;
                nCurRow = pS->nCurRow;
            }
            delete pS;
            pS = nullptr;
            nTab = pE->nTab;
            if (pTables)
            {
                OuterMap::const_iterator it = pTables->find( nTab );
                if ( it != pTables->end() )
                    pTab = it->second;
            }

        }
        SCROW nRow = pE->nRow;
        if ( pE->nCol <= nLastCol )
        {   // Next row
            if ( pE->nRow < nNextRow )
                pE->nRow = nCurRow = nNextRow;
            else
                nCurRow = nNextRow = pE->nRow;
            SCROW nR = 0;
            if ( pTab )
            {
                InnerMap::const_iterator it = pTab->find( nCurRow );
                if ( it != pTab->end() )
                    nR = it->second;
            }
            if ( nR )
                nNextRow += nR;
            else
                nNextRow++;
        }
        else
            pE->nRow = nCurRow;
        nLastCol = pE->nCol; // Read column
        if ( pE->nTab > nTab )
        {   // New table
            aStack.push( new ScHTMLAdjustStackEntry(
                nLastCol, nNextRow, nCurRow ) );
            nTab = pE->nTab;
            if ( pTables )
            {
                OuterMap::const_iterator it = pTables->find( nTab );
                if ( it != pTables->end() )
                    pTab = it->second;
            }
            // New line spacing
            SCROW nR = 0;
            if ( pTab )
            {
                InnerMap::const_iterator it = pTab->find( nCurRow );
                if ( it != pTab->end() )
                    nR = it->second;
            }
            if ( nR )
                nNextRow = nCurRow + nR;
            else
                nNextRow = nCurRow + 1;
        }
        if ( nTab == 0 )
            pE->nWidth = nPageWidth;
        else
        {   // Real table, no paragraphs on the field
            if ( pTab )
            {
                SCROW nRowSpan = pE->nRowOverlap;
                for ( SCROW j=0; j < nRowSpan; j++ )
                {   // RowSpan resulting from merged rows
                    SCROW nRows = 0;
                    InnerMap::const_iterator it = pTab->find( nRow+j );
                    if ( it != pTab->end() )
                        nRows = it->second;
                    if ( nRows > 1 )
                    {
                        pE->nRowOverlap += nRows - 1;
                        if ( j == 0 )
                        {   // Merged rows move the next row
                            SCROW nTmp = nCurRow + nRows;
                            if ( nNextRow < nTmp )
                                nNextRow = nTmp;
                        }
                    }
                }
            }
        }
        // Real column
        (void)SeekOffset( &maColOffset, pE->nOffset, &pE->nCol, nOffsetTolerance );
        SCCOL nColBeforeSkip = pE->nCol;
        SkipLocked( pE, false );
        if ( pE->nCol != nColBeforeSkip )
        {
            SCCOL nCount = (SCCOL)maColOffset.size();
            if ( nCount <= pE->nCol )
            {
                pE->nOffset = (sal_uInt16) maColOffset[nCount-1];
                MakeCol( &maColOffset, pE->nOffset, pE->nWidth, nOffsetTolerance, nOffsetTolerance );
            }
            else
            {
                pE->nOffset = (sal_uInt16) maColOffset[pE->nCol];
            }
        }
        SCCOL nPos;
        if ( pE->nWidth && SeekOffset( &maColOffset, pE->nOffset + pE->nWidth, &nPos, nOffsetTolerance ) )
            pE->nColOverlap = (nPos > pE->nCol ? nPos - pE->nCol : 1);
        else
        {
        //FIXME: This may not be correct, but works anyway ...
            pE->nColOverlap = 1;
        }
        xLockedList->Join( ScRange( pE->nCol, pE->nRow, 0,
            pE->nCol + pE->nColOverlap - 1, pE->nRow + pE->nRowOverlap - 1, 0 ) );
        // Take over MaxDimensions
        SCCOL nColTmp = pE->nCol + pE->nColOverlap;
        if ( nColMax < nColTmp )
            nColMax = nColTmp;
        SCROW nRowTmp = pE->nRow + pE->nRowOverlap;
        if ( nRowMax < nRowTmp )
            nRowMax = nRowTmp;
    }
    while ( !aStack.empty() )
    {
        delete aStack.top();
        aStack.pop();
    }
}

sal_uInt16 ScHTMLLayoutParser::GetWidth( ScEEParseEntry* pE )
{
    if ( pE->nWidth )
        return pE->nWidth;
    sal_Int32 nTmp = std::min( static_cast<sal_Int32>( pE->nCol -
                nColCntStart + pE->nColOverlap),
            static_cast<sal_Int32>( pLocalColOffset->size() - 1));
    SCCOL nPos = (nTmp < 0 ? 0 : static_cast<SCCOL>(nTmp));
    sal_uInt16 nOff2 = (sal_uInt16) (*pLocalColOffset)[nPos];
    if ( pE->nOffset < nOff2 )
        return nOff2 - pE->nOffset;
    return 0;
}

void ScHTMLLayoutParser::SetWidths()
{
    ScEEParseEntry* pE;
    SCCOL nCol;
    if ( !nTableWidth )
        nTableWidth = (sal_uInt16) aPageSize.Width();
    SCCOL nColsPerRow = nMaxCol - nColCntStart;
    if ( nColsPerRow <= 0 )
        nColsPerRow = 1;
    if ( pLocalColOffset->size() <= 2 )
    {   // Only PageSize, there was no width setting
        sal_uInt16 nWidth = nTableWidth / static_cast<sal_uInt16>(nColsPerRow);
        sal_uInt16 nOff = nColOffsetStart;
        pLocalColOffset->clear();
        for ( nCol = 0; nCol <= nColsPerRow; ++nCol, nOff = nOff + nWidth )
        {
            MakeColNoRef( pLocalColOffset, nOff, 0, 0, 0 );
        }
        nTableWidth = (sal_uInt16)(pLocalColOffset->back() - pLocalColOffset->front());
        for ( size_t i = nFirstTableCell, nListSize = maList.size(); i < nListSize; ++i )
        {
            pE = maList[ i ];
            if ( pE->nTab == nTable )
            {
                pE->nOffset = (sal_uInt16) (*pLocalColOffset)[pE->nCol - nColCntStart];
                pE->nWidth = 0; // to be recalculated later
            }
        }
    }
    else
    {   // Some without width
        // Why actually no pE?
        if ( nFirstTableCell < maList.size() )
        {
            std::unique_ptr<sal_uInt16[]> pOffsets(new sal_uInt16[ nColsPerRow+1 ]);
            memset( pOffsets.get(), 0, (nColsPerRow+1) * sizeof(sal_uInt16) );
            std::unique_ptr<sal_uInt16[]> pWidths(new sal_uInt16[ nColsPerRow ]);
            memset( pWidths.get(), 0, nColsPerRow * sizeof(sal_uInt16) );
            pOffsets[0] = nColOffsetStart;
            for ( size_t i = nFirstTableCell, nListSize = maList.size(); i < nListSize; ++i )
            {
                pE = maList[ i ];
                if ( pE->nTab == nTable && pE->nWidth )
                {
                    nCol = pE->nCol - nColCntStart;
                    if ( nCol < nColsPerRow )
                    {
                        if ( pE->nColOverlap == 1 )
                        {
                            if ( pWidths[nCol] < pE->nWidth )
                                pWidths[nCol] = pE->nWidth;
                        }
                        else
                        {   // try to find a single undefined width
                            sal_uInt16 nTotal = 0;
                            bool bFound = false;
                            SCCOL nHere = 0;
                            SCCOL nStop = std::min( static_cast<SCCOL>(nCol + pE->nColOverlap), nColsPerRow );
                            for ( ; nCol < nStop; nCol++ )
                            {
                                if ( pWidths[nCol] )
                                    nTotal = nTotal + pWidths[nCol];
                                else
                                {
                                    if ( bFound )
                                    {
                                        bFound = false;
                                        break;  // for
                                    }
                                    bFound = true;
                                    nHere = nCol;
                                }
                            }
                            if ( bFound && pE->nWidth > nTotal )
                                pWidths[nHere] = pE->nWidth - nTotal;
                        }
                    }
                }
            }
            sal_uInt16 nWidths = 0;
            sal_uInt16 nUnknown = 0;
            for ( nCol = 0; nCol < nColsPerRow; nCol++ )
            {
                if ( pWidths[nCol] )
                    nWidths = nWidths + pWidths[nCol];
                else
                    nUnknown++;
            }
            if ( nUnknown )
            {
                sal_uInt16 nW = ((nWidths < nTableWidth) ?
                    ((nTableWidth - nWidths) / nUnknown) :
                    (nTableWidth / nUnknown));
                for ( nCol = 0; nCol < nColsPerRow; nCol++ )
                {
                    if ( !pWidths[nCol] )
                        pWidths[nCol] = nW;
                }
            }
            for ( nCol = 1; nCol <= nColsPerRow; nCol++ )
            {
                pOffsets[nCol] = pOffsets[nCol-1] + pWidths[nCol-1];
            }
            pLocalColOffset->clear();
            for ( nCol = 0; nCol <= nColsPerRow; nCol++ )
            {
                MakeColNoRef( pLocalColOffset, pOffsets[nCol], 0, 0, 0 );
            }
            nTableWidth = pOffsets[nColsPerRow] - pOffsets[0];

            for ( size_t i = nFirstTableCell, nListSize = maList.size(); i < nListSize; ++i )
            {
                pE = maList[ i ];
                if ( pE->nTab == nTable )
                {
                    nCol = pE->nCol - nColCntStart;
                    OSL_ENSURE( nCol < nColsPerRow, "ScHTMLLayoutParser::SetWidths: column overflow" );
                    if ( nCol < nColsPerRow )
                    {
                        pE->nOffset = pOffsets[nCol];
                        nCol = nCol + pE->nColOverlap;
                        if ( nCol > nColsPerRow )
                            nCol = nColsPerRow;
                        pE->nWidth = pOffsets[nCol] - pE->nOffset;
                    }
                }
            }
        }
    }
    if ( !pLocalColOffset->empty() )
    {
        sal_uInt16 nMax = (sal_uInt16) pLocalColOffset->back();
        if ( aPageSize.Width() < nMax )
            aPageSize.Width() = nMax;
    }
    for ( size_t i = nFirstTableCell, nListSize = maList.size(); i < nListSize; ++i )
    {
        pE = maList[ i ];
        if ( pE->nTab == nTable )
        {
            if ( !pE->nWidth )
            {
                pE->nWidth = GetWidth( pE );
                OSL_ENSURE( pE->nWidth, "SetWidths: pE->nWidth == 0" );
            }
            MakeCol( &maColOffset, pE->nOffset, pE->nWidth, nOffsetTolerance, nOffsetTolerance );
        }
    }
}

void ScHTMLLayoutParser::Colonize( ScEEParseEntry* pE )
{
    if ( pE->nCol == SCCOL_MAX )
        pE->nCol = nColCnt;
    if ( pE->nRow == SCROW_MAX )
        pE->nRow = nRowCnt;
    SCCOL nCol = pE->nCol;
    SkipLocked( pE ); // Change of columns to the right

    if ( nCol < pE->nCol )
    {   // Replaced
        nCol = pE->nCol - nColCntStart;
        SCCOL nCount = static_cast<SCCOL>(pLocalColOffset->size());
        if ( nCol < nCount )
            nColOffset = (sal_uInt16) (*pLocalColOffset)[nCol];
        else
            nColOffset = (sal_uInt16) (*pLocalColOffset)[nCount - 1];
    }
    pE->nOffset = nColOffset;
    sal_uInt16 nWidth = GetWidth( pE );
    MakeCol( pLocalColOffset, pE->nOffset, nWidth, nOffsetTolerance, nOffsetTolerance );
    if ( pE->nWidth )
        pE->nWidth = nWidth;
    nColOffset = pE->nOffset + nWidth;
    if ( nTableWidth < nColOffset - nColOffsetStart )
        nTableWidth = nColOffset - nColOffsetStart;
}

void ScHTMLLayoutParser::CloseEntry( HtmlImportInfo* pInfo )
{
    bInCell = false;
    if ( bTabInTabCell )
    {   // From the stack in TableOff
        bTabInTabCell = false;
        bool found = false;
        for (ScEEParseEntry* p : maList)
        {
            if ( pActEntry == p )
            {
                found = true;
                break;
            }
        }
        if ( !found )
            delete pActEntry;
        NewActEntry( maList.back() ); // New free flying pActEntry
        return ;
    }
    if ( pActEntry->nTab == 0 )
        pActEntry->nWidth = (sal_uInt16) aPageSize.Width();
    Colonize( pActEntry );
    nColCnt = pActEntry->nCol + pActEntry->nColOverlap;
    if ( nMaxCol < nColCnt )
        nMaxCol = nColCnt;      // TableStack MaxCol
    if ( nColMax < nColCnt )
        nColMax = nColCnt;      // Global MaxCol for ScEEParser GetDimensions!
    EntryEnd( pActEntry, pInfo->aSelection );
    ESelection& rSel = pActEntry->aSel;
    while ( rSel.nStartPara < rSel.nEndPara
            && pEdit->GetTextLen( rSel.nStartPara ) == 0 )
    {   // Strip preceding empty paragraphs
        rSel.nStartPara++;
    }
    while ( rSel.nEndPos == 0 && rSel.nEndPara > rSel.nStartPara )
    {   // Strip successive empty paragraphs
        rSel.nEndPara--;
        rSel.nEndPos = pEdit->GetTextLen( rSel.nEndPara );
    }
    if ( rSel.nStartPara > rSel.nEndPara )
    {   // Gives GPF in CreateTextObject
        OSL_FAIL( "CloseEntry: EditEngine ESelection Start > End" );
        rSel.nEndPara = rSel.nStartPara;
    }
    if ( rSel.HasRange() )
        pActEntry->aItemSet.Put( SfxBoolItem( ATTR_LINEBREAK, true ) );
    maList.push_back( pActEntry );
    NewActEntry( pActEntry ); // New free flying pActEntry
}

IMPL_LINK( ScHTMLLayoutParser, HTMLImportHdl, HtmlImportInfo&, rInfo, void )
{
    switch ( rInfo.eState )
    {
        case HtmlImportState::NextToken:
            ProcToken( &rInfo );
            break;
        case HtmlImportState::Start:
            break;
        case HtmlImportState::End:
            if ( rInfo.aSelection.nEndPos )
            {
                // If text remains: create paragraph, without calling CloseEntry().
                if( bInCell )   // ...but only in opened table cells.
                {
                    bInCell = false;
                    NextRow( &rInfo );
                    bInCell = true;
                }
                CloseEntry( &rInfo );
            }
            while ( nTableLevel > 0 )
                TableOff( &rInfo );      // close tables, if </TABLE> missing
            break;
        case HtmlImportState::SetAttr:
            break;
        case HtmlImportState::InsertText:
            break;
        case HtmlImportState::InsertPara:
            if ( nTableLevel < 1 )
            {
                CloseEntry( &rInfo );
                NextRow( &rInfo );
            }
            break;
        case HtmlImportState::InsertField:
            break;
        default:
            OSL_FAIL("HTMLImportHdl: unknown ImportInfo.eState");
    }
}

// Greatest common divisor (Euclid)
// Special case: 0 and something gives 1
static SCROW lcl_GGT( SCROW a, SCROW b )
{
    if ( !a || !b )
        return 1;
    do
    {
        if ( a > b )
            a -= SCROW(a / b) * b;
        else
            b -= SCROW(b / a) * a;
    } while ( a && b );
    return ((a != 0) ? a : b);
}

// Lowest common multiple: a * b / GCD(a,b)
static SCROW lcl_KGV( SCROW a, SCROW b )
{
    if ( a > b )    // Make overflow even less likely
        return (a / lcl_GGT(a,b)) * b;
    else
        return (b / lcl_GGT(a,b)) * a;
}

void ScHTMLLayoutParser::TableDataOn( HtmlImportInfo* pInfo )
{
    if ( bInCell )
        CloseEntry( pInfo );
    if ( !nTableLevel )
    {
        OSL_FAIL( "dumbo doc! <TH> or <TD> without previous <TABLE>" );
        TableOn( pInfo );
    }
    bInCell = true;
    bool bHorJustifyCenterTH = (pInfo->nToken == HtmlTokenId::TABLEHEADER_ON);
    const HTMLOptions& rOptions = static_cast<HTMLParser*>(pInfo->pParser)->GetOptions();
    for (const auto & rOption : rOptions)
    {
        switch( rOption.GetToken() )
        {
            case HtmlOptionId::COLSPAN:
            {
                pActEntry->nColOverlap = ( SCCOL ) rOption.GetString().toInt32();
            }
            break;
            case HtmlOptionId::ROWSPAN:
            {
                pActEntry->nRowOverlap = ( SCROW ) rOption.GetString().toInt32();
            }
            break;
            case HtmlOptionId::ALIGN:
            {
                bHorJustifyCenterTH = false;
                SvxCellHorJustify eVal;
                const OUString& rOptVal = rOption.GetString();
                if ( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_right )  )
                    eVal = SvxCellHorJustify::Right;
                else if ( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_center ) )
                    eVal = SvxCellHorJustify::Center;
                else if ( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_left ) )
                    eVal = SvxCellHorJustify::Left;
                else
                    eVal = SvxCellHorJustify::Standard;
                if ( eVal != SvxCellHorJustify::Standard )
                    pActEntry->aItemSet.Put( SvxHorJustifyItem( eVal, ATTR_HOR_JUSTIFY) );
            }
            break;
            case HtmlOptionId::VALIGN:
            {
                SvxCellVerJustify eVal;
                const OUString& rOptVal = rOption.GetString();
                if ( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_VA_top ) )
                    eVal = SVX_VER_JUSTIFY_TOP;
                else if ( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_VA_middle ) )
                    eVal = SVX_VER_JUSTIFY_CENTER;
                else if ( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_VA_bottom ) )
                    eVal = SVX_VER_JUSTIFY_BOTTOM;
                else
                    eVal = SVX_VER_JUSTIFY_STANDARD;
                pActEntry->aItemSet.Put( SvxVerJustifyItem( eVal, ATTR_VER_JUSTIFY) );
            }
            break;
            case HtmlOptionId::WIDTH:
            {
                pActEntry->nWidth = GetWidthPixel( rOption );
            }
            break;
            case HtmlOptionId::BGCOLOR:
            {
                Color aColor;
                rOption.GetColor( aColor );
                pActEntry->aItemSet.Put(
                    SvxBrushItem( aColor, ATTR_BACKGROUND ) );
            }
            break;
            case HtmlOptionId::SDVAL:
            {
                pActEntry->pValStr = new OUString( rOption.GetString() );
            }
            break;
            case HtmlOptionId::SDNUM:
            {
                pActEntry->pNumStr = new OUString( rOption.GetString() );
            }
            break;
            default: break;
        }
    }
    pActEntry->nCol = nColCnt;
    pActEntry->nRow = nRowCnt;
    pActEntry->nTab = nTable;

    if ( bHorJustifyCenterTH )
        pActEntry->aItemSet.Put(
            SvxHorJustifyItem( SvxCellHorJustify::Center, ATTR_HOR_JUSTIFY) );
}

void ScHTMLLayoutParser::TableRowOn( HtmlImportInfo* pInfo )
{
    if ( nColCnt > nColCntStart )
        NextRow( pInfo ); // The optional TableRowOff wasn't there
    nColOffset = nColOffsetStart;
}

void ScHTMLLayoutParser::TableRowOff( HtmlImportInfo* pInfo )
{
    NextRow( pInfo );
}

void ScHTMLLayoutParser::TableDataOff( HtmlImportInfo* pInfo )
{
    if ( bInCell )
        CloseEntry( pInfo ); // Only if it really was one
}

void ScHTMLLayoutParser::TableOn( HtmlImportInfo* pInfo )
{
    OUString aTabName;

    if ( ++nTableLevel > 1 )
    {   // Table in Table
        sal_uInt16 nTmpColOffset = nColOffset; // Will be changed in Colonize()
        Colonize( pActEntry );
        aTableStack.push( new ScHTMLTableStackEntry(
            pActEntry, xLockedList, pLocalColOffset, nFirstTableCell,
            nRowCnt, nColCntStart, nMaxCol, nTable,
            nTableWidth, nColOffset, nColOffsetStart,
            bFirstRow ) );
        sal_uInt16 nLastWidth = nTableWidth;
        nTableWidth = GetWidth( pActEntry );
        if ( nTableWidth == nLastWidth && nMaxCol - nColCntStart > 1 )
        {   // There must be more than one, so this one cannot be enough
            nTableWidth = nLastWidth / static_cast<sal_uInt16>((nMaxCol - nColCntStart));
        }
        nLastWidth = nTableWidth;
        if ( pInfo->nToken == HtmlTokenId::TABLE_ON )
        {   // It can still be TD or TH, if we didn't have a TABLE earlier
            const HTMLOptions& rOptions = static_cast<HTMLParser*>(pInfo->pParser)->GetOptions();
            for (const auto & rOption : rOptions)
            {
                switch( rOption.GetToken() )
                {
                    case HtmlOptionId::WIDTH:
                    {   // Percent: of document width or outer cell
                        nTableWidth = GetWidthPixel( rOption );
                    }
                    break;
                    case HtmlOptionId::BORDER:
                        // Border is: ((pOption->GetString().Len() == 0) || (pOption->GetNumber() != 0));
                    break;
                    case HtmlOptionId::ID:
                        aTabName = rOption.GetString();
                    break;
                    default: break;
                }
            }
        }
        bInCell = false;
        if ( bTabInTabCell && !(nTableWidth < nLastWidth) )
        {   // Multiple tables in one cell, underneath each other
            bTabInTabCell = false;
            NextRow( pInfo );
        }
        else
        {   // It start's in this cell or next to each other
            bTabInTabCell = false;
            nColCntStart = nColCnt;
            nColOffset = nTmpColOffset;
            nColOffsetStart = nColOffset;
        }

        ScEEParseEntry* pE = nullptr;
        if (maList.size())
            pE = maList.back();
        NewActEntry( pE ); // New free flying pActEntry
        xLockedList = new ScRangeList;
    }
    else
    {   // Simple table at the document level
        EntryEnd( pActEntry, pInfo->aSelection );
        if ( pActEntry->aSel.HasRange() )
        {   // Flying text left
            CloseEntry( pInfo );
            NextRow( pInfo );
        }
        aTableStack.push( new ScHTMLTableStackEntry(
            pActEntry, xLockedList, pLocalColOffset, nFirstTableCell,
            nRowCnt, nColCntStart, nMaxCol, nTable,
            nTableWidth, nColOffset, nColOffsetStart,
            bFirstRow ) );
        // As soon as we have multiple tables we need to be tolerant with the offsets.
        if (nMaxTable > 0)
            nOffsetTolerance = SC_HTML_OFFSET_TOLERANCE_LARGE;
        nTableWidth = 0;
        if ( pInfo->nToken == HtmlTokenId::TABLE_ON )
        {
            // It can still be TD or TH, if we didn't have a TABLE earlier
            const HTMLOptions& rOptions = static_cast<HTMLParser*>(pInfo->pParser)->GetOptions();
            for (const auto & rOption : rOptions)
            {
                switch( rOption.GetToken() )
                {
                    case HtmlOptionId::WIDTH:
                    {   // Percent: of document width or outer cell
                        nTableWidth = GetWidthPixel( rOption );
                    }
                    break;
                    case HtmlOptionId::BORDER:
                        //BorderOn is: ((pOption->GetString().Len() == 0) || (pOption->GetNumber() != 0));
                    break;
                    case HtmlOptionId::ID:
                        aTabName = rOption.GetString();
                    break;
                    default: break;
                }
            }
        }
    }
    nTable = ++nMaxTable;
    bFirstRow = true;
    nFirstTableCell = maList.size();

    pLocalColOffset = new ScHTMLColOffset;
    MakeColNoRef( pLocalColOffset, nColOffsetStart, 0, 0, 0 );
}

void ScHTMLLayoutParser::TableOff( HtmlImportInfo* pInfo )
{
    if ( bInCell )
        CloseEntry( pInfo );
    if ( nColCnt > nColCntStart )
        TableRowOff( pInfo ); // The optional TableRowOff wasn't
    if ( !nTableLevel )
    {
        OSL_FAIL( "dumbo doc! </TABLE> without opening <TABLE>" );
        return ;
    }
    if ( --nTableLevel > 0 )
    {   // Table in Table done
        if ( !aTableStack.empty() )
        {
            ScHTMLTableStackEntry* pS = aTableStack.top();
            aTableStack.pop();

            ScEEParseEntry* pE = pS->pCellEntry;
            SCROW nRows = nRowCnt - pS->nRowCnt;
            if ( nRows > 1 )
            {   // Insert size of table at this position
                SCROW nRow = pS->nRowCnt;
                sal_uInt16 nTab = pS->nTable;
                if ( !pTables )
                    pTables = new OuterMap;
                // Height of outer table
                OuterMap::const_iterator it = pTables->find( nTab );
                InnerMap* pTab1;
                if ( it == pTables->end() )
                {
                    pTab1 = new InnerMap;
                    (*pTables)[ nTab ] = pTab1;
                }
                else
                    pTab1 = it->second;
                SCROW nRowSpan = pE->nRowOverlap;
                SCROW nRowKGV;
                SCROW nRowsPerRow1; // Outer table
                SCROW nRowsPerRow2; // Inner table
                if ( nRowSpan > 1 )
                {   // LCM to which we can map the inner and outer rows
                    nRowKGV = lcl_KGV( nRowSpan, nRows );
                    nRowsPerRow1 = nRowKGV / nRowSpan;
                    nRowsPerRow2 = nRowKGV / nRows;
                }
                else
                {
                    nRowKGV = nRowsPerRow1 = nRows;
                    nRowsPerRow2 = 1;
                }
                InnerMap* pTab2 = nullptr;
                if ( nRowsPerRow2 > 1 )
                {   // Height of the inner table
                    pTab2 = new InnerMap;
                    (*pTables)[ nTable ] = pTab2;
                }
                // Abuse void* Data entry of the Table class for height mapping
                if ( nRowKGV > 1 )
                {
                    if ( nRowsPerRow1 > 1 )
                    {   // Outer
                        for ( SCROW j=0; j < nRowSpan; j++ )
                        {
                            sal_uLong nRowKey = nRow + j;
                            SCROW nR = (*pTab1)[ nRowKey ];
                            if ( !nR )
                                (*pTab1)[ nRowKey ] = nRowsPerRow1;
                            else if ( nRowsPerRow1 > nR )
                                (*pTab1)[ nRowKey ] = nRowsPerRow1;
                            //TODO: How can we improve on this?
                            else if ( nRowsPerRow1 < nR && nRowSpan == 1
                              && nTable == nMaxTable )
                            {   // Still some space left, merge in a better way (if possible)
                                SCROW nAdd = nRowsPerRow1 - (nR % nRowsPerRow1);
                                nR += nAdd;
                                if ( (nR % nRows) == 0 )
                                {   // Only if representable
                                    SCROW nR2 = (*pTab1)[ nRowKey+1 ];
                                    if ( nR2 > nAdd )
                                    {   // Only if we really have enough space
                                        (*pTab1)[ nRowKey ] = nR;
                                        (*pTab1)[ nRowKey+1 ] = nR2 - nAdd;
                                        nRowsPerRow2 = nR / nRows;
                                    }
                                }
                            }
                        }
                    }
                    if ( nRowsPerRow2 > 1 )
                    {   // Inner
                        if ( !pTab2 )
                        {   // nRowsPerRow2 could be've been incremented
                            pTab2 = new InnerMap;
                            (*pTables)[ nTable ] = pTab2;
                        }
                        for ( SCROW j=0; j < nRows; j++ )
                        {
                            sal_uLong nRowKey = nRow + j;
                            (*pTab2)[ nRowKey ] = nRowsPerRow2;
                        }
                    }
                }
            }

            SetWidths();

            if ( !pE->nWidth )
                pE->nWidth = nTableWidth;
            else if ( pE->nWidth < nTableWidth )
            {
                sal_uInt16 nOldOffset = pE->nOffset + pE->nWidth;
                sal_uInt16 nNewOffset = pE->nOffset + nTableWidth;
                ModifyOffset( pS->pLocalColOffset, nOldOffset, nNewOffset, nOffsetTolerance );
                sal_uInt16 nTmp = nNewOffset - pE->nOffset - pE->nWidth;
                pE->nWidth = nNewOffset - pE->nOffset;
                pS->nTableWidth = pS->nTableWidth + nTmp;
                if ( pS->nColOffset >= nOldOffset )
                    pS->nColOffset = pS->nColOffset + nTmp;
            }

            nColCnt = pE->nCol + pE->nColOverlap;
            nRowCnt = pS->nRowCnt;
            nColCntStart = pS->nColCntStart;
            nMaxCol = pS->nMaxCol;
            nTable = pS->nTable;
            nTableWidth = pS->nTableWidth;
            nFirstTableCell = pS->nFirstTableCell;
            nColOffset = pS->nColOffset;
            nColOffsetStart = pS->nColOffsetStart;
            bFirstRow = pS->bFirstRow;
            xLockedList = pS->xLockedList;
            delete pLocalColOffset;
            pLocalColOffset = pS->pLocalColOffset;
            delete pActEntry;
            // pActEntry is kept around if a table is started in the same row
            // (anything's possible in HTML); will be deleted by CloseEntry
            pActEntry = pE;
            delete pS;
        }
        bTabInTabCell = true;
        bInCell = true;
    }
    else
    {   // Simple table finished
        SetWidths();
        nMaxCol = 0;
        nTable = 0;
        if ( !aTableStack.empty() )
        {
            ScHTMLTableStackEntry* pS = aTableStack.top();
            aTableStack.pop();
            delete pLocalColOffset;
            pLocalColOffset = pS->pLocalColOffset;
            delete pS;
        }
    }
}

void ScHTMLLayoutParser::Image( HtmlImportInfo* pInfo )
{
    pActEntry->maImageList.push_back( o3tl::make_unique<ScHTMLImage>() );
    ScHTMLImage* pImage = pActEntry->maImageList.back().get();
    const HTMLOptions& rOptions = static_cast<HTMLParser*>(pInfo->pParser)->GetOptions();
    for (const auto & rOption : rOptions)
    {
        switch( rOption.GetToken() )
        {
            case HtmlOptionId::SRC:
            {
                pImage->aURL = INetURLObject::GetAbsURL( aBaseURL, rOption.GetString() );
            }
            break;
            case HtmlOptionId::ALT:
            {
                if ( !pActEntry->bHasGraphic )
                {   // ALT text only if not any image loaded
                    if (!pActEntry->aAltText.isEmpty())
                        pActEntry->aAltText += "; ";

                    pActEntry->aAltText += rOption.GetString();
                }
            }
            break;
            case HtmlOptionId::WIDTH:
            {
                pImage->aSize.Width() = (long)rOption.GetNumber();
            }
            break;
            case HtmlOptionId::HEIGHT:
            {
                pImage->aSize.Height() = (long)rOption.GetNumber();
            }
            break;
            case HtmlOptionId::HSPACE:
            {
                pImage->aSpace.X() = (long)rOption.GetNumber();
            }
            break;
            case HtmlOptionId::VSPACE:
            {
                pImage->aSpace.Y() = (long)rOption.GetNumber();
            }
            break;
            default: break;
        }
    }
    if (pImage->aURL.isEmpty())
    {
        OSL_FAIL( "Image: graphic without URL ?!?" );
        return ;
    }

    sal_uInt16 nFormat;
    Graphic* pGraphic = new Graphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    if ( ERRCODE_NONE != GraphicFilter::LoadGraphic( pImage->aURL, pImage->aFilterName,
            *pGraphic, &rFilter, &nFormat ) )
    {
        delete pGraphic;
        return ; // Bad luck
    }
    if ( !pActEntry->bHasGraphic )
    {   // discard any ALT text in this cell if we have any image
        pActEntry->bHasGraphic = true;
        (pActEntry->aAltText).clear();
    }
    pImage->aFilterName = rFilter.GetImportFormatName( nFormat );
    pImage->pGraphic.reset( pGraphic );
    if ( !(pImage->aSize.Width() && pImage->aSize.Height()) )
    {
        OutputDevice* pDefaultDev = Application::GetDefaultDevice();
        pImage->aSize = pDefaultDev->LogicToPixel( pGraphic->GetPrefSize(),
            pGraphic->GetPrefMapMode() );
    }
    if ( pActEntry->maImageList.size() > 0 )
    {
        long nWidth = 0;
        for (std::unique_ptr<ScHTMLImage> & pI : pActEntry->maImageList)
        {
            if ( pI->nDir & nHorizontal )
                nWidth += pI->aSize.Width() + 2 * pI->aSpace.X();
            else
                nWidth = 0;
        }
        if ( pActEntry->nWidth
          && (nWidth + pImage->aSize.Width() + 2 * pImage->aSpace.X()
                >= pActEntry->nWidth) )
            pActEntry->maImageList.back()->nDir = nVertical;
    }
}

void ScHTMLLayoutParser::ColOn( HtmlImportInfo* pInfo )
{
    const HTMLOptions& rOptions = static_cast<HTMLParser*>(pInfo->pParser)->GetOptions();
    for (const auto & rOption : rOptions)
    {
        if( rOption.GetToken() == HtmlOptionId::WIDTH )
        {
            sal_uInt16 nVal = GetWidthPixel( rOption );
            MakeCol( pLocalColOffset, nColOffset, nVal, 0, 0 );
            nColOffset = nColOffset + nVal;
        }
    }
}

sal_uInt16 ScHTMLLayoutParser::GetWidthPixel( const HTMLOption& rOption )
{
    const OUString& rOptVal = rOption.GetString();
    if ( rOptVal.indexOf('%') != -1 )
    {   // Percent
        sal_uInt16 nW = (nTableWidth ? nTableWidth : (sal_uInt16) aPageSize.Width());
        return (sal_uInt16)((rOption.GetNumber() * nW) / 100);
    }
    else
    {
        if ( rOptVal.indexOf('*') != -1 )
        {   // Relative to what?
            // TODO: Collect all relative values in ColArray and then MakeCol
            return 0;
        }
        else
            return (sal_uInt16)rOption.GetNumber(); // Pixel
    }
}

void ScHTMLLayoutParser::AnchorOn( HtmlImportInfo* pInfo )
{
    const HTMLOptions& rOptions = static_cast<HTMLParser*>(pInfo->pParser)->GetOptions();
    for (const auto & rOption : rOptions)
    {
        if( rOption.GetToken() == HtmlOptionId::NAME )
            pActEntry->pName = new OUString(rOption.GetString());
    }
}

bool ScHTMLLayoutParser::IsAtBeginningOfText( HtmlImportInfo* pInfo )
{
    ESelection& rSel = pActEntry->aSel;
    return rSel.nStartPara == rSel.nEndPara &&
        rSel.nStartPara <= pInfo->aSelection.nEndPara &&
        pEdit->GetTextLen( rSel.nStartPara ) == 0;
}

void ScHTMLLayoutParser::FontOn( HtmlImportInfo* pInfo )
{
    if ( IsAtBeginningOfText( pInfo ) )
    {   // Only at the start of the text; applies to whole line
        const HTMLOptions& rOptions = static_cast<HTMLParser*>(pInfo->pParser)->GetOptions();
        for (const auto & rOption : rOptions)
        {
            switch( rOption.GetToken() )
            {
                case HtmlOptionId::FACE :
                {
                    const OUString& rFace = rOption.GetString();
                    OUString aFontName;
                    sal_Int32 nPos = 0;
                    while( nPos != -1 )
                    {
                        // Font list, VCL uses the semicolon as separator
                        // HTML uses the comma
                        OUString aFName = rFace.getToken( 0, ',', nPos );
                        aFName = comphelper::string::strip(aFName, ' ');
                        if( !aFontName.isEmpty() )
                            aFontName += ";";
                        aFontName += aFName;
                    }
                    if ( !aFontName.isEmpty() )
                        pActEntry->aItemSet.Put( SvxFontItem( FAMILY_DONTKNOW,
                            aFontName, EMPTY_OUSTRING, PITCH_DONTKNOW,
                            RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ) );
                }
                break;
                case HtmlOptionId::SIZE :
                {
                    sal_uInt16 nSize = (sal_uInt16) rOption.GetNumber();
                    if ( nSize == 0 )
                        nSize = 1;
                    else if ( nSize > SC_HTML_FONTSIZES )
                        nSize = SC_HTML_FONTSIZES;
                    pActEntry->aItemSet.Put( SvxFontHeightItem(
                        maFontHeights[nSize-1], 100, ATTR_FONT_HEIGHT ) );
                }
                break;
                case HtmlOptionId::COLOR :
                {
                    Color aColor;
                    rOption.GetColor( aColor );
                    pActEntry->aItemSet.Put( SvxColorItem( aColor, ATTR_FONT_COLOR ) );
                }
                break;
                default: break;
            }
        }
    }
}

void ScHTMLLayoutParser::ProcToken( HtmlImportInfo* pInfo )
{
    bool bSetLastToken = true;
    switch ( pInfo->nToken )
    {
        case HtmlTokenId::META:
        {
            HTMLParser* pParser = static_cast<HTMLParser*>(pInfo->pParser);
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                mpDoc->GetDocumentShell()->GetModel(), uno::UNO_QUERY_THROW);
            pParser->ParseMetaOptions(
                xDPS->getDocumentProperties(),
                mpDoc->GetDocumentShell()->GetHeaderAttributes() );
        }
        break;
        case HtmlTokenId::TITLE_ON:
        {
            bInTitle = true;
            aString.clear();
        }
        break;
        case HtmlTokenId::TITLE_OFF:
        {
            if ( bInTitle && !aString.isEmpty() )
            {
                // Remove blanks from line brakes
                aString = aString.trim();
                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    mpDoc->GetDocumentShell()->GetModel(),
                    uno::UNO_QUERY_THROW);
                xDPS->getDocumentProperties()->setTitle(aString);
            }
            bInTitle = false;
        }
        break;
        case HtmlTokenId::TABLE_ON:
        {
            TableOn( pInfo );
        }
        break;
        case HtmlTokenId::COL_ON:
        {
            ColOn( pInfo );
        }
        break;
        case HtmlTokenId::TABLEHEADER_ON:       // Opens row
        {
            if ( bInCell )
                CloseEntry( pInfo );
            // Do not set bInCell to true, TableDataOn does that
            pActEntry->aItemSet.Put(
                SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT) );
            SAL_FALLTHROUGH;
        }
        case HtmlTokenId::TABLEDATA_ON:         // Opens cell
        {
            TableDataOn( pInfo );
        }
        break;
        case HtmlTokenId::TABLEHEADER_OFF:
        case HtmlTokenId::TABLEDATA_OFF:        // Closes cell
        {
            TableDataOff( pInfo );
        }
        break;
        case HtmlTokenId::TABLEROW_ON:          // Before first cell in row
        {
            TableRowOn( pInfo );
        }
        break;
        case HtmlTokenId::TABLEROW_OFF:         // After last cell in row
        {
            TableRowOff( pInfo );
        }
        break;
        case HtmlTokenId::TABLE_OFF:
        {
            TableOff( pInfo );
        }
        break;
        case HtmlTokenId::IMAGE:
        {
            Image( pInfo );
        }
        break;
        case HtmlTokenId::PARABREAK_OFF:
        {   // We continue vertically after an image
            if ( pActEntry->maImageList.size() > 0 )
                pActEntry->maImageList.back()->nDir = nVertical;
        }
        break;
        case HtmlTokenId::ANCHOR_ON:
        {
            AnchorOn( pInfo );
        }
        break;
        case HtmlTokenId::FONT_ON :
        {
            FontOn( pInfo );
        }
        break;
        case HtmlTokenId::BIGPRINT_ON :
        {
            // TODO: Remember current font size and increase by 1
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxFontHeightItem(
                    maFontHeights[3], 100, ATTR_FONT_HEIGHT ) );
        }
        break;
        case HtmlTokenId::SMALLPRINT_ON :
        {
            // TODO: Remember current font size and decrease by 1
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxFontHeightItem(
                    maFontHeights[0], 100, ATTR_FONT_HEIGHT ) );
        }
        break;
        case HtmlTokenId::BOLD_ON :
        case HtmlTokenId::STRONG_ON :
        {
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxWeightItem( WEIGHT_BOLD,
                    ATTR_FONT_WEIGHT ) );
        }
        break;
        case HtmlTokenId::ITALIC_ON :
        case HtmlTokenId::EMPHASIS_ON :
        case HtmlTokenId::ADDRESS_ON :
        case HtmlTokenId::BLOCKQUOTE_ON :
        case HtmlTokenId::BLOCKQUOTE30_ON :
        case HtmlTokenId::CITIATION_ON :
        case HtmlTokenId::VARIABLE_ON :
        {
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxPostureItem( ITALIC_NORMAL,
                    ATTR_FONT_POSTURE ) );
        }
        break;
        case HtmlTokenId::DEFINSTANCE_ON :
        {
            if ( IsAtBeginningOfText( pInfo ) )
            {
                pActEntry->aItemSet.Put( SvxWeightItem( WEIGHT_BOLD,
                    ATTR_FONT_WEIGHT ) );
                pActEntry->aItemSet.Put( SvxPostureItem( ITALIC_NORMAL,
                    ATTR_FONT_POSTURE ) );
            }
        }
        break;
        case HtmlTokenId::UNDERLINE_ON :
        {
            if ( IsAtBeginningOfText( pInfo ) )
                pActEntry->aItemSet.Put( SvxUnderlineItem( LINESTYLE_SINGLE,
                    ATTR_FONT_UNDERLINE ) );
        }
        break;
        case HtmlTokenId::TEXTTOKEN:
        {
            if ( bInTitle )
                aString += pInfo->aText;
        }
        break;
        default:
        {   // Don't set nHtmlLastToken!
            bSetLastToken = false;
        }
    }
    if ( bSetLastToken )
        nHtmlLastToken = pInfo->nToken;
}

// HTML DATA QUERY PARSER

template< typename Type >
inline Type getLimitedValue( const Type& rValue, const Type& rMin, const Type& rMax )
{ return std::max( std::min( rValue, rMax ), rMin ); }

ScHTMLEntry::ScHTMLEntry( const SfxItemSet& rItemSet, ScHTMLTableId nTableId ) :
    ScEEParseEntry( rItemSet ),
    mbImportAlways( false )
{
    nTab = nTableId;
    bEntirePara = false;
}

bool ScHTMLEntry::HasContents() const
{
     return mbImportAlways || aSel.HasRange() || !aAltText.isEmpty() || IsTable();
}

void ScHTMLEntry::AdjustStart( const HtmlImportInfo& rInfo )
{
    // set start position
    aSel.nStartPara = rInfo.aSelection.nStartPara;
    aSel.nStartPos = rInfo.aSelection.nStartPos;
    // adjust end position
    if( (aSel.nEndPara < aSel.nStartPara) || ((aSel.nEndPara == aSel.nStartPara) && (aSel.nEndPos < aSel.nStartPos)) )
    {
        aSel.nEndPara = aSel.nStartPara;
        aSel.nEndPos = aSel.nStartPos;
    }
}

void ScHTMLEntry::AdjustEnd( const HtmlImportInfo& rInfo )
{
    OSL_ENSURE( (aSel.nEndPara < rInfo.aSelection.nEndPara) ||
                ((aSel.nEndPara == rInfo.aSelection.nEndPara) && (aSel.nEndPos <= rInfo.aSelection.nEndPos)),
                "ScHTMLQueryParser::AdjustEntryEnd - invalid end position" );
    // set end position
    aSel.nEndPara = rInfo.aSelection.nEndPara;
    aSel.nEndPos = rInfo.aSelection.nEndPos;
}

void ScHTMLEntry::Strip( const EditEngine& rEditEngine )
{
    // strip leading empty paragraphs
    while( (aSel.nStartPara < aSel.nEndPara) && (rEditEngine.GetTextLen( aSel.nStartPara ) <= aSel.nStartPos) )
    {
        ++aSel.nStartPara;
        aSel.nStartPos = 0;
    }
    // strip trailing empty paragraphs
    while( (aSel.nStartPara < aSel.nEndPara) && (aSel.nEndPos == 0) )
    {
        --aSel.nEndPara;
        aSel.nEndPos = rEditEngine.GetTextLen( aSel.nEndPara );
    }
}

/** A map of ScHTMLTable objects.

    Organizes the tables with a unique table key. Stores nested tables inside
    the parent table and forms in this way a tree structure of tables. An
    instance of this class ownes the contained table objects and deletes them
    on destruction.
 */
class ScHTMLTableMap final
{
private:
    typedef std::shared_ptr< ScHTMLTable >          ScHTMLTablePtr;
    typedef std::map< ScHTMLTableId, ScHTMLTablePtr > ScHTMLTableStdMap;

public:
    typedef ScHTMLTableStdMap::iterator             iterator;
    typedef ScHTMLTableStdMap::const_iterator       const_iterator;

private:
    ScHTMLTable&        mrParentTable;      /// Reference to parent table.
    ScHTMLTableStdMap   maTables;           /// Container for all table objects.
    mutable ScHTMLTable* mpCurrTable;       /// Current table, used for fast search.

public:
    explicit            ScHTMLTableMap( ScHTMLTable& rParentTable );

    const_iterator begin() const { return maTables.begin(); }
    const_iterator end() const { return maTables.end(); }

    /** Returns the specified table.
        @param nTableId  Unique identifier of the table.
        @param bDeep  true = searches deep in all nested table; false = only in this container. */
    ScHTMLTable*        FindTable( ScHTMLTableId nTableId, bool bDeep = true ) const;

    /** Inserts a new table into the container. This container owns the created table.
        @param bPreFormText  true = New table is based on preformatted text (<pre> tag). */
    ScHTMLTable*        CreateTable( const HtmlImportInfo& rInfo, bool bPreFormText );

private:
    /** Sets a working table with its index for search optimization. */
    void         SetCurrTable( ScHTMLTable* pTable ) const
                            { if( pTable ) mpCurrTable = pTable; }
};

ScHTMLTableMap::ScHTMLTableMap( ScHTMLTable& rParentTable ) :
    mrParentTable(rParentTable),
    mpCurrTable(nullptr)
{
}

ScHTMLTable* ScHTMLTableMap::FindTable( ScHTMLTableId nTableId, bool bDeep ) const
{
    ScHTMLTable* pResult = nullptr;
    if( mpCurrTable && (nTableId == mpCurrTable->GetTableId()) )
        pResult = mpCurrTable;              // cached table
    else
    {
        const_iterator aFind = maTables.find( nTableId );
        if( aFind != maTables.end() )
            pResult = aFind->second.get();  // table from this container
    }

    // not found -> search deep in nested tables
    if( !pResult && bDeep )
        for( const_iterator aIter = begin(), aEnd = end(); !pResult && (aIter != aEnd); ++aIter )
            pResult = aIter->second->FindNestedTable( nTableId );

    SetCurrTable( pResult );
    return pResult;
}

ScHTMLTable* ScHTMLTableMap::CreateTable( const HtmlImportInfo& rInfo, bool bPreFormText )
{
    ScHTMLTable* pTable = new ScHTMLTable( mrParentTable, rInfo, bPreFormText );
    maTables[ pTable->GetTableId() ].reset( pTable );
    SetCurrTable( pTable );
    return pTable;
}

/** Simplified forward iterator for convenience.

    Before the iterator can be dereferenced, it must be tested with the is()
    method. The iterator may be invalid directly after construction (e.g. empty
    container).
 */
class ScHTMLTableIterator
{
public:
    /** Constructs the iterator for the passed table map.
        @param pTableMap  Pointer to the table map (is allowed to be NULL). */
    explicit            ScHTMLTableIterator( const ScHTMLTableMap* pTableMap );

    bool         is() const { return mpTableMap && maIter != maEnd; }
    ScHTMLTable* operator->() { return maIter->second.get(); }
    ScHTMLTableIterator& operator++() { ++maIter; return *this; }

private:
    ScHTMLTableMap::const_iterator maIter;
    ScHTMLTableMap::const_iterator maEnd;
    const ScHTMLTableMap* mpTableMap;
};

ScHTMLTableIterator::ScHTMLTableIterator( const ScHTMLTableMap* pTableMap ) :
    mpTableMap(pTableMap)
{
    if( pTableMap )
    {
        maIter = pTableMap->begin();
        maEnd = pTableMap->end();
    }
}

ScHTMLTableAutoId::ScHTMLTableAutoId( ScHTMLTableId& rnUnusedId ) :
    mnTableId( rnUnusedId ),
    mrnUnusedId( rnUnusedId )
{
    ++mrnUnusedId;
}

ScHTMLTable::ScHTMLTable( ScHTMLTable& rParentTable, const HtmlImportInfo& rInfo, bool bPreFormText ) :
    mpParentTable( &rParentTable ),
    maTableId( rParentTable.maTableId.mrnUnusedId ),
    maTableItemSet( rParentTable.GetCurrItemSet() ),
    mrEditEngine( rParentTable.mrEditEngine ),
    mrEEParseList( rParentTable.mrEEParseList ),
    mpCurrEntryList( nullptr ),
    maSize( 1, 1 ),
    mpParser(rParentTable.mpParser),
    mbBorderOn( false ),
    mbPreFormText( bPreFormText ),
    mbRowOn( false ),
    mbDataOn( false ),
    mbPushEmptyLine( false )
{
    if( mbPreFormText )
    {
        ImplRowOn();
        ImplDataOn( ScHTMLSize( 1, 1 ) );
    }
    else
    {
        ProcessFormatOptions( maTableItemSet, rInfo );
        const HTMLOptions& rOptions = static_cast<HTMLParser*>(rInfo.pParser)->GetOptions();
        HTMLOptions::const_iterator itr = rOptions.begin(), itrEnd = rOptions.end();
        for (; itr != itrEnd; ++itr)
        {
            switch( itr->GetToken() )
            {
                case HtmlOptionId::BORDER:
                    mbBorderOn = itr->GetString().isEmpty() || (itr->GetNumber() != 0);
                break;
                case HtmlOptionId::ID:
                    maTableName = itr->GetString();
                break;
                default: break;
            }
        }
    }

    CreateNewEntry( rInfo );
}

ScHTMLTable::ScHTMLTable(
    SfxItemPool& rPool,
    EditEngine& rEditEngine,
    std::vector< ScEEParseEntry* >& rEEParseList,
    ScHTMLTableId& rnUnusedId, ScHTMLParser* pParser
) :
    mpParentTable( nullptr ),
    maTableId( rnUnusedId ),
    maTableItemSet( rPool ),
    mrEditEngine( rEditEngine ),
    mrEEParseList( rEEParseList ),
    mpCurrEntryList( nullptr ),
    maSize( 1, 1 ),
    mpParser(pParser),
    mbBorderOn( false ),
    mbPreFormText( false ),
    mbRowOn( false ),
    mbDataOn( false ),
    mbPushEmptyLine( false )
{
    // open the first "cell" of the document
    ImplRowOn();
    ImplDataOn( ScHTMLSize( 1, 1 ) );
    mxCurrEntry = CreateEntry();
}

ScHTMLTable::~ScHTMLTable()
{
}

const SfxItemSet& ScHTMLTable::GetCurrItemSet() const
{
    // first try cell item set, then row item set, then table item set
    return mxDataItemSet.get() ? *mxDataItemSet : (mxRowItemSet.get() ? *mxRowItemSet : maTableItemSet);
}

ScHTMLSize ScHTMLTable::GetSpan( const ScHTMLPos& rCellPos ) const
{
    ScHTMLSize aSpan( 1, 1 );
    const ScRange* pRange = nullptr;
    if(  ( (pRange = maVMergedCells.Find( rCellPos.MakeAddr() ) ) != nullptr)
      || ( (pRange = maHMergedCells.Find( rCellPos.MakeAddr() ) ) != nullptr)
      )
        aSpan.Set( pRange->aEnd.Col() - pRange->aStart.Col() + 1, pRange->aEnd.Row() - pRange->aStart.Row() + 1 );
    return aSpan;
}

ScHTMLTable* ScHTMLTable::FindNestedTable( ScHTMLTableId nTableId ) const
{
    return mxNestedTables.get() ? mxNestedTables->FindTable( nTableId ) : nullptr;
}

void ScHTMLTable::PutItem( const SfxPoolItem& rItem )
{
    OSL_ENSURE( mxCurrEntry.get(), "ScHTMLTable::PutItem - no current entry" );
    if( mxCurrEntry.get() && mxCurrEntry->IsEmpty() )
        mxCurrEntry->GetItemSet().Put( rItem );
}

void ScHTMLTable::PutText( const HtmlImportInfo& rInfo )
{
    OSL_ENSURE( mxCurrEntry.get(), "ScHTMLTable::PutText - no current entry" );
    if( mxCurrEntry.get() )
    {
        if( !mxCurrEntry->HasContents() && IsSpaceCharInfo( rInfo ) )
            mxCurrEntry->AdjustStart( rInfo );
        else
            mxCurrEntry->AdjustEnd( rInfo );
    }
}

void ScHTMLTable::InsertPara( const HtmlImportInfo& rInfo )
{
    if( mxCurrEntry.get() && mbDataOn && !IsEmptyCell() )
        mxCurrEntry->SetImportAlways();
    PushEntry( rInfo );
    CreateNewEntry( rInfo );
    InsertLeadingEmptyLine();
}

void ScHTMLTable::BreakOn()
{
    // empty line, if <br> is at start of cell
    mbPushEmptyLine = !mbPreFormText && mbDataOn && IsEmptyCell();
}

void ScHTMLTable::HeadingOn()
{
    // call directly, InsertPara() has not been called before
    InsertLeadingEmptyLine();
}

void ScHTMLTable::InsertLeadingEmptyLine()
{
    // empty line, if <p>, </p>, <h?>, or </h*> are not at start of cell
    mbPushEmptyLine = !mbPreFormText && mbDataOn && !IsEmptyCell();
}

void ScHTMLTable::AnchorOn()
{
    OSL_ENSURE( mxCurrEntry.get(), "ScHTMLTable::AnchorOn - no current entry" );
    // don't skip entries with single hyperlinks
    if( mxCurrEntry.get() )
        mxCurrEntry->SetImportAlways();
}

ScHTMLTable* ScHTMLTable::TableOn( const HtmlImportInfo& rInfo )
{
    PushEntry( rInfo );
    return InsertNestedTable( rInfo, false );
}

ScHTMLTable* ScHTMLTable::TableOff( const HtmlImportInfo& rInfo )
{
    return mbPreFormText ? this : CloseTable( rInfo );
}

ScHTMLTable* ScHTMLTable::PreOn( const HtmlImportInfo& rInfo )
{
    PushEntry( rInfo );
    return InsertNestedTable( rInfo, true );
}

ScHTMLTable* ScHTMLTable::PreOff( const HtmlImportInfo& rInfo )
{
    return mbPreFormText ? CloseTable( rInfo ) : this;
}

void ScHTMLTable::RowOn( const HtmlImportInfo& rInfo )
{
    PushEntry( rInfo, true );
    if( mpParentTable && !mbPreFormText )   // no rows allowed in global and preformatted tables
    {
        ImplRowOn();
        ProcessFormatOptions( *mxRowItemSet, rInfo );
    }
    CreateNewEntry( rInfo );
}

void ScHTMLTable::RowOff( const HtmlImportInfo& rInfo )
{
    PushEntry( rInfo, true );
    if( mpParentTable && !mbPreFormText )   // no rows allowed in global and preformatted tables
        ImplRowOff();
    CreateNewEntry( rInfo );
}

namespace {

/**
 * Decode a number format string stored in Excel-generated HTML's CSS
 * region.
 */
OUString decodeNumberFormat(const OUString& rFmt)
{
    OUStringBuffer aBuf;
    const sal_Unicode* p = rFmt.getStr();
    sal_Int32 n = rFmt.getLength();
    for (sal_Int32 i = 0; i < n; ++i, ++p)
    {
        if (*p == '\\')
        {
            // Skip '\'.
            ++i;
            ++p;

            // Parse all subsequent digits until first non-digit is found.
            sal_Int32 nDigitCount = 0;
            const sal_Unicode* p1 = p;
            for (; i < n; ++i, ++p, ++nDigitCount)
            {
                if (*p < '0' || '9' < *p)
                {
                    --i;
                    --p;
                    break;
                }

            }
            if (nDigitCount)
            {
                // Hex-encoded character found. Decode it back into its
                // original character. An example of number format with
                // hex-encoded chars: "\0022$\0022\#\,\#\#0\.00"
                sal_uInt32 nVal = OUString(p1, nDigitCount).toUInt32(16);
                aBuf.append(static_cast<sal_Unicode>(nVal));
            }
        }
        else
            aBuf.append(*p);
    }
    return aBuf.makeStringAndClear();
}

}

void ScHTMLTable::DataOn( const HtmlImportInfo& rInfo )
{
    PushEntry( rInfo, true );
    if( mpParentTable && !mbPreFormText )   // no cells allowed in global and preformatted tables
    {
        // read needed options from the <td> tag
        ScHTMLSize aSpanSize( 1, 1 );
        std::unique_ptr<OUString> pValStr, pNumStr;
        const HTMLOptions& rOptions = static_cast<HTMLParser*>(rInfo.pParser)->GetOptions();
        HTMLOptions::const_iterator itr = rOptions.begin(), itrEnd = rOptions.end();
        sal_uInt32 nNumberFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;
        for (; itr != itrEnd; ++itr)
        {
            switch (itr->GetToken())
            {
                case HtmlOptionId::COLSPAN:
                    aSpanSize.mnCols = static_cast<SCCOL>( getLimitedValue<sal_Int32>( itr->GetString().toInt32(), 1, 256 ) );
                break;
                case HtmlOptionId::ROWSPAN:
                    aSpanSize.mnRows = static_cast<SCROW>( getLimitedValue<sal_Int32>( itr->GetString().toInt32(), 1, 256 ) );
                break;
                case HtmlOptionId::SDVAL:
                    pValStr.reset(new OUString(itr->GetString()));
                break;
                case HtmlOptionId::SDNUM:
                    pNumStr.reset(new OUString(itr->GetString()));
                break;
                case HtmlOptionId::CLASS:
                {
                    // Pick up the number format associated with this class (if
                    // any).
                    OUString aElem("td");
                    OUString aClass = itr->GetString();
                    OUString aProp("mso-number-format");
                    const ScHTMLStyles& rStyles = mpParser->GetStyles();
                    const OUString& rVal = rStyles.getPropertyValue(aElem, aClass, aProp);
                    if (!rVal.isEmpty())
                    {
                        OUString aNumFmt = decodeNumberFormat(rVal);

                        nNumberFormat = GetFormatTable()->GetEntryKey(aNumFmt);
                        if (nNumberFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
                        {
                            sal_Int32 nErrPos  = 0;
                            short nDummy;
                            bool bValidFmt = GetFormatTable()->PutEntry(aNumFmt, nErrPos, nDummy, nNumberFormat);
                            if (!bValidFmt)
                                nNumberFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;
                        }
                    }
                }
                break;
                default: break;
            }
        }

        ImplDataOn( aSpanSize );

        if (nNumberFormat != NUMBERFORMAT_ENTRY_NOT_FOUND)
            mxDataItemSet->Put( SfxUInt32Item(ATTR_VALUE_FORMAT, nNumberFormat) );

        ProcessFormatOptions( *mxDataItemSet, rInfo );
        CreateNewEntry( rInfo );
        mxCurrEntry->pValStr = pValStr.release();
        mxCurrEntry->pNumStr = pNumStr.release();
    }
    else
        CreateNewEntry( rInfo );
}

void ScHTMLTable::DataOff( const HtmlImportInfo& rInfo )
{
    PushEntry( rInfo, true );
    if( mpParentTable && !mbPreFormText )   // no cells allowed in global and preformatted tables
        ImplDataOff();
    CreateNewEntry( rInfo );
}

void ScHTMLTable::BodyOn( const HtmlImportInfo& rInfo )
{
    bool bPushed = PushEntry( rInfo );
    if( !mpParentTable )
    {
        // do not start new row, if nothing (no title) precedes the body.
        if( bPushed || !mbRowOn )
            ImplRowOn();
        if( bPushed || !mbDataOn )
            ImplDataOn( ScHTMLSize( 1, 1 ) );
        ProcessFormatOptions( *mxDataItemSet, rInfo );
    }
    CreateNewEntry( rInfo );
}

void ScHTMLTable::BodyOff( const HtmlImportInfo& rInfo )
{
    PushEntry( rInfo );
    if( !mpParentTable )
    {
        ImplDataOff();
        ImplRowOff();
    }
    CreateNewEntry( rInfo );
}

ScHTMLTable* ScHTMLTable::CloseTable( const HtmlImportInfo& rInfo )
{
    if( mpParentTable )     // not allowed to close global table
    {
        PushEntry( rInfo, mbDataOn );
        ImplDataOff();
        ImplRowOff();
        mpParentTable->PushTableEntry( GetTableId() );
        mpParentTable->CreateNewEntry( rInfo );
        if( mbPreFormText ) // enclose preformatted table with empty lines in parent table
            mpParentTable->InsertLeadingEmptyLine();
        return mpParentTable;
    }
    return this;
}

SCCOLROW ScHTMLTable::GetDocSize( ScHTMLOrient eOrient, SCCOLROW nCellPos ) const
{
    const ScSizeVec& rSizes = maCumSizes[ eOrient ];
    size_t nIndex = static_cast< size_t >( nCellPos );
    if( nIndex >= rSizes.size() ) return 0;
    return (nIndex == 0) ? rSizes.front() : (rSizes[ nIndex ] - rSizes[ nIndex - 1 ]);
}

SCCOLROW ScHTMLTable::GetDocSize( ScHTMLOrient eOrient, SCCOLROW nCellBegin, SCCOLROW nCellEnd ) const
{
    const ScSizeVec& rSizes = maCumSizes[ eOrient ];
    size_t nBeginIdx = static_cast< size_t >( std::max< SCCOLROW >( nCellBegin, 0 ) );
    size_t nEndIdx = static_cast< size_t >( std::min< SCCOLROW >( nCellEnd, static_cast< SCCOLROW >( rSizes.size() ) ) );
    if (nBeginIdx >= nEndIdx ) return 0;
    return rSizes[ nEndIdx - 1 ] - ((nBeginIdx == 0) ? 0 : rSizes[ nBeginIdx - 1 ]);
}

SCCOLROW ScHTMLTable::GetDocSize( ScHTMLOrient eOrient ) const
{
    const ScSizeVec& rSizes = maCumSizes[ eOrient ];
    return rSizes.empty() ? 0 : rSizes.back();
}

ScHTMLSize ScHTMLTable::GetDocSize( const ScHTMLPos& rCellPos ) const
{
    ScHTMLSize aCellSpan = GetSpan( rCellPos );
    return ScHTMLSize(
        static_cast< SCCOL >( GetDocSize( tdCol, rCellPos.mnCol, rCellPos.mnCol + aCellSpan.mnCols ) ),
        static_cast< SCROW >( GetDocSize( tdRow, rCellPos.mnRow, rCellPos.mnRow + aCellSpan.mnRows ) ) );
}

SCCOLROW ScHTMLTable::GetDocPos( ScHTMLOrient eOrient, SCCOLROW nCellPos ) const
{
    return maDocBasePos.Get( eOrient ) + GetDocSize( eOrient, 0, nCellPos );
}

ScHTMLPos ScHTMLTable::GetDocPos( const ScHTMLPos& rCellPos ) const
{
    return ScHTMLPos(
        static_cast< SCCOL >( GetDocPos( tdCol, rCellPos.mnCol ) ),
        static_cast< SCROW >( GetDocPos( tdRow, rCellPos.mnRow ) ) );
}

void ScHTMLTable::GetDocRange( ScRange& rRange ) const
{
    rRange.aStart = rRange.aEnd = maDocBasePos.MakeAddr();
    ScAddress aErrorPos( ScAddress::UNINITIALIZED );
    if (!rRange.aEnd.Move( static_cast< SCCOL >( GetDocSize( tdCol ) ) - 1,
                static_cast< SCROW >( GetDocSize( tdRow ) ) - 1, 0, aErrorPos))
    {
        assert(!"can't move");
    }
}

void ScHTMLTable::ApplyCellBorders( ScDocument* pDoc, const ScAddress& rFirstPos ) const
{
    OSL_ENSURE( pDoc, "ScHTMLTable::ApplyCellBorders - no document" );
    if( pDoc && mbBorderOn )
    {
        const SCCOL nLastCol = maSize.mnCols - 1;
        const SCROW nLastRow = maSize.mnRows - 1;
        const long nOuterLine = DEF_LINE_WIDTH_2;
        const long nInnerLine = DEF_LINE_WIDTH_0;
        SvxBorderLine aOuterLine(nullptr, nOuterLine, SvxBorderLineStyle::SOLID);
        SvxBorderLine aInnerLine(nullptr, nInnerLine, SvxBorderLineStyle::SOLID);
        SvxBoxItem aBorderItem( ATTR_BORDER );

        for( SCCOL nCol = 0; nCol <= nLastCol; ++nCol )
        {
            SvxBorderLine* pLeftLine = (nCol == 0) ? &aOuterLine : &aInnerLine;
            SvxBorderLine* pRightLine = (nCol == nLastCol) ? &aOuterLine : &aInnerLine;
            SCCOL nCellCol1 = static_cast< SCCOL >( GetDocPos( tdCol, nCol ) ) + rFirstPos.Col();
            SCCOL nCellCol2 = nCellCol1 + static_cast< SCCOL >( GetDocSize( tdCol, nCol ) ) - 1;
            for( SCROW nRow = 0; nRow <= nLastRow; ++nRow )
            {
                SvxBorderLine* pTopLine = (nRow == 0) ? &aOuterLine : &aInnerLine;
                SvxBorderLine* pBottomLine = (nRow == nLastRow) ? &aOuterLine : &aInnerLine;
                SCROW nCellRow1 = GetDocPos( tdRow, nRow ) + rFirstPos.Row();
                SCROW nCellRow2 = nCellRow1 + GetDocSize( tdRow, nRow ) - 1;
                for( SCCOL nCellCol = nCellCol1; nCellCol <= nCellCol2; ++nCellCol )
                {
                    aBorderItem.SetLine( (nCellCol == nCellCol1) ? pLeftLine : nullptr, SvxBoxItemLine::LEFT );
                    aBorderItem.SetLine( (nCellCol == nCellCol2) ? pRightLine : nullptr, SvxBoxItemLine::RIGHT );
                    for( SCROW nCellRow = nCellRow1; nCellRow <= nCellRow2; ++nCellRow )
                    {
                        aBorderItem.SetLine( (nCellRow == nCellRow1) ? pTopLine : nullptr, SvxBoxItemLine::TOP );
                        aBorderItem.SetLine( (nCellRow == nCellRow2) ? pBottomLine : nullptr, SvxBoxItemLine::BOTTOM );
                        pDoc->ApplyAttr( nCellCol, nCellRow, rFirstPos.Tab(), aBorderItem );
                    }
                }
            }
        }
    }

    for( ScHTMLTableIterator aIter( mxNestedTables.get() ); aIter.is(); ++aIter )
        aIter->ApplyCellBorders( pDoc, rFirstPos );
}

SvNumberFormatter* ScHTMLTable::GetFormatTable()
{
    return mpParser->GetDoc().GetFormatTable();
}

bool ScHTMLTable::IsEmptyCell() const
{
    return mpCurrEntryList && mpCurrEntryList->empty();
}

bool ScHTMLTable::IsSpaceCharInfo( const HtmlImportInfo& rInfo )
{
    return (rInfo.nToken == HtmlTokenId::TEXTTOKEN) && (rInfo.aText.getLength() == 1) && (rInfo.aText[ 0 ] == ' ');
}

ScHTMLTable::ScHTMLEntryPtr ScHTMLTable::CreateEntry() const
{
    return o3tl::make_unique<ScHTMLEntry>( GetCurrItemSet() );
}

void ScHTMLTable::CreateNewEntry( const HtmlImportInfo& rInfo )
{
    OSL_ENSURE( !mxCurrEntry.get(), "ScHTMLTable::CreateNewEntry - old entry still present" );
    mxCurrEntry = CreateEntry();
    mxCurrEntry->aSel = rInfo.aSelection;
}

void ScHTMLTable::ImplPushEntryToList( ScHTMLEntryList& rEntryList, ScHTMLEntryPtr& rxEntry )
{
    // HTML entry list does not own the entries
    rEntryList.push_back( rxEntry.get() );
    // mrEEParseList (reference to member of ScEEParser) owns the entries
    mrEEParseList.push_back( rxEntry.release() );
}

bool ScHTMLTable::PushEntry( ScHTMLEntryPtr& rxEntry )
{
    bool bPushed = false;
    if( rxEntry.get() && rxEntry->HasContents() )
    {
        if( mpCurrEntryList )
        {
            if( mbPushEmptyLine )
            {
                ScHTMLEntryPtr xEmptyEntry = CreateEntry();
                ImplPushEntryToList( *mpCurrEntryList, xEmptyEntry );
                mbPushEmptyLine = false;
            }
            ImplPushEntryToList( *mpCurrEntryList, rxEntry );
            bPushed = true;
        }
        else if( mpParentTable )
        {
            bPushed = mpParentTable->PushEntry( rxEntry );
        }
        else
        {
            OSL_FAIL( "ScHTMLTable::PushEntry - cannot push entry, no parent found" );
        }
    }
    return bPushed;
}

bool ScHTMLTable::PushEntry( const HtmlImportInfo& rInfo, bool bLastInCell )
{
    OSL_ENSURE( mxCurrEntry.get(), "ScHTMLTable::PushEntry - no current entry" );
    bool bPushed = false;
    if( mxCurrEntry.get() )
    {
        mxCurrEntry->AdjustEnd( rInfo );
        mxCurrEntry->Strip( mrEditEngine );

        // import entry always, if it is the last in cell, and cell is still empty
        if( bLastInCell && IsEmptyCell() )
        {
            mxCurrEntry->SetImportAlways();
            // don't insert empty lines before single empty entries
            if( mxCurrEntry->IsEmpty() )
                mbPushEmptyLine = false;
        }

        bPushed = PushEntry( mxCurrEntry );
        mxCurrEntry.reset();
    }
    return bPushed;
}

void ScHTMLTable::PushTableEntry( ScHTMLTableId nTableId )
{
    OSL_ENSURE( nTableId != SC_HTML_GLOBAL_TABLE, "ScHTMLTable::PushTableEntry - cannot push global table" );
    if( nTableId != SC_HTML_GLOBAL_TABLE )
    {
        ScHTMLEntryPtr xEntry( new ScHTMLEntry( maTableItemSet, nTableId ) );
        PushEntry( xEntry );
    }
}

ScHTMLTable* ScHTMLTable::GetExistingTable( ScHTMLTableId nTableId ) const
{
    ScHTMLTable* pTable = ((nTableId != SC_HTML_GLOBAL_TABLE) && mxNestedTables.get()) ?
        mxNestedTables->FindTable( nTableId, false ) : nullptr;
    OSL_ENSURE( pTable || (nTableId == SC_HTML_GLOBAL_TABLE), "ScHTMLTable::GetExistingTable - table not found" );
    return pTable;
}

ScHTMLTable* ScHTMLTable::InsertNestedTable( const HtmlImportInfo& rInfo, bool bPreFormText )
{
    if( !mxNestedTables.get() )
        mxNestedTables.reset( new ScHTMLTableMap( *this ) );
    if( bPreFormText )      // enclose new preformatted table with empty lines
        InsertLeadingEmptyLine();
    return mxNestedTables->CreateTable( rInfo, bPreFormText );
}

void ScHTMLTable::InsertNewCell( const ScHTMLSize& rSpanSize )
{
    ScRange* pRange;

    /*  Find an unused cell by skipping all merged ranges that cover the
        current cell position stored in maCurrCell. */
    while( ((pRange = maVMergedCells.Find( maCurrCell.MakeAddr() )) != nullptr) || ((pRange = maHMergedCells.Find( maCurrCell.MakeAddr() )) != nullptr) )
        maCurrCell.mnCol = pRange->aEnd.Col() + 1;
    mpCurrEntryList = &maEntryMap[ maCurrCell ];

    /*  If the new cell is merged horizontally, try to find collisions with
        other vertically merged ranges. In this case, shrink existing
        vertically merged ranges (do not shrink the new cell). */
    SCCOL nColEnd = maCurrCell.mnCol + rSpanSize.mnCols;
    for( ScAddress aAddr( maCurrCell.MakeAddr() ); aAddr.Col() < nColEnd; aAddr.IncCol() )
        if( (pRange = maVMergedCells.Find( aAddr )) != nullptr )
            pRange->aEnd.SetRow( maCurrCell.mnRow - 1 );

    // insert the new range into the cell lists
    ScRange aNewRange( maCurrCell.MakeAddr() );
    ScAddress aErrorPos( ScAddress::UNINITIALIZED );
    if (!aNewRange.aEnd.Move( rSpanSize.mnCols - 1, rSpanSize.mnRows - 1, 0, aErrorPos))
    {
        assert(!"can't move");
    }
    if( rSpanSize.mnRows > 1 )
    {
        maVMergedCells.Append( aNewRange );
        /*  Do not insert vertically merged ranges into maUsedCells yet,
            because they may be shrunken (see above). The final vertically
            merged ranges are inserted in FillEmptyCells(). */
    }
    else
    {
        if( rSpanSize.mnCols > 1 )
            maHMergedCells.Append( aNewRange );
        /*  Insert horizontally merged ranges and single cells into
            maUsedCells, they will not be changed anymore. */
        maUsedCells.Join( aNewRange );
    }

    // adjust table size
    maSize.mnCols = std::max< SCCOL >( maSize.mnCols, aNewRange.aEnd.Col() + 1 );
    maSize.mnRows = std::max< SCROW >( maSize.mnRows, aNewRange.aEnd.Row() + 1 );
}

void ScHTMLTable::ImplRowOn()
{
    if( mbRowOn )
        ImplRowOff();
    mxRowItemSet.reset( new SfxItemSet( maTableItemSet ) );
    maCurrCell.mnCol = 0;
    mbRowOn = true;
    mbDataOn = false;
}

void ScHTMLTable::ImplRowOff()
{
    if( mbDataOn )
        ImplDataOff();
    if( mbRowOn )
    {
        mxRowItemSet.reset();
        ++maCurrCell.mnRow;
        mbRowOn = mbDataOn = false;
    }
}

void ScHTMLTable::ImplDataOn( const ScHTMLSize& rSpanSize )
{
    if( mbDataOn )
        ImplDataOff();
    if( !mbRowOn )
        ImplRowOn();
    mxDataItemSet.reset( new SfxItemSet( *mxRowItemSet ) );
    InsertNewCell( rSpanSize );
    mbDataOn = true;
    mbPushEmptyLine = false;
}

void ScHTMLTable::ImplDataOff()
{
    if( mbDataOn )
    {
        mxDataItemSet.reset();
        ++maCurrCell.mnCol;
        mpCurrEntryList = nullptr;
        mbDataOn = false;
    }
}

void ScHTMLTable::ProcessFormatOptions( SfxItemSet& rItemSet, const HtmlImportInfo& rInfo )
{
    // special handling for table header cells
    if( rInfo.nToken == HtmlTokenId::TABLEHEADER_ON )
    {
        rItemSet.Put( SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT ) );
        rItemSet.Put( SvxHorJustifyItem( SvxCellHorJustify::Center, ATTR_HOR_JUSTIFY ) );
    }

    const HTMLOptions& rOptions = static_cast<HTMLParser*>(rInfo.pParser)->GetOptions();
    HTMLOptions::const_iterator itr = rOptions.begin(), itrEnd = rOptions.end();
    for (; itr != itrEnd; ++itr)
    {
        switch( itr->GetToken() )
        {
            case HtmlOptionId::ALIGN:
            {
                SvxCellHorJustify eVal = SvxCellHorJustify::Standard;
                const OUString& rOptVal = itr->GetString();
                if( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_right ) )
                    eVal = SvxCellHorJustify::Right;
                else if( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_center ) )
                    eVal = SvxCellHorJustify::Center;
                else if( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_AL_left ) )
                    eVal = SvxCellHorJustify::Left;
                if( eVal != SvxCellHorJustify::Standard )
                    rItemSet.Put( SvxHorJustifyItem( eVal, ATTR_HOR_JUSTIFY ) );
            }
            break;

            case HtmlOptionId::VALIGN:
            {
                SvxCellVerJustify eVal = SVX_VER_JUSTIFY_STANDARD;
                const OUString& rOptVal = itr->GetString();
                if( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_VA_top ) )
                    eVal = SVX_VER_JUSTIFY_TOP;
                else if( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_VA_middle ) )
                    eVal = SVX_VER_JUSTIFY_CENTER;
                else if( rOptVal.equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_VA_bottom ) )
                    eVal = SVX_VER_JUSTIFY_BOTTOM;
                if( eVal != SVX_VER_JUSTIFY_STANDARD )
                    rItemSet.Put( SvxVerJustifyItem( eVal, ATTR_VER_JUSTIFY ) );
            }
            break;

            case HtmlOptionId::BGCOLOR:
            {
                Color aColor;
                itr->GetColor( aColor );
                rItemSet.Put( SvxBrushItem( aColor, ATTR_BACKGROUND ) );
            }
            break;
            default: break;
        }
    }
}

void ScHTMLTable::SetDocSize( ScHTMLOrient eOrient, SCCOLROW nCellPos, SCCOLROW nSize )
{
    OSL_ENSURE( nCellPos >= 0, "ScHTMLTable::SetDocSize - unexpected negative position" );
    ScSizeVec& rSizes = maCumSizes[ eOrient ];
    size_t nIndex = static_cast< size_t >( nCellPos );
    // expand with height/width == 1
    while( nIndex >= rSizes.size() )
        rSizes.push_back( rSizes.empty() ? 1 : (rSizes.back() + 1) );
    // update size of passed position and all following
    // #i109987# only grow, don't shrink - use the largest needed size
    SCCOLROW nDiff = nSize - ((nIndex == 0) ? rSizes.front() : (rSizes[ nIndex ] - rSizes[ nIndex - 1 ]));
    if( nDiff > 0 )
        for( ScSizeVec::iterator aIt = rSizes.begin() + nIndex, aEnd = rSizes.end(); aIt != aEnd; ++aIt )
            *aIt += nDiff;
}

void ScHTMLTable::CalcNeededDocSize(
        ScHTMLOrient eOrient, SCCOLROW nCellPos, SCCOLROW nCellSpan, SCCOLROW nRealDocSize )
{
    SCCOLROW nDiffSize = 0;
    // in merged columns/rows: reduce needed size by size of leading columns
    while( nCellSpan > 1 )
    {
        nDiffSize += GetDocSize( eOrient, nCellPos );
        --nCellSpan;
        ++nCellPos;
    }
    // set remaining needed size to last column/row
    nRealDocSize -= std::min< SCCOLROW >( nRealDocSize - 1, nDiffSize );
    SetDocSize( eOrient, nCellPos, nRealDocSize );
}

void ScHTMLTable::FillEmptyCells()
{
    for( ScHTMLTableIterator aIter( mxNestedTables.get() ); aIter.is(); ++aIter )
        aIter->FillEmptyCells();

    // insert the final vertically merged ranges into maUsedCells
    for ( size_t i = 0, nRanges = maVMergedCells.size(); i < nRanges; ++i )
    {
        ScRange* pRange = maVMergedCells[ i ];
        maUsedCells.Join( *pRange );
    }

    for( ScAddress aAddr; aAddr.Row() < maSize.mnRows; aAddr.IncRow() )
    {
        for( aAddr.SetCol( 0 ); aAddr.Col() < maSize.mnCols; aAddr.IncCol() )
        {
            if( !maUsedCells.Find( aAddr ) )
            {
                // create a range for the lock list (used to calc. cell span)
                ScRange aRange( aAddr );
                do
                {
                    aRange.aEnd.IncCol();
                }
                while( (aRange.aEnd.Col() < maSize.mnCols) && !maUsedCells.Find( aRange.aEnd ) );
                aRange.aEnd.IncCol( -1 );
                maUsedCells.Join( aRange );

                // insert a dummy entry
                ScHTMLEntryPtr xEntry = CreateEntry();
                ImplPushEntryToList( maEntryMap[ ScHTMLPos( aAddr ) ], xEntry );
            }
        }
    }
}

void ScHTMLTable::RecalcDocSize()
{
    // recalc table sizes recursively from inner to outer
    for( ScHTMLTableIterator aIter( mxNestedTables.get() ); aIter.is(); ++aIter )
        aIter->RecalcDocSize();

    /*  Two passes: first calculates the sizes of single columns/rows, then
        the sizes of spanned columns/rows. This allows to fill nested tables
        into merged cells optimally. */
    static const sal_uInt16 PASS_SINGLE = 0;
    static const sal_uInt16 PASS_SPANNED = 1;
    for( sal_uInt16 nPass = PASS_SINGLE; nPass <= PASS_SPANNED; ++nPass )
    {
        // iterate through every table cell
        ScHTMLEntryMap::const_iterator aMapIterEnd = maEntryMap.end();
        for( ScHTMLEntryMap::const_iterator aMapIter = maEntryMap.begin(); aMapIter != aMapIterEnd; ++aMapIter )
        {
            const ScHTMLPos& rCellPos = aMapIter->first;
            ScHTMLSize aCellSpan = GetSpan( rCellPos );

            const ScHTMLEntryList& rEntryList = aMapIter->second;
            ScHTMLEntryList::const_iterator aListIter;
            ScHTMLEntryList::const_iterator aListIterEnd = rEntryList.end();

            // process the dimension of the current cell in this pass?
            // (pass is single and span is 1) or (pass is not single and span is not 1)
            bool bProcessColWidth = ((nPass == PASS_SINGLE) == (aCellSpan.mnCols == 1));
            bool bProcessRowHeight = ((nPass == PASS_SINGLE) == (aCellSpan.mnRows == 1));
            if( bProcessColWidth || bProcessRowHeight )
            {
                ScHTMLSize aDocSize( 1, 0 );    // resulting size of the cell in document

                // expand the cell size for each cell parse entry
                for( aListIter = rEntryList.begin(); aListIter != aListIterEnd; ++aListIter )
                {
                    ScHTMLTable* pTable = GetExistingTable( (*aListIter)->GetTableId() );
                    // find entry with maximum width
                    if( bProcessColWidth && pTable )
                        aDocSize.mnCols = std::max( aDocSize.mnCols, static_cast< SCCOL >( pTable->GetDocSize( tdCol ) ) );
                    // add up height of each entry
                    if( bProcessRowHeight )
                        aDocSize.mnRows += pTable ? pTable->GetDocSize( tdRow ) : 1;
                }
                if( !aDocSize.mnRows )
                    aDocSize.mnRows = 1;

                if( bProcessColWidth )
                    CalcNeededDocSize( tdCol, rCellPos.mnCol, aCellSpan.mnCols, aDocSize.mnCols );
                if( bProcessRowHeight )
                    CalcNeededDocSize( tdRow, rCellPos.mnRow, aCellSpan.mnRows, aDocSize.mnRows );
            }
        }
    }
}

void ScHTMLTable::RecalcDocPos( const ScHTMLPos& rBasePos )
{
    maDocBasePos = rBasePos;
    // after the previous assignment it is allowed to call GetDocPos() methods

    // iterate through every table cell
    ScHTMLEntryMap::iterator aMapIterEnd = maEntryMap.end();
    for( ScHTMLEntryMap::iterator aMapIter = maEntryMap.begin(); aMapIter != aMapIterEnd; ++aMapIter )
    {
        // fixed doc position of the entire cell (first entry)
        const ScHTMLPos aCellDocPos( GetDocPos( aMapIter->first ) );
        // fixed doc size of the entire cell
        const ScHTMLSize aCellDocSize( GetDocSize( aMapIter->first ) );

        // running doc position for single entries
        ScHTMLPos aEntryDocPos( aCellDocPos );

        ScHTMLEntryList& rEntryList = aMapIter->second;
        ScHTMLEntry* pEntry = nullptr;
        ScHTMLEntryList::iterator aListIterEnd = rEntryList.end();
        for( ScHTMLEntryList::iterator aListIter = rEntryList.begin(); aListIter != aListIterEnd; ++aListIter )
        {
            pEntry = *aListIter;
            if( ScHTMLTable* pTable = GetExistingTable( pEntry->GetTableId() ) )
            {
                pTable->RecalcDocPos( aEntryDocPos );   // recalc nested table
                pEntry->nCol = SCCOL_MAX;
                pEntry->nRow = SCROW_MAX;
                SCROW nTableRows = static_cast< SCROW >( pTable->GetDocSize( tdRow ) );

                // use this entry to pad empty space right of table
                if( mpParentTable )     // ... but not in global table
                {
                    SCCOL nStartCol = aEntryDocPos.mnCol + static_cast< SCCOL >( pTable->GetDocSize( tdCol ) );
                    SCCOL nNextCol = aEntryDocPos.mnCol + aCellDocSize.mnCols;
                    if( nStartCol < nNextCol )
                    {
                        pEntry->nCol = nStartCol;
                        pEntry->nRow = aEntryDocPos.mnRow;
                        pEntry->nColOverlap = nNextCol - nStartCol;
                        pEntry->nRowOverlap = nTableRows;
                    }
                }
                aEntryDocPos.mnRow += nTableRows;
            }
            else
            {
                pEntry->nCol = aEntryDocPos.mnCol;
                pEntry->nRow = aEntryDocPos.mnRow;
                if( mpParentTable )    // do not merge in global table
                    pEntry->nColOverlap = aCellDocSize.mnCols;
                ++aEntryDocPos.mnRow;
            }
        }

        // pEntry points now to last entry.
        if( pEntry )
        {
            if( (pEntry == rEntryList.front()) && (pEntry->GetTableId() == SC_HTML_NO_TABLE) )
            {
                // pEntry is the only entry in this cell - merge rows of cell with single non-table entry.
                pEntry->nRowOverlap = aCellDocSize.mnRows;
            }
            else
            {
                // fill up incomplete entry lists
                SCROW nFirstUnusedRow = aCellDocPos.mnRow + aCellDocSize.mnRows;
                while( aEntryDocPos.mnRow < nFirstUnusedRow )
                {
                    ScHTMLEntryPtr xDummyEntry( new ScHTMLEntry( pEntry->GetItemSet() ) );
                    xDummyEntry->nCol = aEntryDocPos.mnCol;
                    xDummyEntry->nRow = aEntryDocPos.mnRow;
                    xDummyEntry->nColOverlap = aCellDocSize.mnCols;
                    ImplPushEntryToList( rEntryList, xDummyEntry );
                    ++aEntryDocPos.mnRow;
                }
            }
        }
    }
}

ScHTMLGlobalTable::ScHTMLGlobalTable(
    SfxItemPool& rPool,
    EditEngine& rEditEngine,
    std::vector< ScEEParseEntry* >& rEEParseList,
    ScHTMLTableId& rnUnusedId,
    ScHTMLParser* pParser
) :
    ScHTMLTable( rPool, rEditEngine, rEEParseList, rnUnusedId, pParser )
{
}

ScHTMLGlobalTable::~ScHTMLGlobalTable()
{
}

void ScHTMLGlobalTable::Recalc()
{
    // Fills up empty cells with a dummy entry. */
    FillEmptyCells();
    // recalc table sizes of all nested tables and this table
    RecalcDocSize();
    // recalc document positions of all entries in this table and in nested tables
    RecalcDocPos( GetDocPos() );
}

ScHTMLQueryParser::ScHTMLQueryParser( EditEngine* pEditEngine, ScDocument* pDoc ) :
    ScHTMLParser( pEditEngine, pDoc ),
    mnUnusedId( SC_HTML_GLOBAL_TABLE ),
    mbTitleOn( false )
{
    mxGlobTable.reset(
        new ScHTMLGlobalTable(*pPool, *pEdit, maList, mnUnusedId, this));
    mpCurrTable = mxGlobTable.get();
}

ScHTMLQueryParser::~ScHTMLQueryParser()
{
}

ErrCode ScHTMLQueryParser::Read( SvStream& rStrm, const OUString& rBaseURL  )
{
    SvKeyValueIteratorRef xValues;
    SvKeyValueIterator* pAttributes = nullptr;

    SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
    if( pObjSh && pObjSh->IsLoading() )
    {
        pAttributes = pObjSh->GetHeaderAttributes();
    }
    else
    {
        /*  When not loading, set up fake HTTP headers to force the SfxHTMLParser
            to use UTF8 (used when pasting from clipboard) */
        const sal_Char* pCharSet = rtl_getBestMimeCharsetFromTextEncoding( RTL_TEXTENCODING_UTF8 );
        if( pCharSet )
        {
            OUString aContentType = "text/html; charset=";
            aContentType += OUString::createFromAscii( pCharSet );

            xValues = new SvKeyValueIterator;
            xValues->Append( SvKeyValue( OOO_STRING_SVTOOLS_HTML_META_content_type, aContentType ) );
            pAttributes = xValues.get();
        }
    }

    Link<HtmlImportInfo&,void> aOldLink = pEdit->GetHtmlImportHdl();
    pEdit->SetHtmlImportHdl( LINK( this, ScHTMLQueryParser, HTMLImportHdl ) );
    ErrCode nErr = pEdit->Read( rStrm, rBaseURL, EE_FORMAT_HTML, pAttributes );
    pEdit->SetHtmlImportHdl( aOldLink );

    mxGlobTable->Recalc();
    nColMax = static_cast< SCCOL >( mxGlobTable->GetDocSize( tdCol ) - 1 );
    nRowMax = static_cast< SCROW >( mxGlobTable->GetDocSize( tdRow ) - 1 );

    return nErr;
}

const ScHTMLTable* ScHTMLQueryParser::GetGlobalTable() const
{
    return mxGlobTable.get();
}

void ScHTMLQueryParser::ProcessToken( const HtmlImportInfo& rInfo )
{
    switch( rInfo.nToken )
    {
// --- meta data ---
        case HtmlTokenId::META:             MetaOn( rInfo );                break;  // <meta>

// --- title handling ---
        case HtmlTokenId::TITLE_ON:         TitleOn( rInfo );               break;  // <title>
        case HtmlTokenId::TITLE_OFF:        TitleOff( rInfo );              break;  // </title>

        case HtmlTokenId::STYLE_ON:                                         break;
        case HtmlTokenId::STYLE_OFF:        ParseStyle(rInfo.aText);        break;

// --- body handling ---
        case HtmlTokenId::BODY_ON:          mpCurrTable->BodyOn( rInfo );   break;  // <body>
        case HtmlTokenId::BODY_OFF:         mpCurrTable->BodyOff( rInfo );  break;  // </body>

// --- insert text ---
        case HtmlTokenId::TEXTTOKEN:        InsertText( rInfo );            break;  // any text
        case HtmlTokenId::LINEBREAK:        mpCurrTable->BreakOn();         break;  // <br>
        case HtmlTokenId::HEAD1_ON:                                                 // <h1>
        case HtmlTokenId::HEAD2_ON:                                                 // <h2>
        case HtmlTokenId::HEAD3_ON:                                                 // <h3>
        case HtmlTokenId::HEAD4_ON:                                                 // <h4>
        case HtmlTokenId::HEAD5_ON:                                                 // <h5>
        case HtmlTokenId::HEAD6_ON:                                                 // <h6>
        case HtmlTokenId::PARABREAK_ON:     mpCurrTable->HeadingOn();       break;  // <p>

// --- misc. contents ---
        case HtmlTokenId::ANCHOR_ON:        mpCurrTable->AnchorOn();        break;  // <a>

// --- table handling ---
        case HtmlTokenId::TABLE_ON:         TableOn( rInfo );               break;  // <table>
        case HtmlTokenId::TABLE_OFF:        TableOff( rInfo );              break;  // </table>
        case HtmlTokenId::TABLEROW_ON:      mpCurrTable->RowOn( rInfo );    break;  // <tr>
        case HtmlTokenId::TABLEROW_OFF:     mpCurrTable->RowOff( rInfo );   break;  // </tr>
        case HtmlTokenId::TABLEHEADER_ON:                                           // <th>
        case HtmlTokenId::TABLEDATA_ON:     mpCurrTable->DataOn( rInfo );   break;  // <td>
        case HtmlTokenId::TABLEHEADER_OFF:                                          // </th>
        case HtmlTokenId::TABLEDATA_OFF:    mpCurrTable->DataOff( rInfo );  break;  // </td>
        case HtmlTokenId::PREFORMTXT_ON:    PreOn( rInfo );                 break;  // <pre>
        case HtmlTokenId::PREFORMTXT_OFF:   PreOff( rInfo );                break;  // </pre>

// --- formatting ---
        case HtmlTokenId::FONT_ON:          FontOn( rInfo );                break;  // <font>

        case HtmlTokenId::BIGPRINT_ON:      // <big>
            //! TODO: store current font size, use following size
            mpCurrTable->PutItem( SvxFontHeightItem( maFontHeights[ 3 ], 100, ATTR_FONT_HEIGHT ) );
        break;
        case HtmlTokenId::SMALLPRINT_ON:    // <small>
            //! TODO: store current font size, use preceding size
            mpCurrTable->PutItem( SvxFontHeightItem( maFontHeights[ 0 ], 100, ATTR_FONT_HEIGHT ) );
        break;

        case HtmlTokenId::BOLD_ON:          // <b>
        case HtmlTokenId::STRONG_ON:        // <strong>
            mpCurrTable->PutItem( SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT ) );
        break;

        case HtmlTokenId::ITALIC_ON:        // <i>
        case HtmlTokenId::EMPHASIS_ON:      // <em>
        case HtmlTokenId::ADDRESS_ON:       // <address>
        case HtmlTokenId::BLOCKQUOTE_ON:    // <blockquote>
        case HtmlTokenId::BLOCKQUOTE30_ON:  // <bq>
        case HtmlTokenId::CITIATION_ON:     // <cite>
        case HtmlTokenId::VARIABLE_ON:      // <var>
            mpCurrTable->PutItem( SvxPostureItem( ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
        break;

        case HtmlTokenId::DEFINSTANCE_ON:   // <dfn>
            mpCurrTable->PutItem( SvxWeightItem( WEIGHT_BOLD, ATTR_FONT_WEIGHT ) );
            mpCurrTable->PutItem( SvxPostureItem( ITALIC_NORMAL, ATTR_FONT_POSTURE ) );
        break;

        case HtmlTokenId::UNDERLINE_ON:     // <u>
            mpCurrTable->PutItem( SvxUnderlineItem( LINESTYLE_SINGLE, ATTR_FONT_UNDERLINE ) );
        break;
        default: break;
    }
}

void ScHTMLQueryParser::InsertText( const HtmlImportInfo& rInfo )
{
    mpCurrTable->PutText( rInfo );
    if( mbTitleOn )
        maTitle.append(rInfo.aText);
}

void ScHTMLQueryParser::FontOn( const HtmlImportInfo& rInfo )
{
    const HTMLOptions& rOptions = static_cast<HTMLParser*>(rInfo.pParser)->GetOptions();
    HTMLOptions::const_iterator itr = rOptions.begin(), itrEnd = rOptions.end();
    for (; itr != itrEnd; ++itr)
    {
        switch( itr->GetToken() )
        {
            case HtmlOptionId::FACE :
            {
                const OUString& rFace = itr->GetString();
                OUString aFontName;
                sal_Int32 nPos = 0;
                while( nPos != -1 )
                {
                    // font list separator: VCL = ';' HTML = ','
                    OUString aFName = comphelper::string::strip(rFace.getToken(0, ',', nPos), ' ');
                    aFontName = ScGlobal::addToken(aFontName, aFName, ';');
                }
                if ( !aFontName.isEmpty() )
                    mpCurrTable->PutItem( SvxFontItem( FAMILY_DONTKNOW,
                        aFontName, EMPTY_OUSTRING, PITCH_DONTKNOW,
                        RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ) );
            }
            break;
            case HtmlOptionId::SIZE :
            {
                sal_uInt32 nSize = getLimitedValue< sal_uInt32 >( itr->GetNumber(), 1, SC_HTML_FONTSIZES );
                mpCurrTable->PutItem( SvxFontHeightItem( maFontHeights[ nSize - 1 ], 100, ATTR_FONT_HEIGHT ) );
            }
            break;
            case HtmlOptionId::COLOR :
            {
                Color aColor;
                itr->GetColor( aColor );
                mpCurrTable->PutItem( SvxColorItem( aColor, ATTR_FONT_COLOR ) );
            }
            break;
            default: break;
        }
    }
}

void ScHTMLQueryParser::MetaOn( const HtmlImportInfo& rInfo )
{
    if( mpDoc->GetDocumentShell() )
    {
        HTMLParser* pParser = static_cast< HTMLParser* >( rInfo.pParser );

        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
            mpDoc->GetDocumentShell()->GetModel(), uno::UNO_QUERY_THROW);
        pParser->ParseMetaOptions(
            xDPS->getDocumentProperties(),
            mpDoc->GetDocumentShell()->GetHeaderAttributes() );
    }
}

void ScHTMLQueryParser::TitleOn( const HtmlImportInfo& /*rInfo*/ )
{
    mbTitleOn = true;
    maTitle.setLength(0);
}

void ScHTMLQueryParser::TitleOff( const HtmlImportInfo& rInfo )
{
    if( mbTitleOn )
    {
        OUString aTitle = maTitle.makeStringAndClear().trim();
        if (!aTitle.isEmpty() && mpDoc->GetDocumentShell())
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                mpDoc->GetDocumentShell()->GetModel(), uno::UNO_QUERY_THROW);

            xDPS->getDocumentProperties()->setTitle(aTitle);
        }
        InsertText( rInfo );
        mbTitleOn = false;
    }
}

void ScHTMLQueryParser::TableOn( const HtmlImportInfo& rInfo )
{
    mpCurrTable = mpCurrTable->TableOn( rInfo );
}

void ScHTMLQueryParser::TableOff( const HtmlImportInfo& rInfo )
{
    mpCurrTable = mpCurrTable->TableOff( rInfo );
}

void ScHTMLQueryParser::PreOn( const HtmlImportInfo& rInfo )
{
    mpCurrTable = mpCurrTable->PreOn( rInfo );
}

void ScHTMLQueryParser::PreOff( const HtmlImportInfo& rInfo )
{
    mpCurrTable = mpCurrTable->PreOff( rInfo );
}

void ScHTMLQueryParser::CloseTable( const HtmlImportInfo& rInfo )
{
    mpCurrTable = mpCurrTable->CloseTable( rInfo );
}

namespace {

/**
 * Handler class for the CSS parser.
 */
class CSSHandler
{
    struct MemStr
    {
        const char* mp;
        size_t      mn;

        MemStr() : mp(nullptr), mn(0) {}
        MemStr(const char* p, size_t n) : mp(p), mn(n) {}
        MemStr& operator=(const MemStr& r)
        {
            mp = r.mp;
            mn = r.mn;
            return *this;
        }
    };

    typedef std::pair<MemStr, MemStr> SelectorName; // element : class
    typedef std::vector<SelectorName> SelectorNames;
    SelectorNames maSelectorNames; /// current selector names.
    MemStr maPropName;  /// current property name.
    MemStr maPropValue; /// current property value.

    ScHTMLStyles& mrStyles;
public:
    explicit CSSHandler(ScHTMLStyles& rStyles) : mrStyles(rStyles) {}

    static void at_rule_name(const char* /*p*/, size_t /*n*/)
    {
        // TODO: For now, we ignore at-rule properties.
    }

    void property_name(const char* p, size_t n)
    {
        maPropName = MemStr(p, n);
    }

    void value(const char* p, size_t n)
    {
        maPropValue = MemStr(p, n);
    }

    static void begin_parse() {}

    static void end_parse() {}

    static void begin_block() {}

    void end_block()
    {
        maSelectorNames.clear();
    }

    static void begin_property() {}

    void end_property()
    {
        SelectorNames::const_iterator itr = maSelectorNames.begin(), itrEnd = maSelectorNames.end();
        for (; itr != itrEnd; ++itr)
        {
            // Add this property to the collection for each selector.
            const SelectorName& rSelName = *itr;
            const MemStr& rElem = rSelName.first;
            const MemStr& rClass = rSelName.second;
            OUString aName(maPropName.mp, maPropName.mn, RTL_TEXTENCODING_UTF8);
            OUString aValue(maPropValue.mp, maPropValue.mn, RTL_TEXTENCODING_UTF8);
            mrStyles.add(rElem.mp, rElem.mn, rClass.mp, rClass.mn, aName, aValue);
        }
        maPropName = MemStr();
        maPropValue = MemStr();
    }

    // new members
    static void simple_selector_type(const char* /*p*/, size_t /*n*/) {}

    static void simple_selector_class(const char* /*p*/, size_t /*n*/) {}

    static void simple_selector_pseudo_element(orcus::css::pseudo_element_t /*pe*/) {}

    static void simple_selector_pseudo_class(orcus::css::pseudo_class_t /*pc*/) {}

    static void simple_selector_id(const char* /*p*/, size_t /*n*/) {}

    static void end_simple_selector() {}

    static void end_selector() {}

    static void combinator(orcus::css::combinator_t /*combinator*/) {}

    static void rgb(uint8_t /*red*/ , uint8_t /*green*/ , uint8_t /*blue*/ ) {}

    static void rgba(uint8_t /*red*/ , uint8_t /*green*/ , uint8_t /*blue*/ , double /*alpha*/ ) {}

    static void hsl(uint8_t /*hue*/ , uint8_t /*sat*/ , uint8_t /*light*/ ) {}

    static void hsla(uint8_t /*hue*/ , uint8_t /*sat*/ , uint8_t /*light*/ , double /*alpha*/ ) {}

    static void url(const char* /*p*/, size_t /*n*/) {}

};

}

void ScHTMLQueryParser::ParseStyle(const OUString& rStrm)
{
    OString aStr = OUStringToOString(rStrm, RTL_TEXTENCODING_UTF8);
    CSSHandler aHdl(GetStyles());
    orcus::css_parser<CSSHandler> aParser(aStr.getStr(), aStr.getLength(), aHdl);
    try
    {
        aParser.parse();
    }
    catch (const orcus::css::parse_error&)
    {
        // TODO: Parsing of CSS failed.  Do nothing for now.
    }
}

IMPL_LINK( ScHTMLQueryParser, HTMLImportHdl, HtmlImportInfo&, rInfo, void )
{
    switch( rInfo.eState )
    {
        case HtmlImportState::Start:
        break;

        case HtmlImportState::NextToken:
            ProcessToken( rInfo );
        break;

        case HtmlImportState::InsertPara:
            mpCurrTable->InsertPara( rInfo );
        break;

        case HtmlImportState::SetAttr:
        case HtmlImportState::InsertText:
        case HtmlImportState::InsertField:
        break;

        case HtmlImportState::End:
            while( mpCurrTable->GetTableId() != SC_HTML_GLOBAL_TABLE )
                CloseTable( rInfo );
        break;

        default:
            OSL_FAIL( "ScHTMLQueryParser::HTMLImportHdl - unknown ImportInfo::eState" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

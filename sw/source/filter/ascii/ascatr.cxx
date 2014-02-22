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

#include <hintids.hxx>
#include <tools/stream.hxx>
#include <comphelper/string.hxx>
#include <editeng/fontitem.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <wrtasc.hxx>
#include <txatbase.hxx>
#include <fchrfmt.hxx>
#include <txtfld.hxx>
#include <txtatr.hxx>
#include <fmtftn.hxx>
#include <charfmt.hxx>
#include <fmtfld.hxx>
#include <fldbas.hxx>
#include <ftninfo.hxx>

#include <algorithm>

/*
 * This file contains all output functions of the ASCII-Writer;
 * For all nodes, attributes, formats and chars.
 */

class SwASC_AttrIter
{
    SwASCWriter& rWrt;
    const SwTxtNode& rNd;
    sal_Int32 nAktSwPos;

    sal_Int32 SearchNext( sal_Int32 nStartPos );

public:
    SwASC_AttrIter( SwASCWriter& rWrt, const SwTxtNode& rNd, sal_Int32 nStt );

    void NextPos()
    {
        nAktSwPos = SearchNext( nAktSwPos + 1 );
    }

    sal_Int32 WhereNext() const
    {
        return nAktSwPos;
    }

    bool OutAttr( sal_Int32 nSwPos );
};


SwASC_AttrIter::SwASC_AttrIter(
    SwASCWriter& rWr,
    const SwTxtNode& rTxtNd,
    sal_Int32 nStt )
    : rWrt( rWr )
    , rNd( rTxtNd )
    , nAktSwPos( 0 )
{
    nAktSwPos = SearchNext( nStt + 1 );
}


sal_Int32 SwASC_AttrIter::SearchNext( sal_Int32 nStartPos )
{
    sal_Int32 nMinPos = SAL_MAX_INT32;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        // TODO: This can be optimized, if we make use of the fact that the TxtAttrs
        // are sorted by starting position. We would need to remember two indices, however.
        for ( sal_uInt16 i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if ( pHt->HasDummyChar() )
            {
                sal_Int32 nPos = *pHt->GetStart();

                if( nPos >= nStartPos && nPos <= nMinPos )
                    nMinPos = nPos;

                if( ( ++nPos ) >= nStartPos && nPos < nMinPos )
                    nMinPos = nPos;
            }
            else if ( pHt->HasContent() )
            {
                const sal_Int32 nHintStart = *pHt->GetStart();
                if ( nHintStart >= nStartPos && nHintStart <= nMinPos )
                {
                    nMinPos = nHintStart;
                }

                const sal_Int32 nHintEnd = pHt->End() ? *pHt->End() : COMPLETE_STRING;
                if ( nHintEnd >= nStartPos && nHintEnd < nMinPos )
                {
                    nMinPos = nHintEnd;
                }
            }
        }
    }
    return nMinPos;
}


bool SwASC_AttrIter::OutAttr( sal_Int32 nSwPos )
{
    bool bRet = false;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        sal_uInt16 i;
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if ( ( pHt->HasDummyChar()
                   || pHt->HasContent() )
                 && nSwPos == *pHt->GetStart() )
            {
                bRet = true;
                OUString sOut;
                switch( pHt->Which() )
                {
                case RES_TXTATR_FIELD:
                case RES_TXTATR_ANNOTATION:
                case RES_TXTATR_INPUTFIELD:
                    sOut = static_cast<SwTxtFld const*>(pHt)->GetFmtFld().GetField()->ExpandField(true);
                    break;

                case RES_TXTATR_FTN:
                    {
                        const SwFmtFtn& rFtn = pHt->GetFtn();
                        if( !rFtn.GetNumStr().isEmpty() )
                            sOut = rFtn.GetNumStr();
                        else if( rFtn.IsEndNote() )
                            sOut = rWrt.pDoc->GetEndNoteInfo().aFmt.
                            GetNumStr( rFtn.GetNumber() );
                        else
                            sOut = rWrt.pDoc->GetFtnInfo().aFmt.
                            GetNumStr( rFtn.GetNumber() );
                    }
                    break;
                }
                if( !sOut.isEmpty() )
                    rWrt.Strm().WriteUnicodeOrByteText( sOut );
            }
            else if( nSwPos < *pHt->GetStart() )
                break;
        }
    }
    return bRet;
}



// Output of the node


static Writer& OutASC_SwTxtNode( Writer& rWrt, SwCntntNode& rNode )
{
    const SwTxtNode& rNd = (SwTxtNode&)rNode;

    sal_Int32 nStrPos = rWrt.pCurPam->GetPoint()->nContent.GetIndex();
    const sal_Int32 nNodeEnd = rNd.Len();
    sal_Int32 nEnd = nNodeEnd;
    bool bLastNd =  rWrt.pCurPam->GetPoint()->nNode == rWrt.pCurPam->GetMark()->nNode;
    if( bLastNd )
        nEnd = rWrt.pCurPam->GetMark()->nContent.GetIndex();

    SwASC_AttrIter aAttrIter( (SwASCWriter&)rWrt, rNd, nStrPos );

    if( !nStrPos && rWrt.bExportPargraphNumbering )
    {
        OUString numString( rNd.GetNumString() );
        if (!numString.isEmpty())
        {
            numString += " ";
            rWrt.Strm().WriteUnicodeOrByteText(numString);
        }
    }

    OUString aStr( rNd.GetTxt() );
    if( rWrt.bASCII_ParaAsBlanc )
        aStr = aStr.replace(0x0A, ' ');

    const bool bExportSoftHyphens = RTL_TEXTENCODING_UCS2 == rWrt.GetAsciiOptions().GetCharSet() ||
                                    RTL_TEXTENCODING_UTF8 == rWrt.GetAsciiOptions().GetCharSet();

    do {
        const sal_Int32 nNextAttr = std::min(aAttrIter.WhereNext(), nEnd);

        if( !aAttrIter.OutAttr( nStrPos ))
        {
            OUString aOutStr( aStr.copy( nStrPos, nNextAttr - nStrPos ) );
            if ( !bExportSoftHyphens )
                aOutStr = comphelper::string::remove(aOutStr, CHAR_SOFTHYPHEN);

            rWrt.Strm().WriteUnicodeOrByteText( aOutStr );
        }
        nStrPos = nNextAttr;
        aAttrIter.NextPos();
    } while( nStrPos < nEnd );

    if( !bLastNd ||
        ( ( !rWrt.bWriteClipboardDoc && !rWrt.bASCII_NoLastLineEnd )
            && !nStrPos && nEnd == nNodeEnd ) )
        rWrt.Strm().WriteUnicodeOrByteText( ((SwASCWriter&)rWrt).GetLineEnd());

    return rWrt;
}

/*
 * Create the table for the ASCII function pointers to the output
 * functon.
 * There are local structures that only need to be known to the ASCII DLL.
 */

SwNodeFnTab aASCNodeFnTab = {
/* RES_TXTNODE  */                   OutASC_SwTxtNode,
/* RES_GRFNODE  */                   0,
/* RES_OLENODE  */                   0
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

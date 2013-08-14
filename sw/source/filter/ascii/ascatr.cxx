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

/*
 * This file contains all output functions of the ASCII-Writer;
 * For all nodes, attributes, formats and chars.
 */

class SwASC_AttrIter
{
    SwASCWriter& rWrt;
    const SwTxtNode& rNd;
    xub_StrLen nAktSwPos;

    xub_StrLen SearchNext( xub_StrLen nStartPos );

public:
    SwASC_AttrIter( SwASCWriter& rWrt, const SwTxtNode& rNd, xub_StrLen nStt );

    void NextPos()      { nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    xub_StrLen WhereNext() const        { return nAktSwPos; }
    bool OutAttr( xub_StrLen nSwPos );
};


SwASC_AttrIter::SwASC_AttrIter( SwASCWriter& rWr, const SwTxtNode& rTxtNd,
                                xub_StrLen nStt )
    : rWrt( rWr ), rNd( rTxtNd ), nAktSwPos( 0 )
{
    nAktSwPos = SearchNext( nStt + 1 );
}


xub_StrLen SwASC_AttrIter::SearchNext( xub_StrLen nStartPos )
{
    xub_StrLen nMinPos = STRING_MAXLEN;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
// TODO: This can be optimized, if we make use of the fact that the TxtAttrs
// are sorted by starting position. We would need to remember two indices, however.
        for ( sal_uInt16 i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if (pHt->HasDummyChar())
            {
                xub_StrLen nPos = *pHt->GetStart();

                if( nPos >= nStartPos && nPos <= nMinPos )
                    nMinPos = nPos;

                if( ( ++nPos ) >= nStartPos && nPos < nMinPos )
                    nMinPos = nPos;
            }
        }
    }
    return nMinPos;
}


bool SwASC_AttrIter::OutAttr( xub_StrLen nSwPos )
{
    bool bRet = false;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    if( pTxtAttrs )
    {
        sal_uInt16 i;
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if ( pHt->HasDummyChar() && nSwPos == *pHt->GetStart() )
            {
                bRet = true;
                String sOut;
                switch( pHt->Which() )
                {
                case RES_TXTATR_FIELD:
                    sOut = static_cast<SwTxtFld const*>(pHt)->GetFld().GetFld()
                            ->ExpandField(true);
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
                if( sOut.Len() )
                    rWrt.Strm().WriteUnicodeOrByteText( sOut );
            }
            else if( nSwPos < *pHt->GetStart() )
                break;
        }
    }
    return bRet;
}


//------------------------
// Output of the node
//------------------------

static Writer& OutASC_SwTxtNode( Writer& rWrt, SwCntntNode& rNode )
{
    const SwTxtNode& rNd = (SwTxtNode&)rNode;

    xub_StrLen nStrPos = rWrt.pCurPam->GetPoint()->nContent.GetIndex();
    xub_StrLen nNodeEnde = rNd.Len(), nEnde = nNodeEnde;
    bool bLastNd =  rWrt.pCurPam->GetPoint()->nNode == rWrt.pCurPam->GetMark()->nNode;
    if( bLastNd )
        nEnde = rWrt.pCurPam->GetMark()->nContent.GetIndex();

    SwASC_AttrIter aAttrIter( (SwASCWriter&)rWrt, rNd, nStrPos );

    if( !nStrPos && rWrt.bExportPargraphNumbering )
    {
        String numString( rNd.GetNumString() );
        if (numString.Len())
        {
            numString.Append(' ');
            rWrt.Strm().WriteUnicodeOrByteText(numString);
        }
    }

    String aStr( rNd.GetTxt() );
    if( rWrt.bASCII_ParaAsBlanc )
        aStr.SearchAndReplaceAll( 0x0A, ' ' );

    const bool bExportSoftHyphens = RTL_TEXTENCODING_UCS2 == rWrt.GetAsciiOptions().GetCharSet() ||
                                    RTL_TEXTENCODING_UTF8 == rWrt.GetAsciiOptions().GetCharSet();

    do {
        xub_StrLen nNextAttr = aAttrIter.WhereNext();

        if( nNextAttr > nEnde )
            nNextAttr = nEnde;

        if( !aAttrIter.OutAttr( nStrPos ))
        {
            String aOutStr( aStr.Copy( nStrPos, nNextAttr - nStrPos ) );
            if ( !bExportSoftHyphens )
                aOutStr = comphelper::string::remove(aOutStr, CHAR_SOFTHYPHEN);

            rWrt.Strm().WriteUnicodeOrByteText( aOutStr );
        }
        nStrPos = nNextAttr;
        aAttrIter.NextPos();
    } while( nStrPos < nEnde );

    if( !bLastNd ||
        ( ( !rWrt.bWriteClipboardDoc && !rWrt.bASCII_NoLastLineEnd )
            && !nStrPos && nEnde == nNodeEnde ) )
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

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


#include <ctype.h>
#include <hintids.hxx>

#include <sal/macros.h>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/string.hxx>
#include <vcl/graph.hxx>
#include <svl/urihelper.hxx>
#include <svtools/rtftoken.h>
#include <svl/zforlist.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/brkitem.hxx>
#include <fmtfld.hxx>
#include <fmtinfmt.hxx>
#include <swtypes.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <fldbas.hxx>
#include <swparrtf.hxx>
#include <txatbase.hxx>
#include <dbfld.hxx>
#include <usrfld.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <charfmt.hxx>
#include <fmtruby.hxx>
#include <breakit.hxx>
#include <reffld.hxx>
#include <SwStyleNameMapper.hxx>
#include <editeng/charhiddenitem.hxx>


// bestimme, ob es sich um ein IMPORT/TOC - Feld handelt.
// return:  0 - weder noch,
//          1 - TOC
//          2 - IMPORT
//          3 - INDEX
enum RTF_FLD_TYPES {
    RTFFLD_UNKNOWN = 0,
    RTFFLD_TOC,
    RTFFLD_IMPORT,
    RTFFLD_INDEX,
    RTFFLD_SYMBOL,
    RTFFLD_PAGE,
    RTFFLD_NUMPAGES,
    RTFFLD_DATE,
    RTFFLD_TIME,
    RTFFLD_DATA,
    RTFFLD_MERGEFLD,
    RTFFLD_HYPERLINK,
    RTFFLD_REF,
    RTFFLD_PAGEREF,
    RTFFLD_EQ,
    RTFFLD_INCLUDETEXT
};

class RtfFieldSwitch
{
    String sParam;
    xub_StrLen nCurPos;
public:
    RtfFieldSwitch( const String& rParam );
    sal_Unicode GetSwitch( String& rParam );

    sal_Bool IsAtEnd() const                { return nCurPos >= sParam.Len(); }
    xub_StrLen GetCurPos() const        { return nCurPos; }
    void Erase( xub_StrLen nEndPos )    { sParam.Erase( 0, nEndPos ); }
    void Insert( const String& rIns )   { sParam.Insert( rIns, 0 ); }
    const String& GetStr() const        { return sParam; }
};

RtfFieldSwitch::RtfFieldSwitch( const String& rParam )
    : nCurPos( 0  )
{
    sParam = comphelper::string::strip(rParam, ' ');
}

sal_Unicode RtfFieldSwitch::GetSwitch( String& rParam )
{
    // beginnt ein Schalter?
    sal_Unicode c, cKey = 0;
    if( '\\' == (c = sParam.GetChar( nCurPos )) )
    {
        if( '\\' == ( c = sParam.GetChar( ++nCurPos )) )
            c = sParam.GetChar( ++nCurPos );

        cKey = c;

        while( ++nCurPos < sParam.Len() &&
                ' ' == ( c = sParam.GetChar( nCurPos )) )
            ;
    }

    // dann alles in Hochkommatas oder bis zum naechsten // als
    // Param returnen
    sal_uInt16 nOffset;
    if( '"' != c && '\'' != c )
        c = '\\', nOffset = 0;
    else
        nOffset = 1;

    sParam.Erase( 0, nCurPos + nOffset );
    rParam = sParam.GetToken( 0, c );
    sParam = comphelper::string::stripStart(sParam.Erase(0, rParam.Len() + nOffset), ' ');
    if( '\\' == c )
        rParam = comphelper::string::stripEnd(rParam, ' ');
    nCurPos = 0;

    return cKey;
}

struct RTF_EquationData
{
    String sFontName, sUp, sDown, sText;
    sal_Int32 nJustificationCode, nFontSize, nUp, nDown, nStyleNo;

    inline RTF_EquationData()
        : nJustificationCode(0), nFontSize(0), nUp(0), nDown(0),
        nStyleNo( -1 )
    {}
};

xub_StrLen lcl_FindEndBracket( const String& rStr )
{
    xub_StrLen nEnd = rStr.Len(), nRet = STRING_NOTFOUND, nPos = 0;
    int nOpenCnt = 1;
    sal_Unicode cCh;
    for( ; nPos < nEnd; ++nPos )
        if( ')' == (cCh = rStr.GetChar( nPos )) && !--nOpenCnt )
        {
            nRet = nPos;
            break;
        }
        else if( '(' == cCh )
            ++nOpenCnt;

    return nRet;
}

static void lcl_ScanEquationField( const String& rStr, RTF_EquationData& rData,
                            sal_Unicode nSttKey )
{
    int nSubSupFlag(0);
    RtfFieldSwitch aRFS( rStr );
    while( !aRFS.IsAtEnd() )
    {
        String sParam;
        sal_Unicode cKey = aRFS.GetSwitch( sParam );
        if( 1 == nSubSupFlag )
            ++nSubSupFlag;
        else if( 1 < nSubSupFlag )
            nSubSupFlag = 0;

        sal_Bool bCheckBracket = sal_False;
        switch( cKey )
        {
        case 0:
            switch( nSttKey )
            {
            case 'u':   rData.sUp += sParam;    break;
            case 'd':   rData.sDown += sParam;  break;
            default:    rData.sText += sParam;  break;
            }
            break;

        case '*':
            if( sParam.Len() )
            {
                if( sParam.EqualsIgnoreCaseAscii( "jc", 0, 2 ) )
                    rData.nJustificationCode = sParam.Copy( 2 ).ToInt32();
                else if( sParam.EqualsIgnoreCaseAscii( "hps", 0, 3 ) )
                    rData.nFontSize= sParam.Copy( 3 ).ToInt32();
                else if( sParam.EqualsIgnoreCaseAscii( "Font:", 0, 5 ) )
                    rData.sFontName = sParam.Copy( 5 );
                else if( sParam.EqualsIgnoreCaseAscii( "cs", 0, 2 ) )
                    rData.nStyleNo = sParam.Copy( 2 ).ToInt32();
            }
            break;
        case 's' :
            ++nSubSupFlag;
            break;

        case 'u':
            if( sParam.Len() && 'p' == sParam.GetChar( 0 ) &&
                2 == nSubSupFlag )
            {
                rData.nUp = sParam.Copy( 1 ).ToInt32();
                bCheckBracket = sal_True;
            }
            break;

        case 'd':
            if( sParam.Len() && 'o' == sParam.GetChar( 0 ) &&
                2 == nSubSupFlag )
            {
                rData.nDown = sParam.Copy( 1 ).ToInt32();
                bCheckBracket = sal_True;
            }
            break;

        default:
            bCheckBracket = sal_True;
            cKey = 0;
            break;
        }

        if( bCheckBracket && sParam.Len() )
        {
            xub_StrLen nEnd, nStt = sParam.Search( '(' ),
                        nLen = sParam.Len();
            if( STRING_NOTFOUND != nStt )
            {
                sParam.Erase( 0, nStt + 1 ) += aRFS.GetStr();
                if( STRING_NOTFOUND !=
                        (nEnd = ::lcl_FindEndBracket( sParam )) )
                {
                    // end in the added string?
                    if( (nLen - nStt - 1 ) < nEnd )
                        aRFS.Erase( nEnd + 1 - (nLen - nStt - 1));
                    else
                    {
                        // not all handled here, so set new into the RFS
                        aRFS.Insert( sParam.Copy( nEnd + 1,
                                                nLen - nStt - nEnd - 2 ));
                        sal_Unicode cCh;
                        if( aRFS.GetStr().Len() &&
                            ( ',' == (cCh = aRFS.GetStr().GetChar(0)) ||
                              ';' == cCh ))
                            aRFS.Erase( 1 );
                    }

                    ::lcl_ScanEquationField( sParam.Copy( 0, nEnd ),
                                                rData, cKey );
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

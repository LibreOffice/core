/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbgoutsw.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:40:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef DEBUG

#include <tools/string.hxx>
#include <svtools/poolitem.hxx>
#include <svtools/itemiter.hxx>
#include <string>
#include <map>
#include <node.hxx>
#include <ndtxt.hxx>
#include <ndhints.hxx>
#include <txatbase.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <undobj.hxx>
#include <doc.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <swrect.hxx>
#include <ndarr.hxx>
#include <paratr.hxx>
#include <dbgoutsw.hxx>
#include <SwRewriter.hxx>

using namespace std;

static ByteString aDbgOutResult;
bool bDbgOutStdErr = false;
bool bDbgOutPrintAttrSet = false;

char* db_pretty_print(const String* str, int flags, char* fmt)
{
    (void) fmt;
    (void) flags;
    return const_cast<char*>(dbg_out(*str));
}

template<class T>
String lcl_dbg_out_SvPtrArr(const T & rArr)
{
    String aStr("[ ", RTL_TEXTENCODING_ASCII_US);

    for (sal_Int16 n = 0; n < rArr.Count(); n++)
    {
        if (n > 0)
            aStr += String(", ", RTL_TEXTENCODING_ASCII_US);

        if (rArr[n])
            aStr += lcl_dbg_out(*rArr[n]);
        else
            aStr += String("(null)", RTL_TEXTENCODING_ASCII_US);
    }

    aStr += String(" ]", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const void * pVoid)
{
    char sBuffer[1024];

    sprintf(sBuffer, "%p", pVoid);

    String aTmpStr(sBuffer, RTL_TEXTENCODING_ASCII_US);

    return dbg_out(aTmpStr);
}

const char * dbg_out(const String & aStr)
{
    aDbgOutResult = ByteString(aStr, RTL_TEXTENCODING_ASCII_US);

    if (bDbgOutStdErr)
        fprintf(stderr, "%s", aDbgOutResult.GetBuffer());

    return aDbgOutResult.GetBuffer();
}


struct CompareUShort
{
    bool operator()(USHORT a, USHORT b) const
    {
        return a < b;
    }
};

map<USHORT,String,CompareUShort> & GetItemWhichMap()
{
    static map<USHORT,String,CompareUShort> aItemWhichMap;
    static bool bInitialized = false;

    if (! bInitialized)
    {
        aItemWhichMap[RES_CHRATR_CASEMAP] = String("CHRATR_CASEMAP", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CHARSETCOLOR] = String("CHRATR_CHARSETCOLOR", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_COLOR] = String("CHRATR_COLOR", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CONTOUR] = String("CHRATR_CONTOUR", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CROSSEDOUT] = String("CHRATR_CROSSEDOUT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_ESCAPEMENT] = String("CHRATR_ESCAPEMENT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_FONT] = String("CHRATR_FONT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_FONTSIZE] = String("CHRATR_FONTSIZE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_KERNING] = String("CHRATR_KERNING", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_LANGUAGE] = String("CHRATR_LANGUAGE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_POSTURE] = String("CHRATR_POSTURE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_PROPORTIONALFONTSIZE] = String("CHRATR_PROPORTIONALFONTSIZE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_SHADOWED] = String("CHRATR_SHADOWED", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_UNDERLINE] = String("CHRATR_UNDERLINE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_WEIGHT] = String("CHRATR_WEIGHT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_WORDLINEMODE] = String("CHRATR_WORDLINEMODE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_AUTOKERN] = String("CHRATR_AUTOKERN", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_BLINK] = String("CHRATR_BLINK", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_NOHYPHEN] = String("CHRATR_NOHYPHEN", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_NOLINEBREAK] = String("CHRATR_NOLINEBREAK", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_BACKGROUND] = String("CHRATR_BACKGROUND", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CJK_FONT] = String("CHRATR_CJK_FONT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CJK_FONTSIZE] = String("CHRATR_CJK_FONTSIZE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CJK_LANGUAGE] = String("CHRATR_CJK_LANGUAGE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CJK_POSTURE] = String("CHRATR_CJK_POSTURE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CJK_WEIGHT] = String("CHRATR_CJK_WEIGHT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CTL_FONT] = String("CHRATR_CTL_FONT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CTL_FONTSIZE] = String("CHRATR_CTL_FONTSIZE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CTL_LANGUAGE] = String("CHRATR_CTL_LANGUAGE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CTL_POSTURE] = String("CHRATR_CTL_POSTURE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_CTL_WEIGHT] = String("CHRATR_CTL_WEIGHT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_ROTATE] = String("CHRATR_ROTATE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_EMPHASIS_MARK] = String("CHRATR_EMPHASIS_MARK", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_TWO_LINES] = String("CHRATR_TWO_LINES", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_SCALEW] = String("CHRATR_SCALEW", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_RELIEF] = String("CHRATR_RELIEF", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHRATR_HIDDEN] = String("CHRATR_HIDDEN", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_AUTOFMT] = String("TXTATR_AUTOFMT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_INETFMT] = String("TXTATR_INETFMT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_REFMARK] = String("TXTATR_REFMARK", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_TOXMARK] = String("TXTATR_TOXMARK", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_CHARFMT] = String("TXTATR_CHARFMT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_DUMMY5] = String("TXTATR_DUMMY5", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_CJK_RUBY] = String("TXTATR_CJK_RUBY", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_UNKNOWN_CONTAINER] = String("TXTATR_UNKNOWN_CONTAINER", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_DUMMY6] = String("TXTATR_DUMMY6", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_DUMMY7] = String("TXTATR_DUMMY7", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_FIELD] = String("TXTATR_FIELD", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_FLYCNT] = String("TXTATR_FLYCNT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_FTN] = String("TXTATR_FTN", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_SOFTHYPH] = String("TXTATR_SOFTHYPH", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_HARDBLANK] = String("TXTATR_HARDBLANK", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_DUMMY1] = String("TXTATR_DUMMY1", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TXTATR_DUMMY2] = String("TXTATR_DUMMY2", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_LINESPACING] = String("PARATR_LINESPACING", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_ADJUST] = String("PARATR_ADJUST", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_SPLIT] = String("PARATR_SPLIT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_ORPHANS] = String("PARATR_ORPHANS", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_WIDOWS] = String("PARATR_WIDOWS", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_TABSTOP] = String("PARATR_TABSTOP", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_HYPHENZONE] = String("PARATR_HYPHENZONE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_DROP] = String("PARATR_DROP", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_REGISTER] = String("PARATR_REGISTER", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_NUMRULE] = String("PARATR_NUMRULE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_SCRIPTSPACE] = String("PARATR_SCRIPTSPACE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_HANGINGPUNCTUATION] = String("PARATR_HANGINGPUNCTUATION", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_FORBIDDEN_RULES] = String("PARATR_FORBIDDEN_RULES", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_VERTALIGN] = String("PARATR_VERTALIGN", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_SNAPTOGRID] = String("PARATR_SNAPTOGRID", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PARATR_CONNECT_BORDER] = String("PARATR_CONNECT_BORDER", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_FILL_ORDER] = String("FILL_ORDER", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_FRM_SIZE] = String("FRM_SIZE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PAPER_BIN] = String("PAPER_BIN", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_LR_SPACE] = String("LR_SPACE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_UL_SPACE] = String("UL_SPACE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PAGEDESC] = String("PAGEDESC", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_BREAK] = String("BREAK", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CNTNT] = String("CNTNT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_HEADER] = String("HEADER", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_FOOTER] = String("FOOTER", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PRINT] = String("PRINT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_OPAQUE] = String("OPAQUE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_PROTECT] = String("PROTECT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_SURROUND] = String("SURROUND", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_VERT_ORIENT] = String("VERT_ORIENT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_HORI_ORIENT] = String("HORI_ORIENT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_ANCHOR] = String("ANCHOR", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_BACKGROUND] = String("BACKGROUND", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_BOX] = String("BOX", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_SHADOW] = String("SHADOW", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_FRMMACRO] = String("FRMMACRO", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_COL] = String("COL", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_KEEP] = String("KEEP", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_URL] = String("URL", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_EDIT_IN_READONLY] = String("EDIT_IN_READONLY", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_LAYOUT_SPLIT] = String("LAYOUT_SPLIT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_CHAIN] = String("CHAIN", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_TEXTGRID] = String("TEXTGRID", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_LINENUMBER  ] = String("LINENUMBER  ", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_FTN_AT_TXTEND] = String("FTN_AT_TXTEND", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_END_AT_TXTEND] = String("END_AT_TXTEND", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_COLUMNBALANCE] = String("COLUMNBALANCE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_FRAMEDIR] = String("FRAMEDIR", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_HEADER_FOOTER_EAT_SPACING] = String("HEADER_FOOTER_EAT_SPACING", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_ROW_SPLIT] = String("ROW_SPLIT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_MIRRORGRF] = String("GRFATR_MIRRORGRF", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_CROPGRF] = String("GRFATR_CROPGRF", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_ROTATION] = String("GRFATR_ROTATION", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_LUMINANCE] = String("GRFATR_LUMINANCE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_CONTRAST] = String("GRFATR_CONTRAST", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_CHANNELR] = String("GRFATR_CHANNELR", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_CHANNELG] = String("GRFATR_CHANNELG", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_CHANNELB] = String("GRFATR_CHANNELB", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_GAMMA] = String("GRFATR_GAMMA", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_INVERT] = String("GRFATR_INVERT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_TRANSPARENCY] = String("GRFATR_TRANSPARENCY", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_GRFATR_DRAWMODE] = String("GRFATR_DRAWMODE", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_BOXATR_FORMAT] = String("BOXATR_FORMAT", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_BOXATR_FORMULA] = String("BOXATR_FORMULA", RTL_TEXTENCODING_ASCII_US);
        aItemWhichMap[RES_BOXATR_VALUE] = String("BOXATR_VALUE", RTL_TEXTENCODING_ASCII_US);

        bInitialized = true;
    }

    return aItemWhichMap;
}

const String lcl_dbg_out(const SfxPoolItem & rItem)
{
    String aStr("[ ", RTL_TEXTENCODING_ASCII_US);

    if (GetItemWhichMap().find(rItem.Which()) != GetItemWhichMap().end())
        aStr += GetItemWhichMap()[rItem.Which()];
    else
        aStr += String::CreateFromInt32(rItem.Which());

    aStr += String(" ]", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const SfxPoolItem & rItem)
{
    return dbg_out(lcl_dbg_out(rItem));
}

const char * dbg_out(const SfxPoolItem * pItem)
{
    return dbg_out(pItem ? lcl_dbg_out(*pItem) :
                   String("(nil)", RTL_TEXTENCODING_ASCII_US));
}

const String lcl_dbg_out(const SfxItemSet & rSet)
{
    SfxItemIter aIter(rSet);
    const SfxPoolItem * pItem;
    bool bFirst = true;
    String aStr = String("[ ", RTL_TEXTENCODING_ASCII_US);

    pItem = aIter.FirstItem();

    while (pItem )
    {
        if (!bFirst)
            aStr += String(", ", RTL_TEXTENCODING_ASCII_US);

        if ((sal_uInt32)pItem != 0xffffffff)
            aStr += lcl_dbg_out(*pItem);
        else
            aStr += String("invalid", RTL_TEXTENCODING_ASCII_US);

        bFirst = false;

        pItem = aIter.NextItem();
    }

    aStr += String(" ]", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const SfxItemSet & rSet)
{
    return dbg_out(lcl_dbg_out(rSet));
}

const String lcl_dbg_out(const SwTxtAttr & rAttr)
{
    String aStr("[ ", RTL_TEXTENCODING_ASCII_US);

    aStr += String::CreateFromInt32(*rAttr.GetStart());
    aStr += String("->", RTL_TEXTENCODING_ASCII_US);
    aStr += String::CreateFromInt32(*rAttr.GetEnd());
    aStr += String(" ", RTL_TEXTENCODING_ASCII_US);
    aStr += lcl_dbg_out(rAttr.GetAttr());

    aStr += String(" ]", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const SwTxtAttr & rAttr)
{
    return dbg_out(lcl_dbg_out(rAttr));
}

const String lcl_dbg_out(const SwpHints & rHints)
{
    String aStr("[ SwpHints\n", RTL_TEXTENCODING_ASCII_US);

    for (USHORT i = 0; i < rHints.Count(); i++)
    {
        aStr += String("  ", RTL_TEXTENCODING_ASCII_US);
        aStr += lcl_dbg_out(*rHints[i]);
        aStr += String("\n", RTL_TEXTENCODING_ASCII_US);
    }

    aStr += String("]\n", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const SwpHints &rHints)
{
    return dbg_out(lcl_dbg_out(rHints));
}

String lcl_dbg_out(const SwPosition & rPos)
{
    String aStr("( ", RTL_TEXTENCODING_ASCII_US);

    aStr += String::CreateFromInt32(rPos.nNode.GetIndex());
    aStr += String(", ", RTL_TEXTENCODING_ASCII_US);
    aStr += String::CreateFromInt32(rPos.nContent.GetIndex());
    aStr += String(": ", RTL_TEXTENCODING_ASCII_US);
    aStr += String::CreateFromInt32
        (reinterpret_cast<sal_IntPtr>(rPos.nContent.GetIdxReg()), 16);

    aStr += String(" )", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const SwPosition & rPos)
{
    return dbg_out(lcl_dbg_out(rPos));
}

String lcl_dbg_out(const SwPaM & rPam)
{
   String aStr("[ Pt: ", RTL_TEXTENCODING_ASCII_US);

   aStr += lcl_dbg_out(*rPam.GetPoint());

   if (rPam.HasMark())
   {
       aStr += String(", Mk: ", RTL_TEXTENCODING_ASCII_US);
       aStr += lcl_dbg_out(*rPam.GetMark());
   }

   aStr += String(" ]", RTL_TEXTENCODING_ASCII_US);

   return aStr;
}

const char * dbg_out(const SwPaM & rPam)
{
    return dbg_out(lcl_dbg_out(rPam));
}

String lcl_dbg_out(const SwNodeNum & rNum)
{
    return rNum.ToString();
}

const char * dbg_out(const SwNodeNum & rNum)
{
    return dbg_out(lcl_dbg_out(rNum));
}

String lcl_dbg_out(const SwRect & rRect)
{
    String aResult("[ [", RTL_TEXTENCODING_ASCII_US);

    aResult += String::CreateFromInt32(rRect.Left());
    aResult += String(", ", RTL_TEXTENCODING_ASCII_US);
    aResult += String::CreateFromInt32(rRect.Top());
    aResult += String("], [", RTL_TEXTENCODING_ASCII_US);
    aResult += String::CreateFromInt32(rRect.Right());
    aResult += String(", ", RTL_TEXTENCODING_ASCII_US);
    aResult += String::CreateFromInt32(rRect.Bottom());

    aResult += String("] ]", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

const char * dbg_out(const SwRect & rRect)
{
    return dbg_out(lcl_dbg_out(rRect));
}

String lcl_dbg_out(const SwFrmFmt & rFrmFmt)
{
    String aResult("[ ", RTL_TEXTENCODING_ASCII_US);

    char sBuffer[256];
    sprintf(sBuffer, "%p", &rFrmFmt);

    aResult += String(sBuffer, RTL_TEXTENCODING_ASCII_US);
    aResult += String("(", RTL_TEXTENCODING_ASCII_US);
    aResult += rFrmFmt.GetName();
    aResult += String(")", RTL_TEXTENCODING_ASCII_US);

    if (rFrmFmt.IsAuto())
        aResult += String("*", RTL_TEXTENCODING_ASCII_US);

    aResult += String(" ,", RTL_TEXTENCODING_ASCII_US);
    aResult += lcl_dbg_out(rFrmFmt.FindLayoutRect());
    aResult += String(" ]", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

const char * dbg_out(const SwFrmFmt & rFrmFmt)
{
    return dbg_out(lcl_dbg_out(rFrmFmt));
}

const String lcl_AnchoredFrames(const SwNode & rNode)
{
    String aResult("[", RTL_TEXTENCODING_ASCII_US);

    const SwDoc * pDoc = rNode.GetDoc();
    if (pDoc)
    {
        const SwSpzFrmFmts * pFrmFmts = pDoc->GetSpzFrmFmts();

        if (pFrmFmts)
        {
            bool bFirst = true;
            for (USHORT nI = 0; nI < pFrmFmts->Count(); nI++)
            {
                const SwFmtAnchor & rAnchor = (*pFrmFmts)[nI]->GetAnchor();
                const SwPosition * pPos = rAnchor.GetCntntAnchor();

                if (pPos && &pPos->nNode.GetNode() == &rNode)
                {
                    if (! bFirst)
                        aResult += String(", ", RTL_TEXTENCODING_ASCII_US);

                    if ((*pFrmFmts)[nI])
                        aResult += lcl_dbg_out(*(*pFrmFmts)[nI]);
                    bFirst = false;
                }
            }
        }
    }

    aResult += String("]", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

String lcl_dbg_out_NumType(sal_Int16 nType)
{
    String aTmpStr;

    switch (nType)
    {
    case SVX_NUM_NUMBER_NONE:
        aTmpStr += String(" NONE", RTL_TEXTENCODING_ASCII_US);

        break;
    case SVX_NUM_CHARS_UPPER_LETTER:
        aTmpStr += String(" CHARS_UPPER_LETTER",
                          RTL_TEXTENCODING_ASCII_US);

        break;
    case SVX_NUM_CHARS_LOWER_LETTER:
        aTmpStr += String(" CHARS_LOWER_LETTER",
                          RTL_TEXTENCODING_ASCII_US);

        break;
    case SVX_NUM_ROMAN_UPPER:
        aTmpStr += String(" ROMAN_UPPER",
                          RTL_TEXTENCODING_ASCII_US);

        break;
    case SVX_NUM_ROMAN_LOWER:
        aTmpStr += String(" ROMAN_LOWER",
                          RTL_TEXTENCODING_ASCII_US);

        break;
    case SVX_NUM_ARABIC:
        aTmpStr += String(" ARABIC",
                          RTL_TEXTENCODING_ASCII_US);

        break;
    default:
        aTmpStr += String(" ??",
                          RTL_TEXTENCODING_ASCII_US);

        break;
    }

    return aTmpStr;
}

String lcl_dbg_out(const SwNode & rNode)
{
    String aTmpStr;

    aTmpStr += String("[ Idx: ", RTL_TEXTENCODING_ASCII_US);
    aTmpStr += String::CreateFromInt32(rNode.GetIndex());

#ifndef PRODUCT
    aTmpStr += String("(", RTL_TEXTENCODING_ASCII_US);
    aTmpStr += String::CreateFromInt32(rNode.GetSerial());
    aTmpStr += String(")", RTL_TEXTENCODING_ASCII_US);
#endif

    aTmpStr += String(" ", RTL_TEXTENCODING_ASCII_US);

    char aBuffer[128];
    sprintf(aBuffer, "%p", &rNode);
    aTmpStr += String(aBuffer, RTL_TEXTENCODING_ASCII_US);

    const SwTxtNode * pTxtNode = rNode.GetTxtNode();

    if (rNode.IsStartNode())
        aTmpStr += String(" Start", RTL_TEXTENCODING_ASCII_US);
    else if (rNode.IsEndNode())
        aTmpStr += String(" End", RTL_TEXTENCODING_ASCII_US);
    else if (rNode.IsTxtNode())
    {
        const SfxItemSet * pAttrSet = pTxtNode->GetpSwAttrSet();

        aTmpStr += String(" Txt ", RTL_TEXTENCODING_ASCII_US);
        aTmpStr += pTxtNode->GetTxt().Copy(0, 10);

        if (rNode.IsTableNode())
            aTmpStr += String(" Tbl", RTL_TEXTENCODING_ASCII_US);

        aTmpStr += String(" olvl:", RTL_TEXTENCODING_ASCII_US);
        aTmpStr += String::CreateFromInt32(pTxtNode->GetOutlineLevel());

        const SwNumRule * pNumRule = pTxtNode->GetNumRule();

        if (pNumRule != NULL)
        {
            aTmpStr += String(" Num: ", RTL_TEXTENCODING_ASCII_US);
            if ( pTxtNode->GetNum() )
            {
                aTmpStr += lcl_dbg_out(*(pTxtNode->GetNum()));
            }

            aTmpStr += String(" Rule: ", RTL_TEXTENCODING_ASCII_US);
            aTmpStr += pNumRule->GetName();

            const SfxPoolItem * pItem = NULL;

            if (pAttrSet && SFX_ITEM_SET ==
                pAttrSet->GetItemState(RES_PARATR_NUMRULE, FALSE, &pItem))
            {
                aTmpStr += String("(", RTL_TEXTENCODING_ASCII_US);
                aTmpStr +=
                    static_cast<const SwNumRuleItem *>(pItem)->GetValue();
                aTmpStr += String(")", RTL_TEXTENCODING_ASCII_US);
                aTmpStr += String("*", RTL_TEXTENCODING_ASCII_US);
            }

            const SwNumFmt * pNumFmt = NULL;

            if (pTxtNode->GetLevel() > 0)
                pNumFmt = pNumRule->GetNumFmt( static_cast< USHORT >(pTxtNode->GetLevel()) );

            if (pNumFmt)
            {
                aTmpStr += String(" NumFmt: ", RTL_TEXTENCODING_ASCII_US);
                aTmpStr +=
                    lcl_dbg_out_NumType(pNumFmt->GetNumberingType());
            }
        }

        if (pTxtNode->IsCounted())
            aTmpStr += String(" counted", RTL_TEXTENCODING_ASCII_US);

        SwFmtColl * pColl = pTxtNode->GetFmtColl();

        if (pColl)
        {
            aTmpStr += String(" Coll: ", RTL_TEXTENCODING_ASCII_US);
            aTmpStr += pColl->GetName();

            aTmpStr += String("(", RTL_TEXTENCODING_ASCII_US);
            aTmpStr += String::CreateFromInt32
                (static_cast<SwTxtFmtColl *>(pColl)->GetOutlineLevel());

            const SwNumRuleItem & rItem =
                static_cast<const SwNumRuleItem &>
                (pColl->GetAttr(RES_PARATR_NUMRULE));
            const String sNumruleName = rItem.GetValue();

            if (sNumruleName.Len() > 0)
            {
                aTmpStr += String(", ", RTL_TEXTENCODING_ASCII_US);
                aTmpStr += sNumruleName;
            }
            aTmpStr += String(")", RTL_TEXTENCODING_ASCII_US);
        }

        SwFmtColl * pCColl = pTxtNode->GetCondFmtColl();

        if (pCColl)
        {
            aTmpStr += String(" CCOll: ", RTL_TEXTENCODING_ASCII_US);
            aTmpStr += pCColl->GetName();
        }

        aTmpStr += String(", Frms: ", RTL_TEXTENCODING_ASCII_US);
        aTmpStr += lcl_AnchoredFrames(rNode);

        if (bDbgOutPrintAttrSet)
        {
            aTmpStr += String(" Attrs: ", RTL_TEXTENCODING_ASCII_US);
            aTmpStr += lcl_dbg_out(pTxtNode->GetSwAttrSet());
        }
    }

    aTmpStr += String(" ]", RTL_TEXTENCODING_ASCII_US);

    return aTmpStr;
}

const char * dbg_out(const SwNode & rNode)
{
    return dbg_out(lcl_dbg_out(rNode));
}

const char * dbg_out(const SwNode * pNode)
{
    if (NULL != pNode)
        return dbg_out(*pNode);
    else
        return NULL;
}

const char * dbg_out(const SwCntntNode * pNode)
{
    if (NULL != pNode)
        return dbg_out(*pNode);
    else
        return NULL;
}

const char * dbg_out(const SwTxtNode * pNode)
{
    if (NULL != pNode)
        return dbg_out(*pNode);
    else
        return NULL;
}

BOOL lcl_dbg_add_node(const SwNodePtr & pNode, void * pArgs)
{
    if (pNode)
    {
        (*((String *) pArgs)) += lcl_dbg_out(*pNode);
        (*((String *) pArgs)) += String("\n", RTL_TEXTENCODING_ASCII_US);
    }

    //MBA: this code didn't compile, needed to add a return value
    return TRUE;
}

String lcl_dbg_out(SwNodes & rNodes)
{
    String aStr("[\n", RTL_TEXTENCODING_ASCII_US);

    for (ULONG i = 0; i < rNodes.Count(); i++)
    {
        aStr += lcl_dbg_out(*rNodes[i]);
        aStr += String("\n", RTL_TEXTENCODING_ASCII_US);
    }

    aStr += String("]\n", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(SwNodes & rNodes)
{
    return dbg_out(lcl_dbg_out(rNodes));
}

String lcl_dbg_out(const SwUndo & rUndo)
{
    String aStr("[ ", RTL_TEXTENCODING_ASCII_US);

    aStr += String::CreateFromInt32(rUndo.GetId());
    aStr += String(": ", RTL_TEXTENCODING_ASCII_US);

    switch(rUndo.GetId())
    {
    case UNDO_START:
        aStr += String(", ", RTL_TEXTENCODING_ASCII_US);
        aStr +=
            String::CreateFromInt32(dynamic_cast
                                    <const SwUndoStart &>(rUndo).
                                    GetUserId());
        aStr += String(", ", RTL_TEXTENCODING_ASCII_US);
        aStr += String::CreateFromInt32(dynamic_cast
                                        <const SwUndoStart &>(rUndo).
                                        GetEndOffset());
        aStr += String(" ", RTL_TEXTENCODING_ASCII_US);

        break;

    case UNDO_END:
        aStr += String(", ", RTL_TEXTENCODING_ASCII_US);
        aStr +=
            String::CreateFromInt32(dynamic_cast
                                    <const SwUndoEnd &>(rUndo).
                                    GetId());
        aStr += String(", ", RTL_TEXTENCODING_ASCII_US);
        aStr += String::CreateFromInt32(dynamic_cast
                                        <const SwUndoEnd &>(rUndo).
                                        GetSttOffset());
        aStr += String(" ", RTL_TEXTENCODING_ASCII_US);

        break;

    default:
        break;
    }

    aStr += rUndo.GetComment();
    aStr += String(" ]", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const SwUndo & rUndo)
{
    return dbg_out(lcl_dbg_out(rUndo));
}

String lcl_dbg_out(SwOutlineNodes & rNodes)
{
    String aStr("[\n", RTL_TEXTENCODING_ASCII_US);

    for (USHORT i = 0; i < rNodes.Count(); i++)
    {
        aStr += lcl_dbg_out(*rNodes[i]);
        aStr += String("\n", RTL_TEXTENCODING_ASCII_US);
    }

    aStr += String("]\n", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(SwOutlineNodes & rNodes)
{
    return dbg_out(lcl_dbg_out(rNodes));
}

String lcl_dbg_out(const SwUndos & rUndos)
{
    USHORT nIndent = 0;

    String aStr("[\n", RTL_TEXTENCODING_ASCII_US);

    for (USHORT n = 0; n < rUndos.Count(); n++)
    {
        SwUndo * pUndo = rUndos[n];

        if (pUndo->GetId() == UNDO_END)
            nIndent--;

        for (USHORT nI = 0; n < nIndent; nI++)
            aStr += String("  ", RTL_TEXTENCODING_ASCII_US);

        aStr += lcl_dbg_out(*pUndo);
        aStr += String("\n", RTL_TEXTENCODING_ASCII_US);

        if (pUndo->GetId() == UNDO_START)
            nIndent++;
    }

    aStr += String("]\n", RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const SwUndos & rUndos)
{
    return dbg_out(lcl_dbg_out(rUndos));
}

String lcl_dbg_out(const SwRewriter & rRewriter)
{
    (void) rRewriter;
    String aResult;

    //aResult = rRewriter.ToString();

    return aResult;
}

const char * dbg_out(const SwRewriter & rRewriter)
{
    return dbg_out(lcl_dbg_out(rRewriter));
}

String lcl_dbg_out(const SvxNumberFormat & rFmt)
{
    String aResult;

    aResult = lcl_dbg_out_NumType(rFmt.GetNumberingType());

    return aResult;
}

String lcl_dbg_out(const SwNumRule & rRule)
{
    String aResult("[ ", RTL_TEXTENCODING_ASCII_US);

    aResult += rRule.GetName();
    aResult += String(" [", RTL_TEXTENCODING_ASCII_US);

    for (BYTE n = 0; n < MAXLEVEL; n++)
    {
        if (n > 0)
            aResult += String(", ", RTL_TEXTENCODING_ASCII_US);

        aResult += lcl_dbg_out(rRule.Get(n));
    }

    aResult += String("]", RTL_TEXTENCODING_ASCII_US);

    aResult += String("]", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

const char * dbg_out(const SwNumRule & rRule)
{
    return dbg_out(lcl_dbg_out(rRule));
}

String lcl_dbg_out(const SwTxtFmtColl & rFmt)
{
    String aResult(rFmt.GetName());

    aResult += String("(", RTL_TEXTENCODING_ASCII_US);
    aResult += String::CreateFromInt32(rFmt.GetOutlineLevel());
    aResult += String(")", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

const char * dbg_out(const SwTxtFmtColl & rFmt)
{
    return dbg_out(lcl_dbg_out(rFmt));
}

String lcl_dbg_out(const SwFrmFmts & rFrmFmts)
{
    return lcl_dbg_out_SvPtrArr<SwFrmFmts>(rFrmFmts);
}

const char * dbg_out(const SwFrmFmts & rFrmFmts)
{
    return dbg_out(lcl_dbg_out(rFrmFmts));
}

String lcl_dbg_out(const SwNumRuleTbl & rTbl)
{
    String aResult("[", RTL_TEXTENCODING_ASCII_US);

    for (USHORT n = 0; n < rTbl.Count(); n++)
    {
        if (n > 0)
            aResult += String(", ", RTL_TEXTENCODING_ASCII_US);

        aResult += rTbl[n]->GetName();

        char sBuffer[256];
        sprintf(sBuffer, "(%p)", rTbl[n]);
        aResult += String(sBuffer, RTL_TEXTENCODING_ASCII_US);
    }

    aResult += String("]", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

const char * dbg_out(const SwNumRuleTbl & rTbl)
{
    return dbg_out(lcl_dbg_out(rTbl));
}

String lcl_TokenType2Str(FormTokenType nType)
{
    switch(nType)
    {
    case TOKEN_ENTRY_NO:
        return String("NO", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_ENTRY_TEXT:
        return String("ENTRY_TEXT", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_ENTRY:
        return String("ENTRY", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_TAB_STOP:
        return String("TAB_STOP", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_TEXT:
        return String("TOKEN_TEXT", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_PAGE_NUMS:
        return String("NUMS", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_CHAPTER_INFO:
        return String("CHAPTER_INFO", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_LINK_START:
        return String("LINK_START", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_LINK_END:
        return String("LINK_END", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_AUTHORITY:
        return String("AUTHORITY", RTL_TEXTENCODING_ASCII_US);
    case TOKEN_END:
        return String("END", RTL_TEXTENCODING_ASCII_US);
    default:
        return String("??", RTL_TEXTENCODING_ASCII_US);
    }

    ASSERT(false, "should not be reached");

    return  String("??", RTL_TEXTENCODING_ASCII_US);
}

String lcl_dbg_out(const SwFormToken & rToken)
{
    return rToken.GetString();
}

const char * dbg_out(const SwFormToken & rToken)
{
    return dbg_out(lcl_dbg_out(rToken));
}

String lcl_dbg_out(const SwFormTokens & rTokens)
{
    String aStr("[", RTL_TEXTENCODING_ASCII_US);

    SwFormTokens::const_iterator aIt;

    for (aIt = rTokens.begin(); aIt != rTokens.end(); aIt++)
    {
        if (aIt != rTokens.begin())
            aStr += String(", ", RTL_TEXTENCODING_ASCII_US);

        aStr += lcl_TokenType2Str(aIt->eTokenType);
        aStr += String(": ", RTL_TEXTENCODING_ASCII_US);
        aStr += lcl_dbg_out(*aIt);
    }

    aStr += String("]" , RTL_TEXTENCODING_ASCII_US);

    return aStr;
}

const char * dbg_out(const SwFormTokens & rTokens)
{
    return dbg_out(lcl_dbg_out(rTokens));
}
#endif // DEBUG


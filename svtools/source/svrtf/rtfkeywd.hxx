/*************************************************************************
 *
 *  $RCSfile: rtfkeywd.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-16 17:54:11 $
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

#ifndef _RTFKEYWD_HXX
#define _RTFKEYWD_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef SVTOOLS_CONSTASCII_DECL
#define SVTOOLS_CONSTASCII_DECL( n, s ) n[sizeof(s)]
#endif
#ifndef SVTOOLS_CONSTASCII_DEF
#define SVTOOLS_CONSTASCII_DEF( n, s ) n[sizeof(s)] = s
#endif

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEXCHAR, "\\'" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_IGNORE, "\\*" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OPTHYPH, "\\-" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SUBENTRY, "\\:" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ABSH, "\\absh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ABSW, "\\absw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ALT, "\\alt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ANNOTATION, "\\annotation" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ANSI, "\\ansi" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ATNID, "\\atnid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AUTHOR, "\\author" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_B, "\\b" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGBDIAG, "\\bgbdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGCROSS, "\\bgcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGDCROSS, "\\bgdcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGDKBDIAG, "\\bgdkbdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGDKCROSS, "\\bgdkcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGDKDCROSS, "\\bgdkdcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGDKFDIAG, "\\bgdkfdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGDKHORIZ, "\\bgdkhoriz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGDKVERT, "\\bgdkvert" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGFDIAG, "\\bgfdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGHORIZ, "\\bghoriz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BGVERT, "\\bgvert" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BIN, "\\bin" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BINFSXN, "\\binfsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BINSXN, "\\binsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BKMKCOLF, "\\bkmkcolf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BKMKCOLL, "\\bkmkcoll" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BKMKEND, "\\bkmkend" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BKMKSTART, "\\bkmkstart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BLUE, "\\blue" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BOX, "\\box" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRB, "\\brdrb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRBAR, "\\brdrbar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRBTW, "\\brdrbtw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRCF, "\\brdrcf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRDB, "\\brdrdb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRDOT, "\\brdrdot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRHAIR, "\\brdrhair" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRL, "\\brdrl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRR, "\\brdrr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRS, "\\brdrs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRSH, "\\brdrsh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRT, "\\brdrt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTH, "\\brdrth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRW, "\\brdrw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRSP, "\\brsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BULLET, "\\bullet" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BUPTIM, "\\buptim" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BXE, "\\bxe" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CAPS, "\\caps" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CB, "\\cb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CBPAT, "\\cbpat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CELL, "\\cell" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CELLX, "\\cellx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CF, "\\cf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CFPAT, "\\cfpat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHATN, "\\chatn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHDATE, "\\chdate" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHDPA, "\\chdpa" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHDPL, "\\chdpl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHFTN, "\\chftn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHFTNSEP, "\\chftnsep" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHFTNSEPC, "\\chftnsepc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHPGN, "\\chpgn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHTIME, "\\chtime" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGBDIAG, "\\clbgbdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGCROSS, "\\clbgcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGDCROSS, "\\clbgdcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGDKBDIAG, "\\clbgdkbdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGDKCROSS, "\\clbgdkcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGDKDCROSS, "\\clbgdkdcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGDKFDIAG, "\\clbgdkfdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGDKHOR, "\\clbgdkhor" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGDKVERT, "\\clbgdkvert" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGFDIAG, "\\clbgfdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGHORIZ, "\\clbghoriz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBGVERT, "\\clbgvert" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBRDRB, "\\clbrdrb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBRDRL, "\\clbrdrl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBRDRR, "\\clbrdrr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLBRDRT, "\\clbrdrt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLCBPAT, "\\clcbpat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLCFPAT, "\\clcfpat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLMGF, "\\clmgf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLMRG, "\\clmrg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLSHDNG, "\\clshdng" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COLNO, "\\colno" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COLORTBL, "\\colortbl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COLS, "\\cols" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COLSR, "\\colsr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COLSX, "\\colsx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COLUMN, "\\column" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COLW, "\\colw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COMMENT, "\\comment" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CREATIM, "\\creatim" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CTRL, "\\ctrl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DEFF, "\\deff" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DEFFORMAT, "\\defformat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DEFLANG, "\\deflang" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DEFTAB, "\\deftab" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DELETED, "\\deleted" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRMTXTX, "\\dfrmtxtx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRMTXTY, "\\dfrmtxty" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DIBITMAP, "\\dibitmap" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DN, "\\dn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOCCOMM, "\\doccomm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOCTEMP, "\\doctemp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DROPCAPLI, "\\dropcapli" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DROPCAPLT, "\\dropcapt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DXFRTEXT, "\\dxfrtext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DY, "\\dy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_EDMINS, "\\edmins" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_EMDASH, "\\emdash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ENDASH, "\\endash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ENDDOC, "\\enddoc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ENDNHERE, "\\endnhere" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ENDNOTES, "\\endnotes" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_EXPND, "\\expnd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_EXPNDTW, "\\expndtw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_F, "\\f" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FACINGP, "\\facingp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FACPGSXN, "\\facpgsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FALT, "\\falt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FCHARSET, "\\fcharset" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FDECOR, "\\fdecor" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FI, "\\fi" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FIELD, "\\field" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLDDIRTY, "\\flddirty" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLDEDIT, "\\fldedit" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLDINST, "\\fldinst" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLDLOCK, "\\fldlock" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLDPRIV, "\\fldpriv" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLDRSLT, "\\fldrslt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FMODERN, "\\fmodern" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FN, "\\fn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FNIL, "\\fnil" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FONTTBL, "\\fonttbl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTER, "\\footer" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTERF, "\\footerf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTERL, "\\footerl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTERR, "\\footerr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTERY, "\\footery" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTNOTE, "\\footnote" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FPRQ, "\\fprq" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FRACWIDTH, "\\fracwidth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FROMAN, "\\froman" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FS, "\\fs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FSCRIPT, "\\fscript" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FSWISS, "\\fswiss" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTECH, "\\ftech" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNBJ, "\\ftnbj" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNCN, "\\ftncn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNRESTART, "\\ftnrestart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNSEP, "\\ftnsep" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNSEPC, "\\ftnsepc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNSTART, "\\ftnstart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNTJ, "\\ftntj" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_GREEN, "\\green" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_GUTTER, "\\gutter" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_GUTTERSXN, "\\guttersxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADER, "\\header" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADERF, "\\headerf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADERL, "\\headerl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADERR, "\\headerr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADERY, "\\headery" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HR, "\\hr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHHOTZ, "\\hyphhotz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_I, "\\i" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ID, "\\id" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_INFO, "\\info" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_INTBL, "\\intbl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_IXE, "\\ixe" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_KEEP, "\\keep" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_KEEPN, "\\keepn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_KERNING, "\\kerning" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_KEYCODE, "\\keycode" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_KEYWORDS, "\\keywords" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LANDSCAPE, "\\landscape" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LANG, "\\lang" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LDBLQUOTE, "\\ldblquote" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVEL, "\\level" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LI, "\\li" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINE, "\\line" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINEBETCOL, "\\linebetcol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINECONT, "\\linecont" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINEMOD, "\\linemod" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINEPPAGE, "\\lineppage" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINERESTART, "\\linerestart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINESTART, "\\linestart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINESTARTS, "\\linestarts" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINEX, "\\linex" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LNDSCPSXN, "\\lndscpsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LQUOTE, "\\lquote" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MAC, "\\mac" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MACPICT, "\\macpict" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MAKEBACKUP, "\\makebackup" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGB, "\\margb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGBSXN, "\\margbsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGL, "\\margl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGLSXN, "\\marglsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGMIRROR, "\\margmirror" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGR, "\\margr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGRSXN, "\\margrsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGT, "\\margt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MARGTSXN, "\\margtsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MIN, "\\min" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MO, "\\mo" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NEXTCSET, "\\nextcset" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NEXTFILE, "\\nextfile" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOFCHARS, "\\nofchars" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOFPAGES, "\\nofpages" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOFWORDS, "\\nofwords" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOLINE, "\\noline" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOSUPERSUB, "\\nosupersub" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOWRAP, "\\nowrap" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OPERATOR, "\\operator" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OUTL, "\\outl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PAGE, "\\page" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PAGEBB, "\\pagebb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PAPERH, "\\paperh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PAPERW, "\\paperw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PAR, "\\par" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PARD, "\\pard" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PC, "\\pc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PCA, "\\pca" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGHSXN, "\\pghsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNCONT, "\\pgncont" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNDEC, "\\pgndec" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNLCLTR, "\\pgnlcltr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNLCRM, "\\pgnlcrm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNRESTART, "\\pgnrestart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNSTART, "\\pgnstart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNSTARTS, "\\pgnstarts" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNUCLTR, "\\pgnucltr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNUCRM, "\\pgnucrm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNX, "\\pgnx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNY, "\\pgny" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGWSXN, "\\pgwsxn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PHCOL, "\\phcol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PHMRG, "\\phmrg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PHPG, "\\phpg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICCROPB, "\\piccropb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICCROPL, "\\piccropl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICCROPR, "\\piccropr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICCROPT, "\\piccropt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICH, "\\pich" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICHGOAL, "\\pichgoal" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICSCALED, "\\picscaled" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICSCALEX, "\\picscalex" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICSCALEY, "\\picscaley" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICT, "\\pict" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICW, "\\picw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICWGOAL, "\\picwgoal" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PLAIN, "\\plain" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PMMETAFILE, "\\pmmetafile" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSNEGX, "\\posnegx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSNEGY, "\\posnegy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSX, "\\posx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSXC, "\\posxc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSXI, "\\posxi" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSXL, "\\posxl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSXO, "\\posxo" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSXR, "\\posxr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSY, "\\posy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSYB, "\\posyb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSYC, "\\posyc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSYIL, "\\posyil" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSYT, "\\posyt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PRINTIM, "\\printim" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PSOVER, "\\psover" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PVMRG, "\\pvmrg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PVPARA, "\\pvpara" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PVPG, "\\pvpg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_QC, "\\qc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_QJ, "\\qj" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_QL, "\\ql" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_QR, "\\qr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RDBLQUOTE, "\\rdblquote" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RED, "\\red" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVBAR, "\\revbar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVISED, "\\revised" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVISIONS, "\\revisions" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVPROP, "\\revprop" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVTIM, "\\revtim" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RI, "\\ri" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ROW, "\\row" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RQUOTE, "\\rquote" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RTF, "\\rtf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RXE, "\\rxe" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_S, "\\s" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SA, "\\sa" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SB, "\\sb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SBASEDON, "\\sbasedon" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SBKCOL, "\\sbkcol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SBKEVEN, "\\sbkeven" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SBKNONE, "\\sbknone" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SBKODD, "\\sbkodd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SBKPAGE, "\\sbkpage" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SBYS, "\\sbys" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SCAPS, "\\scaps" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECT, "\\sect" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECTD, "\\sectd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHAD, "\\shad" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHADING, "\\shading" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHIFT, "\\shift" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SL, "\\sl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SNEXT, "\\snext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_STRIKE, "\\strike" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_STYLESHEET, "\\stylesheet" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SUB, "\\sub" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SUBJECT, "\\subject" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SUPER, "\\super" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TAB, "\\tab" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TB, "\\tb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TC, "\\tc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TCF, "\\tcf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TCL, "\\tcl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TEMPLATE, "\\template" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TITLE, "\\title" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TITLEPG, "\\titlepg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TLDOT, "\\tldot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TLEQ, "\\tleq" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TLHYPH, "\\tlhyph" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TLTH, "\\tlth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TLUL, "\\tlul" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TQC, "\\tqc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TQDEC, "\\tqdec" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TQR, "\\tqr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TQL, "\\tql" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRGAPH, "\\trgaph" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRLEFT, "\\trleft" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TROWD, "\\trowd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRQC, "\\trqc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRQL, "\\trql" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRQR, "\\trqr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRRH, "\\trrh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TX, "\\tx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TXE, "\\txe" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_UL, "\\ul" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULD, "\\uld" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULDB, "\\uldb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULNONE, "\\ulnone" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULW, "\\ulw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_UP, "\\up" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_V, "\\v" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VERN, "\\vern" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VERSION, "\\version" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VERTALB, "\\vertalb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VERTALC, "\\vertalc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VERTALJ, "\\vertalj" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VERTALT, "\\vertalt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WBITMAP, "\\wbitmap" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WBMBITSPIXEL, "\\wbmbitspixel" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WBMPLANES, "\\wbmplanes" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WBMWIDTHBYTES, "\\wbmwidthbytes" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WIDOWCTRL, "\\widowctrl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WMETAFILE, "\\wmetafile" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_XE, "\\xe" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_YR, "\\yr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOBRKHYPH, "\\_" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FORMULA, "\\|" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOBREAK, "\\~" );


extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AB, "\\ab" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ACAPS, "\\acaps" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ACF, "\\acf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ADDITIVE, "\\additive" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ADN, "\\adn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AENDDOC, "\\aenddoc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AENDNOTES, "\\aendnotes" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AEXPND, "\\aexpnd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AF, "\\af" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFS, "\\afs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNBJ, "\\aftnbj" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNCN, "\\aftncn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNALC, "\\aftnnalc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNAR, "\\aftnnar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNAUC, "\\aftnnauc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNCHI, "\\aftnnchi" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNRLC, "\\aftnnrlc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNRUC, "\\aftnnruc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNRESTART, "\\aftnrestart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNRSTCONT, "\\aftnrstcont" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNSEP, "\\aftnsep" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNSEPC, "\\aftnsepc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNSTART, "\\aftnstart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNTJ, "\\aftntj" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AI, "\\ai" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ALANG, "\\alang" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ALLPROT, "\\allprot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ANNOTPROT, "\\annotprot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AOUTL, "\\aoutl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ASCAPS, "\\ascaps" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ASHAD, "\\ashad" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ASTRIKE, "\\astrike" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ATNAUTHOR, "\\atnauthor" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ATNICN, "\\atnicn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ATNREF, "\\atnref" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ATNTIME, "\\atntime" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ATRFEND, "\\atrfend" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ATRFSTART, "\\atrfstart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AUL, "\\aul" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AULD, "\\auld" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AULDB, "\\auldb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AULNONE, "\\aulnone" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AULW, "\\aulw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AUP, "\\aup" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BKMKPUB, "\\bkmkpub" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRDASH, "\\brdrdash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRKFRM, "\\brkfrm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CCHS, "\\cchs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CPG, "\\cpg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CS, "\\cs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CVMME, "\\cvmme" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DATAFIELD, "\\datafield" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DO, "\\do" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOBXCOLUMN, "\\dobxcolumn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOBXMARGIN, "\\dobxmargin" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOBXPAGE, "\\dobxpage" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOBYMARGIN, "\\dobymargin" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOBYPAGE, "\\dobypage" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOBYPARA, "\\dobypara" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DODHGT, "\\dodhgt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOLOCK, "\\dolock" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPAENDHOL, "\\dpaendhol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPAENDL, "\\dpaendl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPAENDSOL, "\\dpaendsol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPAENDW, "\\dpaendw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPARC, "\\dparc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPARCFLIPX, "\\dparcflipx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPARCFLIPY, "\\dparcflipy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPASTARTHOL, "\\dpastarthol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPASTARTL, "\\dpastartl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPASTARTSOL, "\\dpastartsol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPASTARTW, "\\dpastartw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCALLOUT, "\\dpcallout" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOA, "\\dpcoa" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOACCENT, "\\dpcoaccent" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOBESTFIT, "\\dpcobestfit" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOBORDER, "\\dpcoborder" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCODABS, "\\dpcodabs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCODBOTTOM, "\\dpcodbottom" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCODCENTER, "\\dpcodcenter" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCODTOP, "\\dpcodtop" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOLENGTH, "\\dpcolength" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOMINUSX, "\\dpcominusx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOMINUSY, "\\dpcominusy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOOFFSET, "\\dpcooffset" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOSMARTA, "\\dpcosmarta" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOTDOUBLE, "\\dpcotdouble" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOTRIGHT, "\\dpcotright" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOTSINGLE, "\\dpcotsingle" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOTTRIPLE, "\\dpcottriple" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCOUNT, "\\dpcount" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPELLIPSE, "\\dpellipse" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPENDGROUP, "\\dpendgroup" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLBGCB, "\\dpfillbgcb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLBGCG, "\\dpfillbgcg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLBGCR, "\\dpfillbgcr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLBGGRAY, "\\dpfillbggray" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLBGPAL, "\\dpfillbgpal" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLFGCB, "\\dpfillfgcb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLFGCG, "\\dpfillfgcg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLFGCR, "\\dpfillfgcr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLFGGRAY, "\\dpfillfggray" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLFGPAL, "\\dpfillfgpal" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPFILLPAT, "\\dpfillpat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPGROUP, "\\dpgroup" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINE, "\\dpline" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINECOB, "\\dplinecob" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINECOG, "\\dplinecog" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINECOR, "\\dplinecor" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINEDADO, "\\dplinedado" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINEDADODO, "\\dplinedadodo" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINEDASH, "\\dplinedash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINEDOT, "\\dplinedot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINEGRAY, "\\dplinegray" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINEHOLLOW, "\\dplinehollow" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINEPAL, "\\dplinepal" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINESOLID, "\\dplinesolid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPLINEW, "\\dplinew" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPPOLYCOUNT, "\\dppolycount" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPPOLYGON, "\\dppolygon" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPPOLYLINE, "\\dppolyline" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPPTX, "\\dpptx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPPTY, "\\dppty" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPRECT, "\\dprect" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPROUNDR, "\\dproundr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPSHADOW, "\\dpshadow" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPSHADX, "\\dpshadx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPSHADY, "\\dpshady" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPTXBX, "\\dptxbx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPTXBXMAR, "\\dptxbxmar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPTXBXTEXT, "\\dptxbxtext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPX, "\\dpx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPXSIZE, "\\dpxsize" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPY, "\\dpy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPYSIZE, "\\dpysize" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DS, "\\ds" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_EMSPACE, "\\emspace" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ENSPACE, "\\enspace" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FBIDI, "\\fbidi" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FET, "\\fet" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FID, "\\fid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FILE, "\\file" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FILETBL, "\\filetbl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLDALT, "\\fldalt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FNETWORK, "\\fnetwork" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FONTEMB, "\\fontemb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FONTFILE, "\\fontfile" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FORMDISP, "\\formdisp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FORMPROT, "\\formprot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FORMSHADE, "\\formshade" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOSNUM, "\\fosnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FRELATIVE, "\\frelative" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNALT, "\\ftnalt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNIL, "\\ftnil" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNALC, "\\ftnnalc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNAR, "\\ftnnar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNAUC, "\\ftnnauc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNCHI, "\\ftnnchi" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNRLC, "\\ftnnrlc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNRUC, "\\ftnnruc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNRSTCONT, "\\ftnrstcont" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNRSTPG, "\\ftnrstpg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTTRUETYPE, "\\fttruetype" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FVALIDDOS, "\\fvaliddos" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FVALIDHPFS, "\\fvalidhpfs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FVALIDMAC, "\\fvalidmac" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FVALIDNTFS, "\\fvalidntfs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHAUTO, "\\hyphauto" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHCAPS, "\\hyphcaps" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHCONSEC, "\\hyphconsec" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHPAR, "\\hyphpar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINKSELF, "\\linkself" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINKSTYLES, "\\linkstyles" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LTRCH, "\\ltrch" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LTRDOC, "\\ltrdoc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LTRMARK, "\\ltrmark" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LTRPAR, "\\ltrpar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LTRROW, "\\ltrrow" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LTRSECT, "\\ltrsect" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOCOLBAL, "\\nocolbal" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOEXTRASPRL, "\\noextrasprl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOTABIND, "\\notabind" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOWIDCTLPAR, "\\nowidctlpar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJALIAS, "\\objalias" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJALIGN, "\\objalign" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJAUTLINK, "\\objautlink" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJCLASS, "\\objclass" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJCROPB, "\\objcropb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJCROPL, "\\objcropl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJCROPR, "\\objcropr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJCROPT, "\\objcropt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJDATA, "\\objdata" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJECT, "\\object" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJEMB, "\\objemb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJH, "\\objh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJICEMB, "\\objicemb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJLINK, "\\objlink" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJLOCK, "\\objlock" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJNAME, "\\objname" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJPUB, "\\objpub" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJSCALEX, "\\objscalex" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJSCALEY, "\\objscaley" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJSECT, "\\objsect" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJSETSIZE, "\\objsetsize" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJSUB, "\\objsub" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJTIME, "\\objtime" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJTRANSY, "\\objtransy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJUPDATE, "\\objupdate" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJW, "\\objw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OTBLRUL, "\\otblrul" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNHN, "\\pgnhn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNHNSC, "\\pgnhnsc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNHNSH, "\\pgnhnsh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNHNSM, "\\pgnhnsm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNHNSN, "\\pgnhnsn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNHNSP, "\\pgnhnsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICBMP, "\\picbmp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICBPP, "\\picbpp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PN, "\\pn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNACROSS, "\\pnacross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNB, "\\pnb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNCAPS, "\\pncaps" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNCARD, "\\pncard" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNCF, "\\pncf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNDEC, "\\pndec" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNF, "\\pnf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNFS, "\\pnfs" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNHANG, "\\pnhang" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNI, "\\pni" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNINDENT, "\\pnindent" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNLCLTR, "\\pnlcltr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNLCRM, "\\pnlcrm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNLVL, "\\pnlvl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNLVLBLT, "\\pnlvlblt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNLVLBODY, "\\pnlvlbody" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNLVLCONT, "\\pnlvlcont" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNNUMONCE, "\\pnnumonce" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNORD, "\\pnord" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNORDT, "\\pnordt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNPREV, "\\pnprev" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNQC, "\\pnqc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNQL, "\\pnql" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNQR, "\\pnqr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRESTART, "\\pnrestart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNSCAPS, "\\pnscaps" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNSECLVL, "\\pnseclvl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNSP, "\\pnsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNSTART, "\\pnstart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNSTRIKE, "\\pnstrike" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNTEXT, "\\pntext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNTXTA, "\\pntxta" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNTXTB, "\\pntxtb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNUCLTR, "\\pnucltr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNUCRM, "\\pnucrm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNUL, "\\pnul" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNULD, "\\pnuld" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNULDB, "\\pnuldb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNULNONE, "\\pnulnone" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNULW, "\\pnulw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PRCOLBL, "\\prcolbl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PRINTDATA, "\\printdata" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PSZ, "\\psz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PUBAUTO, "\\pubauto" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RESULT, "\\result" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVAUTH, "\\revauth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVDTTM, "\\revdttm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVPROT, "\\revprot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVTBL, "\\revtbl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RSLTBMP, "\\rsltbmp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RSLTMERGE, "\\rsltmerge" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RSLTPICT, "\\rsltpict" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RSLTRTF, "\\rsltrtf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RSLTTXT, "\\rslttxt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RTLCH, "\\rtlch" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RTLDOC, "\\rtldoc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RTLMARK, "\\rtlmark" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RTLPAR, "\\rtlpar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RTLROW, "\\rtlrow" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_RTLSECT, "\\rtlsect" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SEC, "\\sec" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECTNUM, "\\sectnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECTUNLOCKED, "\\sectunlocked" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SLMULT, "\\slmult" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SOFTCOL, "\\softcol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SOFTLHEIGHT, "\\softlheight" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SOFTLINE, "\\softline" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SOFTPAGE, "\\softpage" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SPRSSPBF, "\\sprsspbf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SPRSTSP, "\\sprstsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SUBDOCUMENT, "\\subdocument" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SWPBDR, "\\swpbdr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TCN, "\\tcn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRANSMF, "\\transmf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRBRDRB, "\\trbrdrb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRBRDRH, "\\trbrdrh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRBRDRL, "\\trbrdrl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRBRDRR, "\\trbrdrr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRBRDRT, "\\trbrdrt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRBRDRV, "\\trbrdrv" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRHDR, "\\trhdr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRKEEP, "\\trkeep" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WRAPTRSP, "\\wraptrsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_XEF, "\\xef" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ZWJ, "\\zwj" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ZWNJ, "\\zwnj" );

// neue Tokens zur 1.5
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ABSLOCK, "\\abslock" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ADJUSTRIGHT, "\\adjustright" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNCHOSUNG, "\\aftnnchosung" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNCNUM, "\\aftnncnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNDBAR, "\\aftnndbar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNDBNUM, "\\aftnndbnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNDBNUMD, "\\aftnndbnumd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNDBNUMK, "\\aftnndbnumk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNDBNUMT, "\\aftnndbnumt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNGANADA, "\\aftnnganada" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNGBNUM, "\\aftnngbnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNGBNUMD, "\\aftnngbnumd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNGBNUMK, "\\aftnngbnumk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNGBNUML, "\\aftnngbnuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNZODIAC, "\\aftnnzodiac" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNZODIACD, "\\aftnnzodiacd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_AFTNNZODIACL, "\\aftnnzodiacl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ANIMTEXT, "\\animtext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ANSICPG, "\\ansicpg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BACKGROUND, "\\background" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BDBFHDR, "\\bdbfhdr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BLIPTAG, "\\bliptag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BLIPUID, "\\blipuid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BLIPUPI, "\\blipupi" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRART, "\\brdrart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRDASHD, "\\brdrdashd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRDASHDD, "\\brdrdashdd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRDASHDOTSTR, "\\brdrdashdotstr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRDASHSM, "\\brdrdashsm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDREMBOSS, "\\brdremboss" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRENGRAVE, "\\brdrengrave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRFRAME, "\\brdrframe" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTHTNLG, "\\brdrthtnlg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTHTNMG, "\\brdrthtnmg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTHTNSG, "\\brdrthtnsg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTNTHLG, "\\brdrtnthlg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTNTHMG, "\\brdrtnthmg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTNTHSG, "\\brdrtnthsg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTNTHTNLG, "\\brdrtnthtnlg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTNTHTNMG, "\\brdrtnthtnmg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTNTHTNSG, "\\brdrtnthtnsg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRTRIPLE, "\\brdrtriple" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRWAVY, "\\brdrwavy" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDRWAVYDB, "\\brdrwavydb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CATEGORY, "\\category" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CGRID, "\\cgrid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHARSCALEX, "\\charscalex" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGBDIAG, "\\chbgbdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGCROSS, "\\chbgcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGDCROSS, "\\chbgdcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGDKBDIAG, "\\chbgdkbdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGDKCROSS, "\\chbgdkcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGDKDCROSS, "\\chbgdkdcross" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGDKFDIAG, "\\chbgdkfdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGDKHORIZ, "\\chbgdkhoriz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGDKVERT, "\\chbgdkvert" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGFDIAG, "\\chbgfdiag" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGHORIZ, "\\chbghoriz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBGVERT, "\\chbgvert" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHBRDR, "\\chbrdr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHCBPAT, "\\chcbpat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHCFPAT, "\\chcfpat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CHSHDNG, "\\chshdng" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLTXLRTB, "\\cltxlrtb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLTXTBRL, "\\cltxtbrl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLVERTALB, "\\clvertalb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLVERTALC, "\\clvertalc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLVERTALT, "\\clvertalt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLVMGF, "\\clvmgf" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLVMRG, "\\clvmrg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLTXTBRLV, "\\cltxtbrlv" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLTXBTLR, "\\cltxbtlr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CLTXLRTBV, "\\cltxlrtbv" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_COMPANY, "\\company" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CRAUTH, "\\crauth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_CRDATE, "\\crdate" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DATE, "\\date" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DEFLANGFE, "\\deflangfe" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRAUTH, "\\dfrauth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRDATE, "\\dfrdate" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRSTART, "\\dfrstart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRSTOP, "\\dfrstop" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRXST, "\\dfrxst" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DGMARGIN, "\\dgmargin" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DNTBLNSBDB, "\\dntblnsbdb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOCTYPE, "\\doctype" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DOCVAR, "\\docvar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DPCODESCENT, "\\dpcodescent" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_EMBO, "\\embo" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_EMFBLIP, "\\emfblip" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_EXPSHRTN, "\\expshrtn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FAAUTO, "\\faauto" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FBIAS, "\\fbias" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFDEFRES, "\\ffdefres" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFDEFTEXT, "\\ffdeftext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFENTRYMCR, "\\ffentrymcr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFEXITMCR, "\\ffexitmcr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFFORMAT, "\\ffformat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFHASLISTBOX, "\\ffhaslistbox" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFHELPTEXT, "\\ffhelptext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFHPS, "\\ffhps" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFL, "\\ffl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFMAXLEN, "\\ffmaxlen" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFNAME, "\\ffname" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFOWNHELP, "\\ffownhelp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFOWNSTAT, "\\ffownstat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFPROT, "\\ffprot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFRECALC, "\\ffrecalc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFRES, "\\ffres" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFSIZE, "\\ffsize" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFSTATTEXT, "\\ffstattext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFTYPE, "\\fftype" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FFTYPETXT, "\\fftypetxt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLDTYPE, "\\fldtype" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FNAME, "\\fname" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FORMFIELD, "\\formfield" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FROMTEXT, "\\fromtext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNCHOSUNG, "\\ftnnchosung" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNCNUM, "\\ftnncnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNDBAR, "\\ftnndbar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNDBNUM, "\\ftnndbnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNDBNUMD, "\\ftnndbnumd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNDBNUMK, "\\ftnndbnumk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNDBNUMT, "\\ftnndbnumt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNGANADA, "\\ftnnganada" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNGBNUM, "\\ftnngbnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNGBNUMD, "\\ftnngbnumd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNGBNUMK, "\\ftnngbnumk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNGBNUML, "\\ftnngbnuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNZODIAC, "\\ftnnzodiac" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNZODIACD, "\\ftnnzodiacd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FTNNZODIACL, "\\ftnnzodiacl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_G, "\\g" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_GCW, "\\gcw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_GRIDTBL, "\\gridtbl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HIGHLIGHT, "\\highlight" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HLFR, "\\hlfr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HLINKBASE, "\\hlinkbase" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HLLOC, "\\hlloc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HLSRC, "\\hlsrc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ILVL, "\\ilvl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_IMPR, "\\impr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_JPEGBLIP, "\\jpegblip" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELFOLLOW, "\\levelfollow" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELINDENT, "\\levelindent" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELJC, "\\leveljc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELLEGAL, "\\levellegal" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELNFC, "\\levelnfc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELNORESTART, "\\levelnorestart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELNUMBERS, "\\levelnumbers" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELOLD, "\\levelold" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELPREV, "\\levelprev" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELPREVSPACE, "\\levelprevspace" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELSPACE, "\\levelspace" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELSTARTAT, "\\levelstartat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LEVELTEXT, "\\leveltext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LINKVAL, "\\linkval" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LIST, "\\list" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTID, "\\listid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTLEVEL, "\\listlevel" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTNAME, "\\listname" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTOVERRIDE, "\\listoverride" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTOVERRIDECOUNT, "\\listoverridecount" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTOVERRIDEFORMAT, "\\listoverrideformat" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTOVERRIDESTART, "\\listoverridestart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTOVERRIDETABLE, "\\listoverridetable" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTRESTARTHDN, "\\listrestarthdn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTSIMPLE, "\\listsimple" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTTABLE, "\\listtable" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTTEMPLATEID, "\\listtemplateid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LISTTEXT, "\\listtext" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LS, "\\ls" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LYTEXCTTP, "\\lytexcttp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LYTPRTMET, "\\lytprtmet" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MANAGER, "\\manager" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_MSMCAP, "\\msmcap" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOFCHARSWS, "\\nofcharsws" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOLEAD, "\\nolead" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NONSHPPICT, "\\nonshppict" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOSECTEXPAND, "\\nosectexpand" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOSNAPLINEGRID, "\\nosnaplinegrid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOSPACEFORUL, "\\nospaceforul" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOULTRLSPC, "\\noultrlspc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_NOXLATTOYEN, "\\noxlattoyen" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJATTPH, "\\objattph" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJHTML, "\\objhtml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OBJOCX, "\\objocx" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OLDLINEWRAP, "\\oldlinewrap" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OUTLINELEVEL, "\\outlinelevel" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_OVERLAY, "\\overlay" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PANOSE, "\\panose" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRDRB, "\\pgbrdrb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRDRFOOT, "\\pgbrdrfoot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRDRHEAD, "\\pgbrdrhead" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRDRL, "\\pgbrdrl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRDROPT, "\\pgbrdropt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRDRR, "\\pgbrdrr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRDRSNAP, "\\pgbrdrsnap" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRDRT, "\\pgbrdrt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNCHOSUNG, "\\pgnchosung" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNCNUM, "\\pgncnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNDBNUMK, "\\pgndbnumk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNDBNUMT, "\\pgndbnumt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNGANADA, "\\pgnganada" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNGBNUM, "\\pgngbnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNGBNUMD, "\\pgngbnumd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNGBNUMK, "\\pgngbnumk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNGBNUML, "\\pgngbnuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNZODIAC, "\\pgnzodiac" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNZODIACD, "\\pgnzodiacd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGNZODIACL, "\\pgnzodiacl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PICPROP, "\\picprop" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNAIUEO, "\\pnaiueo" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNAIUEOD, "\\pnaiueod" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNCHOSUNG, "\\pnchosung" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNDBNUMD, "\\pndbnumd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNDBNUMK, "\\pndbnumk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNDBNUML, "\\pndbnuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNDBNUMT, "\\pndbnumt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNGANADA, "\\pnganada" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNGBLIP, "\\pngblip" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNGBNUM, "\\pngbnum" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNGBNUMD, "\\pngbnumd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNGBNUMK, "\\pngbnumk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNGBNUML, "\\pngbnuml" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRAUTH, "\\pnrauth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRDATE, "\\pnrdate" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRNFC, "\\pnrnfc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRNOT, "\\pnrnot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRPNBR, "\\pnrpnbr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRRGB, "\\pnrrgb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRSTART, "\\pnrstart" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRSTOP, "\\pnrstop" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNRXST, "\\pnrxst" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNZODIAC, "\\pnzodiac" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNZODIACD, "\\pnzodiacd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PNZODIACL, "\\pnzodiacl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LFOLEVEL, "\\lfolevel" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSYIN, "\\posyin" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_POSYOUT, "\\posyout" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PRIVATE, "\\private" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PROPNAME, "\\propname" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PROPTYPE, "\\proptype" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVAUTHDEL, "\\revauthdel" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_REVDTTMDEL, "\\revdttmdel" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SAUTOUPD, "\\sautoupd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECTDEFAULTCL, "\\sectdefaultcl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECTEXPAND, "\\sectexpand" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECTLINEGRID, "\\sectlinegrid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECTSPECIFYCL, "\\sectspecifycl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SECTSPECIFYL, "\\sectspecifyl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHIDDEN, "\\shidden" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPBOTTOM, "\\shpbottom" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPBXCOLUMN, "\\shpbxcolumn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPBXMARGIN, "\\shpbxmargin" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPBXPAGE, "\\shpbxpage" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPBYMARGIN, "\\shpbymargin" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPBYPAGE, "\\shpbypage" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPBYPARA, "\\shpbypara" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPFBLWTXT, "\\shpfblwtxt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPFHDR, "\\shpfhdr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPGRP, "\\shpgrp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPLEFT, "\\shpleft" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPLID, "\\shplid" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPLOCKANCHOR, "\\shplockanchor" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPPICT, "\\shppict" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPRIGHT, "\\shpright" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPRSLT, "\\shprslt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPTOP, "\\shptop" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPTXT, "\\shptxt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPWRK, "\\shpwrk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPWR, "\\shpwr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHPZ, "\\shpz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SPRSBSP, "\\sprsbsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SPRSLNSP, "\\sprslnsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SPRSTSM, "\\sprstsm" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_STATICVAL, "\\staticval" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_STEXTFLOW, "\\stextflow" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_STRIKED, "\\striked" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SUBFONTBYSIZE, "\\subfontbysize" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TCELLD, "\\tcelld" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TIME, "\\time" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TRUNCATEFONTHEIGHT, "\\truncatefontheight" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_UC, "\\uc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_UD, "\\ud" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULDASH, "\\uldash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULDASHD, "\\uldashd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULDASHDD, "\\uldashdd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULTH, "\\ulth" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULWAVE, "\\ulwave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULC, "\\ulc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_U, "\\u" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_UPR, "\\upr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_USERPROPS, "\\userprops" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VIEWKIND, "\\viewkind" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VIEWSCALE, "\\viewscale" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_VIEWZK, "\\viewzk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WIDCTLPAR, "\\widctlpar" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WINDOWCAPTION, "\\windowcaption" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WPEQN, "\\wpeqn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WPJST, "\\wpjst" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_WPSP, "\\wpsp" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_YXE, "\\yxe" );

// MS-2000 Tokens
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULTHD, "\\ulthd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULTHDASH, "\\ulthdash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULLDASH, "\\ulldash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULTHLDASH, "\\ulthldash" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULTHDASHD, "\\ulthdashd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULTHDASHDD, "\\ulthdashdd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULHWAVE, "\\ulhwave" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ULULDBWAVE, "\\ululdbwave" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LOCH,      "\\loch" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HICH,      "\\hich" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DBCH,      "\\dbch" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_LANGFE,    "\\langfe" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ADEFLANG,  "\\adeflang" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ADEFF,     "\\adeff" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ACCNONE,   "\\accnone" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ACCDOT,    "\\accdot" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_ACCCOMMA,  "\\acccomma" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TWOINONE,  "\\twoinone" );

// SWG spezifische Attribute
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_GRFALIGNV, "\\grfalignv" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_GRFALIGNH, "\\grfalignh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_GRFMIRROR, "\\grfmirror" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADERYB, "\\headeryb" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADERXL, "\\headerxl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADERXR, "\\headerxr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTERYT, "\\footeryt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTERXL, "\\footerxl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTERXR, "\\footerxr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HEADERYH, "\\headeryh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FOOTERYH, "\\footeryh" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BALANCEDCOLUMN, "\\swcolmnblnc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_UPDNPROP, "\\updnprop" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PRTDATA, "\\prtdata" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BKMKKEY, "\\bkmkkey" );

// Attribute fuer die freifliegenden Rahmen
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYPRINT, "\\flyprint" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYOPAQUE, "\\flyopaque" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYPRTCTD, "\\flyprtctd" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYMAINCNT, "\\flymaincnt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYVERT, "\\flyvert" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYHORZ, "\\flyhorz" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRMTXTL, "\\dfrmtxtl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRMTXTR, "\\dfrmtxtr" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRMTXTU, "\\dfrmtxtu" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_DFRMTXTW, "\\dfrmtxtw" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYANCHOR, "\\flyanchor" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYCNTNT, "\\flycntnt" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYCOLUMN, "\\flycolumn" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYPAGE, "\\flypage" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_FLYINPARA, "\\flyinpara" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDBOX, "\\brdbox" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDLNCOL, "\\brdlncol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDLNIN, "\\brdlnin" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDLNOUT, "\\brdlnout" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_BRDLNDIST, "\\brdlndist" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHADOW, "\\shadow" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHDWDIST, "\\shdwdist" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHDWSTYLE, "\\shdwstyle" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHDWCOL, "\\shdwcol" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SHDWFCOL, "\\shdwfcol" );


extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGDSCTBL, "\\pgdsctbl" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGDSC, "\\pgdsc" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGDSCUSE, "\\pgdscuse" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGDSCNXT, "\\pgdscnxt" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHEN, "\\hyphen" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHLEAD, "\\hyphlead" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHTRAIL, "\\hyphtrail" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_HYPHMAX, "\\hyphmax" );

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_TLSWG, "\\tlswg" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGBRK, "\\pgbrk" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_PGDSCNO, "\\pgdscno" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sRTF_SOUTLVL, "\\soutlvl" );

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/svtools/source/svrtf/rtfkeywd.hxx,v 1.3 2000-11-16 17:54:11 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/11/10 11:23:04  jp
      new Tokens for the CJK-Version

      Revision 1.1.1.1  2000/09/18 16:59:05  hr
      initial import

      Revision 1.19  2000/09/18 14:13:46  willem.vandorp
      OpenOffice header added.

      Revision 1.18  2000/08/31 13:51:38  willem.vandorp
      Header and footer replaced

      Revision 1.17  2000/03/28 11:47:53  jp
      changes for Unicode

      Revision 1.16  2000/01/07 10:46:14  jp
      Task #71411#: new SW token

      Revision 1.15  1999/11/01 11:27:39  jp
      neue tokens

      Revision 1.14  1999/10/25 14:36:55  jp
      Bug #69464#: new tabstop token tql

      Revision 1.13  1999/07/22 17:44:51  jp
      new RTF-Tokens for Header/Footer height

      Revision 1.12  1999/02/16 11:01:16  JP
      Task #61633#: neue Unter-/Durchstreichungs Werte


      Rev 1.11   16 Feb 1999 12:01:16   JP
   Task #61633#: neue Unter-/Durchstreichungs Werte

      Rev 1.10   27 May 1998 21:22:34   JP
   Bug #50585#: fehlendes Token lfolevel zugefuegt

      Rev 1.9   03 Apr 1998 18:55:20   JP
   RTF-Parser um neue Tokens erweitert, nTokenValue wurde ein long

      Rev 1.8   20 Nov 1997 22:28:02   JP
   HeaderFile Umstellung

      Rev 1.7   08 Oct 1996 13:51:18   JP
   neue RTF-Token zugefuegt

      Rev 1.6   16 Oct 1995 17:01:16   JP
   neue Tokens

      Rev 1.5   29 Sep 1995 20:03:30   JP
   neues Token: kerning

      Rev 1.4   10 Aug 1995 21:02:26   JP
   neue Token fuer Fonts; Segmentierung

      Rev 1.3   29 Jul 1995 22:40:04   JP
   neue Token lesen/auswerten: expand/-tw

      Rev 1.2   05 Jul 1995 13:58:22   JP
   neue Tokens COLSX/COLNO/COLSR

      Rev 1.1   04 Jan 1995 16:20:32   JP
   TokenName und Token korrigiert

      Rev 1.0   22 Dec 1994 17:19:52   JP
   Initial revision.

*************************************************************************/

#endif
    // _RTFKEYWD_HXX

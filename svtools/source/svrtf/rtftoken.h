/*************************************************************************
 *
 *  $RCSfile: rtftoken.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-10 11:23:04 $
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

#ifndef _RTFTOKEN_H
#define _RTFTOKEN_H

// suche die TokenID zu dem Token
int GetRTFToken( const String& rSearch );

enum RTF_TOKEN_RANGES {
    RTF_NOGROUP                     = 0x0100,
    RTF_DOCFMT                      = 0x0200,
    RTF_SECTFMT                     = 0x0300,
    RTF_PARFMT                      = 0x0400,
    RTF_TABSTOPDEF                  = 0x0500,
    RTF_BRDRDEF                     = 0x0600,
    RTF_CHRFMT                      = 0x0700,
    RTF_SPECCHAR                    = 0x0800,
    RTF_APOCTL                      = 0x0900,
    RTF_SHADINGDEF                  = 0x0A00,
    // free                         = 0x0B00,
    RTF_DRAWOBJECTS                 = 0x0C00,
    RTF_OBJECTS                     = 0x0D00,
    RTF_NUMBULLETS                  = 0x0E00,

    // !!! kann hinein verodert werden (Border/Background) !!!!
    RTF_TABLEDEF                    = 0x1000,

    // !!! kann hinein verodert werden (Border/Tab) !!!!
    RTF_SWGDEFS                     = 0x4000
};

enum RTF_TOKEN_IDS {

    RTF_TEXTTOKEN                       = RTF_NOGROUP,
    RTF_SINGLECHAR,
    RTF_UNKNOWNCONTROL,
    RTF_UNKNOWNDATA,

    RTF_RTF,
    RTF_ANSITYPE,
    RTF_MACTYPE,
    RTF_PCTYPE,
    RTF_PCATYPE,
    RTF_NEXTTYPE,

    RTF_STYLESHEET,
    RTF_SBASEDON,
    RTF_SNEXT,
    RTF_SHIDDEN,
    RTF_SAUTOUPD,

    RTF_KEYCODE,
    RTF_FNKEY,
    RTF_ALTKEY,
    RTF_SHIFTKEY,
    RTF_CTRLKEY,

    RTF_FONTTBL,
    RTF_DEFF,
    RTF_FNIL,
    RTF_FROMAN,
    RTF_FSWISS,
    RTF_FMODERN,
    RTF_FSCRIPT,
    RTF_FDECOR,
    RTF_FTECH,

    RTF_COLORTBL,
    RTF_RED,
    RTF_GREEN,
    RTF_BLUE,

    RTF_INFO,
    RTF_TITLE,
    RTF_SUBJECT,
    RTF_AUTHOR,
    RTF_OPERATOR,
    RTF_KEYWORDS,
    RTF_COMMENT,
    RTF_VERSION,
    RTF_DOCCOMM,
    RTF_VERN,
    RTF_CREATIM,
    RTF_REVTIM,
    RTF_PRINTIM,
    RTF_BUPTIM,
    RTF_EDMINS,
    RTF_NOFPAGES,
    RTF_NOFWORDS,
    RTF_NOFCHARS,
    RTF_ID,
    RTF_YR,
    RTF_MO,
    RTF_DY,
    RTF_HR,
    RTF_MIN,
    RTF_USERPROPS,
    RTF_CATEGORY,
    RTF_COMPANY,
    RTF_MANAGER,
    RTF_PROPNAME,
    RTF_LINKVAL,
    RTF_NOFCHARSWS,
    RTF_HLINKBASE,
    RTF_STATICVAL,
    RTF_PROPTYPE,

    RTF_ANNOTATION,
    RTF_ATNID,

    RTF_FOOTNOTE,

    RTF_XE,
    RTF_BXE,
    RTF_IXE,
    RTF_RXE,
    RTF_TXE,
    RTF_YXE,

    RTF_TC,
    RTF_TCF,
    RTF_TCL,

    RTF_BKMKSTART,
    RTF_BKMKEND,

    RTF_PICT,                           // Bitmaps
    RTF_PICW,
    RTF_PICH,
    RTF_WBMBITSPIXEL,
    RTF_WBMPLANES,
    RTF_WBMWIDTHBYTES,
    RTF_PICWGOAL,
    RTF_PICHGOAL,
    RTF_BIN,
    RTF_PICSCALEX,
    RTF_PICSCALEY,
    RTF_PICSCALED,
    RTF_WBITMAP,
    RTF_WMETAFILE,
    RTF_MACPICT,
    RTF_OSMETAFILE,
    RTF_DIBITMAP,
    RTF_PICCROPT,
    RTF_PICCROPB,
    RTF_PICCROPL,
    RTF_PICCROPR,
    RTF_PICDATA,
    RTF_PICBMP,
    RTF_PICBPP,
    RTF_PICPROP,
    RTF_SHPPICT,
    RTF_EMFBLIP,
    RTF_PNGBLIP,
    RTF_JPEGBLIP,
    RTF_NONSHPPICT,
    RTF_BLIPTAG,
    RTF_BLIPUID,
    RTF_BLIPUPI,

    RTF_FIELD,                      // Felder
    RTF_FLDDIRTY,
    RTF_FLDEDIT,
    RTF_FLDLOCK,
    RTF_FLDPRIV,
    RTF_FLDINST,
    RTF_FLDRSLT,
    RTF_FLDTYPE,
    RTF_TIME,
    RTF_DATE,
    RTF_WPEQN,

    RTF_NOLINE,
    RTF_REVISED,

    RTF_BKMKCOLF,
    RTF_BKMKCOLL,
    RTF_PSOVER,
    RTF_DOCTEMP,
    RTF_DELETED,

    RTF_FCHARSET,
    RTF_FALT,
    RTF_FPRQ,
    RTF_PANOSE,
    RTF_FNAME,
    RTF_FBIAS,

    RTF_ADDITIVE,
    RTF_BKMKPUB,
    RTF_CPG,
    RTF_DATAFIELD,

    RTF_FBIDI,
    RTF_FID,
    RTF_FILE,
    RTF_FILETBL,
    RTF_FNETWORK,
    RTF_FONTEMB,
    RTF_FONTFILE,
    RTF_FOSNUM,
    RTF_FRELATIVE,
    RTF_FTNIL,
    RTF_FTTRUETYPE,
    RTF_FVALIDDOS,
    RTF_FVALIDHPFS,
    RTF_FVALIDMAC,
    RTF_FVALIDNTFS,
    RTF_LINKSELF,
    RTF_PUBAUTO,
    RTF_REVTBL,
    RTF_RTLMARK,
    RTF_SEC,
    RTF_TCN,
    RTF_XEF,

    RTF_UD,                 // Unicode
    RTF_UPR,
    RTF_U,
    RTF_UC,
    RTF_ANSICPG,

    RTF_FFEXITMCR,          // Form Fields
    RTF_FFENTRYMCR,
    RTF_FFDEFTEXT,
    RTF_FFFORMAT,
    RTF_FFSTATTEXT,
    RTF_FORMFIELD,
    RTF_FFNAME,
    RTF_FFHELPTEXT,
    RTF_FFL,
    RTF_FFOWNHELP,
    RTF_FFOWNSTAT,
    RTF_FFMAXLEN,
    RTF_FFHASLISTBOX,
    RTF_FFHPS,
    RTF_FFPROT,
    RTF_FFTYPE,
    RTF_FFTYPETXT,
    RTF_FFSIZE,
    RTF_FFRECALC,
    RTF_FFRES,
    RTF_FFDEFRES,

    RTF_HIGHLIGHT,

/*  */

    RTF_DEFTAB          = RTF_DOCFMT,
    RTF_HYPHHOTZ,
    RTF_LINESTART,
    RTF_FRACWIDTH,
    RTF_NEXTFILE,
    RTF_TEMPLATE,
    RTF_MAKEBACKUP,
    RTF_DEFFORMAT,
    RTF_DEFLANG,
    RTF_FTNSEP,
    RTF_FTNSEPC,
    RTF_FTNCN,
    RTF_ENDNOTES,
    RTF_ENDDOC,
    RTF_FTNTJ,
    RTF_FTNBJ,
    RTF_FTNSTART,
    RTF_FTNRESTART,
    RTF_PAPERW,
    RTF_PAPERH,
    RTF_MARGL,
    RTF_MARGR,
    RTF_MARGT,
    RTF_MARGB,
    RTF_FACINGP,
    RTF_GUTTER,
    RTF_MARGMIRROR,
    RTF_LANDSCAPE,
    RTF_PGNSTART,
    RTF_WIDOWCTRL,
    RTF_REVISIONS,
    RTF_REVPROP,
    RTF_REVBAR,

    RTF_AENDDOC,
    RTF_AENDNOTES,
    RTF_AFTNBJ,
    RTF_AFTNCN,
    RTF_AFTNNALC,
    RTF_AFTNNAR,
    RTF_AFTNNAUC,
    RTF_AFTNNCHI,
    RTF_AFTNNRLC,
    RTF_AFTNNRUC,
    RTF_AFTNRESTART,
    RTF_AFTNRSTCONT,
    RTF_AFTNSEP,
    RTF_AFTNSEPC,
    RTF_AFTNSTART,
    RTF_AFTNTJ,
    RTF_ALLPROT,
    RTF_ANNOTPROT,
    RTF_ATNAUTHOR,
    RTF_ATNICN,
    RTF_ATNREF,
    RTF_ATNTIME,
    RTF_ATRFEND,
    RTF_ATRFSTART,
    RTF_BRKFRM,
    RTF_CVMME,
    RTF_FET,
    RTF_FLDALT,
    RTF_FORMDISP,
    RTF_FORMPROT,
    RTF_FORMSHADE,
    RTF_FTNALT,
    RTF_FTNNALC,
    RTF_FTNNAR,
    RTF_FTNNAUC,
    RTF_FTNNCHI,
    RTF_FTNNRLC,
    RTF_FTNNRUC,
    RTF_FTNRSTCONT,
    RTF_FTNRSTPG,
    RTF_HYPHAUTO,
    RTF_HYPHCAPS,
    RTF_HYPHCONSEC,
    RTF_LINKSTYLES,
    RTF_LTRDOC,
    RTF_NOCOLBAL,
    RTF_NOEXTRASPRL,
    RTF_NOTABIND,
    RTF_OTBLRUL,
    RTF_PRCOLBL,
    RTF_PRINTDATA,
    RTF_PSZ,
    RTF_REVPROT,
    RTF_RTLDOC,
    RTF_SPRSSPBF,
    RTF_SPRSTSP,
    RTF_SWPBDR,
    RTF_TRANSMF,
    RTF_WRAPTRSP,

    RTF_PRIVATE,
    RTF_NOULTRLSPC,
    RTF_MSMCAP,
    RTF_NOLEAD,
    RTF_NOSPACEFORUL,
    RTF_LYTEXCTTP,
    RTF_LYTPRTMET,
    RTF_DNTBLNSBDB,
    RTF_FROMTEXT,
    RTF_EXPSHRTN,
    RTF_PGBRDRT,
    RTF_SPRSBSP,
    RTF_PGBRDRR,
    RTF_PGBRDRSNAP,
    RTF_BDBFHDR,
    RTF_SUBFONTBYSIZE,
    RTF_TRUNCATEFONTHEIGHT,
    RTF_SPRSLNSP,
    RTF_SPRSTSM,
    RTF_PGBRDRL,
    RTF_WPJST,
    RTF_PGBRDRB,
    RTF_WPSP,
    RTF_NOXLATTOYEN,
    RTF_OLDLINEWRAP,
    RTF_PGBRDRFOOT,
    RTF_PGBRDRHEAD,
    RTF_DEFLANGFE,
    RTF_DOCTYPE,
    RTF_PGBRDROPT,
    RTF_VIEWKIND,
    RTF_VIEWSCALE,
    RTF_WINDOWCAPTION,
    RTF_BRDRART,
    RTF_VIEWZK,
    RTF_DOCVAR,

    RTF_DGMARGIN,
    RTF_AFTNNCHOSUNG,
    RTF_AFTNNCNUM,
    RTF_AFTNNDBAR,
    RTF_AFTNNDBNUM,
    RTF_AFTNNDBNUMD,
    RTF_AFTNNDBNUMK,
    RTF_AFTNNDBNUMT,
    RTF_AFTNNGANADA,
    RTF_AFTNNGBNUM,
    RTF_AFTNNGBNUMD,
    RTF_AFTNNGBNUMK,
    RTF_AFTNNGBNUML,
    RTF_AFTNNZODIAC,
    RTF_AFTNNZODIACD,
    RTF_AFTNNZODIACL,
    RTF_FTNNCHOSUNG,
    RTF_FTNNCNUM,
    RTF_FTNNDBAR,
    RTF_FTNNDBNUM,
    RTF_FTNNDBNUMD,
    RTF_FTNNDBNUMK,
    RTF_FTNNDBNUMT,
    RTF_FTNNGANADA,
    RTF_FTNNGBNUM,
    RTF_FTNNGBNUMD,
    RTF_FTNNGBNUMK,
    RTF_FTNNGBNUML,
    RTF_FTNNZODIAC,
    RTF_FTNNZODIACD,
    RTF_FTNNZODIACL,

    RTF_ADEFLANG,
    RTF_ADEFF,

/*  */

    RTF_SECTD               = RTF_SECTFMT,
    RTF_ENDNHERE,
    RTF_BINFSXN,
    RTF_BINSXN,
    RTF_SBKNONE,
    RTF_SBKCOL,
    RTF_SBKPAGE,
    RTF_SBKEVEN,
    RTF_SBKODD,
    RTF_COLS,
    RTF_COLSX,
    RTF_COLNO,
    RTF_COLSR,
    RTF_COLW,
    RTF_LINEBETCOL,
    RTF_LINEMOD,
    RTF_LINEX,
    RTF_LINESTARTS,
    RTF_LINERESTART,
    RTF_LINEPAGE,
    RTF_LINECONT,
    RTF_PGWSXN,
    RTF_PGHSXN,
    RTF_MARGLSXN,
    RTF_MARGRSXN,
    RTF_MARGTSXN,
    RTF_MARGBSXN,
    RTF_GUTTERSXN,
    RTF_LNDSCPSXN,
    RTF_FACPGSXN,
    RTF_TITLEPG,
    RTF_HEADERY,
    RTF_FOOTERY,
    RTF_PGNSTARTS,
    RTF_PGNCONT,
    RTF_PGNRESTART,
    RTF_PGNX,
    RTF_PGNY,
    RTF_PGNDEC,
    RTF_PGNUCRM,
    RTF_PGNLCRM,
    RTF_PGNUCLTR,
    RTF_PGNLCLTR,
    RTF_VERTALT,
    RTF_VERTALB,
    RTF_VERTALC,
    RTF_VERTALJ,

    RTF_FOOTER,
    RTF_FOOTERL,
    RTF_FOOTERR,
    RTF_FOOTERF,
    RTF_HEADER,
    RTF_HEADERL,
    RTF_HEADERR,
    RTF_HEADERF,
    RTF_DS,
    RTF_LTRSECT,
    RTF_PGNHN,
    RTF_PGNHNSC,
    RTF_PGNHNSH,
    RTF_PGNHNSM,
    RTF_PGNHNSN,
    RTF_PGNHNSP,
    RTF_RTLSECT,
    RTF_SECTUNLOCKED,
    RTF_STEXTFLOW,
    RTF_PGNCHOSUNG,
    RTF_PGNCNUM,
    RTF_PGNDBNUMK,
    RTF_PGNDBNUMT,
    RTF_PGNGANADA,
    RTF_PGNGBNUM,
    RTF_PGNGBNUMD,
    RTF_PGNGBNUMK,
    RTF_PGNGBNUML,
    RTF_PGNZODIAC,
    RTF_PGNZODIACD,
    RTF_PGNZODIACL,
    RTF_SECTDEFAULTCL,
    RTF_SECTEXPAND,
    RTF_SECTLINEGRID,
    RTF_SECTSPECIFYCL,
    RTF_SECTSPECIFYL,

    // Swg-Header/Footer-Tokens
    RTF_HEADER_YB   = (RTF_SECTFMT|RTF_SWGDEFS),
    RTF_HEADER_XL,
    RTF_HEADER_XR,
    RTF_FOOTER_YT,
    RTF_FOOTER_XL,
    RTF_FOOTER_XR,
    RTF_HEADER_YH,
    RTF_FOOTER_YH,
    RTF_BALANCED_COLUMN,


/*  */

    RTF_PARD                = RTF_PARFMT,
    RTF_S,
    RTF_INTBL,
    RTF_KEEP,
    RTF_KEEPN,
    RTF_LEVEL,
    RTF_PAGEBB,
    RTF_SBYS,
    RTF_QL,
    RTF_QR,
    RTF_QJ,
    RTF_QC,
    RTF_FI,
    RTF_LI,
    RTF_RI,
    RTF_SB,
    RTF_SA,
    RTF_SL,
    RTF_HYPHPAR,
    RTF_LTRPAR,
    RTF_NOWIDCTLPAR,
    RTF_RTLPAR,
    RTF_SLMULT,
    RTF_SUBDOCUMENT,

    RTF_WIDCTLPAR,

    RTF_LISTTEXT,
    RTF_POSYIN,
    RTF_PNRNOT,
    RTF_BRDRDASHDOTSTR,
    RTF_POSYOUT,
    RTF_BRDRDASHD,
    RTF_BRDRDASHDD,
    RTF_BRDRENGRAVE,
    RTF_BRDRTHTNLG,
    RTF_BRDREMBOSS,
    RTF_BRDRTNTHTNLG,
    RTF_BRDRDASHSM,
    RTF_BRDRTHTNMG,
    RTF_OVERLAY,
    RTF_BRDRTNTHSG,
    RTF_BRDRTNTHMG,
    RTF_BRDRTHTNSG,
    RTF_BRDRTNTHLG,
    RTF_BRDRTRIPLE,
    RTF_BRDRTNTHTNSG,
    RTF_BRDRTNTHTNMG,
    RTF_BRDRWAVYDB,
    RTF_BRDRWAVY,
    RTF_ILVL,
    RTF_DFRSTOP,
    RTF_DFRXST,
    RTF_PNRAUTH,
    RTF_DFRSTART,
    RTF_OUTLINELEVEL,
    RTF_DFRAUTH,
    RTF_DFRDATE,
    RTF_PNRRGB,
    RTF_PNRPNBR,
    RTF_PNRSTART,
    RTF_PNRXST,
    RTF_PNRSTOP,
    RTF_PNRDATE,
    RTF_PNRNFC,
    RTF_NOSNAPLINEGRID,
    RTF_FAAUTO,

    RTF_ADJUSTRIGHT,
    RTF_LS,


/*  */

    RTF_TX                  = RTF_TABSTOPDEF,
    RTF_TB,
    RTF_TQL,
    RTF_TQR,
    RTF_TQC,
    RTF_TQDEC,
    RTF_TLDOT,
    RTF_TLHYPH,
    RTF_TLUL,
    RTF_TLTH,
    RTF_TLEQ,

    // Swg-TabStop-Tokens
    RTF_TLSWG = (RTF_TABSTOPDEF|RTF_SWGDEFS),

/*  */

    RTF_BRDRT       = RTF_BRDRDEF,
    RTF_BRDRB,
    RTF_BRDRL,
    RTF_BRDRR,
    RTF_BRDRBTW,
    RTF_BRDRBAR,
    RTF_BOX,
    RTF_BRSP,
    RTF_BRDRW,
    RTF_BRDRCF,
    RTF_BRDRS,
    RTF_BRDRTH,
    RTF_BRDRSH,
    RTF_BRDRDB,
    RTF_BRDRDOT,
    RTF_BRDRHAIR,
    RTF_BRDRDASH,
    RTF_BRDRFRAME,

    // Swg-Border-Tokens
    RTF_BRDBOX          = (RTF_BRDRDEF|RTF_SWGDEFS),
    RTF_BRDLINE_COL,
    RTF_BRDLINE_IN,
    RTF_BRDLINE_OUT,
    RTF_BRDLINE_DIST,

/*  */

    RTF_PLAIN       = RTF_CHRFMT,
    RTF_B,
    RTF_CAPS,
    RTF_DN,
    RTF_SUB,
    RTF_NOSUPERSUB,
    RTF_EXPND,
    RTF_EXPNDTW,
    RTF_KERNING,
    RTF_F,
    RTF_FS,
    RTF_I,
    RTF_OUTL,
    RTF_SCAPS,
    RTF_SHAD,
    RTF_STRIKE,
    RTF_UL,
    RTF_ULD,
    RTF_ULDB,
    RTF_ULNONE,
    RTF_ULW,
    RTF_UP,
    RTF_SUPER,
    RTF_V,
    RTF_CF,
    RTF_CB,
    RTF_LANG,
    RTF_CCHS,
    RTF_CS,
    RTF_LTRCH,
    RTF_REVAUTH,
    RTF_REVDTTM,
    RTF_RTLCH,

    RTF_CHBGFDIAG,
    RTF_CHBGDKVERT,
    RTF_CHBGDKHORIZ,
    RTF_CHBRDR,
    RTF_CHBGVERT,
    RTF_CHBGHORIZ,
    RTF_CHBGDKFDIAG,
    RTF_CHBGDCROSS,
    RTF_CHBGCROSS,
    RTF_CHBGBDIAG,
    RTF_CHBGDKDCROSS,
    RTF_CHBGDKCROSS,
    RTF_CHBGDKBDIAG,
    RTF_ULDASHD,
    RTF_ULDASH,
    RTF_ULDASHDD,
    RTF_ULWAVE,
    RTF_ULTH,
    RTF_EMBO,
    RTF_IMPR,
    RTF_STRIKED,
    RTF_CRDATE,
    RTF_CRAUTH,
    RTF_CHARSCALEX,
    RTF_CHCBPAT,
    RTF_CHCFPAT,
    RTF_CHSHDNG,
    RTF_REVAUTHDEL,
    RTF_REVDTTMDEL,
    RTF_CGRID,
    RTF_GCW,
    RTF_NOSECTEXPAND,
    RTF_GRIDTBL,
    RTF_G,
    RTF_ANIMTEXT,
    RTF_ULTHD,
    RTF_ULTHDASH,
    RTF_ULLDASH,
    RTF_ULTHLDASH,
    RTF_ULTHDASHD,
    RTF_ULTHDASHDD,
    RTF_ULHWAVE,
    RTF_ULULDBWAVE,

    // association control words
    RTF_AB,
    RTF_ACAPS,
    RTF_ACF,
    RTF_ADN,
    RTF_AEXPND,
    RTF_AF,
    RTF_AFS,
    RTF_AI,
    RTF_ALANG,
    RTF_AOUTL,
    RTF_ASCAPS,
    RTF_ASHAD,
    RTF_ASTRIKE,
    RTF_AUL,
    RTF_AULD,
    RTF_AULDB,
    RTF_AULNONE,
    RTF_AULW,
    RTF_AUP,

    RTF_LOCH,
    RTF_HICH,
    RTF_DBCH,
    RTF_LANGFE,
    RTF_ACCNONE,
    RTF_ACCDOT,
    RTF_ACCCOMMA,

    // Swg-Border-Tokens
    RTF_SWG_ESCPROP     = (RTF_CHRFMT|RTF_SWGDEFS),
    RTF_HYPHEN,
    RTF_HYPHLEAD,
    RTF_HYPHTRAIL,
    RTF_HYPHMAX,


/*  */

    RTF_CHDATE              = RTF_SPECCHAR,
    RTF_CHDATEL,
    RTF_CHDATEA,
    RTF_CHTIME,
    RTF_CHPGN,
    RTF_CHFTN,
    RTF_CHATN,
    RTF_CHFTNSEP,
    RTF_CHFTNSEPC,
    RTF_CELL,
    RTF_ROW,
    RTF_PAR,
    RTF_SECT,
    RTF_PAGE,
    RTF_COLUM,
    RTF_LINE,
    RTF_TAB,
    RTF_EMDASH,
    RTF_ENDASH,
    RTF_BULLET,
    RTF_LQUOTE,
    RTF_RQUOTE,
    RTF_LDBLQUOTE,
    RTF_RDBLQUOTE,
    RTF_FORMULA,
    RTF_NONBREAKINGSPACE,
    RTF_OPTIONALHYPHEN,
    RTF_NONBREAKINGHYPHEN,
    RTF_SUBENTRYINDEX,
    RTF_IGNOREFLAG,
    RTF_HEX,
    RTF_EMSPACE,
    RTF_ENSPACE,
    RTF_LTRMARK,
    RTF_SECTNUM,
    RTF_SOFTCOL,
    RTF_SOFTLHEIGHT,
    RTF_SOFTLINE,
    RTF_SOFTPAGE,
    RTF_ZWJ,
    RTF_ZWNJ,

/*  */

    RTF_ABSW = RTF_APOCTL,
    RTF_ABSH,
    RTF_NOWRAP,
    RTF_DXFRTEXT,
    RTF_DFRMTXTX,
    RTF_DFRMTXTY,
    RTF_DROPCAPLI,
    RTF_DROPCAPLT,
    RTF_PHMRG,
    RTF_PHPG,
    RTF_PHCOL,
    RTF_POSX,
    RTF_POSNEGX,
    RTF_POSXC,
    RTF_POSXI,
    RTF_POSXO,
    RTF_POSXL,
    RTF_POSXR,
    RTF_PVMRG,
    RTF_PVPG,
    RTF_PVPARA,
    RTF_POSY,
    RTF_POSNEGY,
    RTF_POSYT,
    RTF_POSYIL,
    RTF_POSYB,
    RTF_POSYC,
    RTF_ABSLOCK,

    // Swg-Frame-Tokens
    RTF_FLYPRINT = (RTF_APOCTL|RTF_SWGDEFS),
    RTF_FLYOPAQUE,
    RTF_FLYPRTCTD,
    RTF_FLYMAINCNT,
    RTF_FLYVERT,
    RTF_FLYHORZ,
    RTF_FLYOUTLEFT,
    RTF_FLYOUTRIGHT,
    RTF_FLYOUTUPPER,
    RTF_FLYOUTLOWER,
    RTF_FLYANCHOR,
    RTF_FLY_CNTNT,
    RTF_FLY_COLUMN,
    RTF_FLY_PAGE,
    RTF_FLY_INPARA,


/*  */

    RTF_SHADING     = RTF_SHADINGDEF,
    RTF_CFPAT,
    RTF_CBPAT,
    RTF_BGHORIZ,
    RTF_BGVERT,
    RTF_BGFDIAG,
    RTF_BGBDIAG,
    RTF_BGCROSS,
    RTF_BGDCROSS,
    RTF_BGDKHORIZ,
    RTF_BGDKVERT,
    RTF_BGDKFDIAG,
    RTF_BGDKBDIAG,
    RTF_BGDKCROSS,
    RTF_BGDKDCROSS,

/*  */

    RTF_TROWD           = RTF_TABLEDEF,
    RTF_TRGAPH,
    RTF_TRLEFT,
    RTF_TRRH,

    RTF_TRQL,
    RTF_TRQR,
    RTF_TRQC,

    RTF_CLMGF,
    RTF_CLMRG,
    RTF_CELLX,
    RTF_LTRROW,
    RTF_RTLROW,
    RTF_TRBRDRB,
    RTF_TRBRDRH,
    RTF_TRBRDRL,
    RTF_TRBRDRR,
    RTF_TRBRDRT,
    RTF_TRBRDRV,
    RTF_TRHDR,
    RTF_TRKEEP,
    RTF_TCELLD,
    RTF_CLTXTBRL,
    RTF_CLTXLRTB,
    RTF_CLVERTALB,
    RTF_CLVERTALT,
    RTF_CLVERTALC,
    RTF_CLVMGF,
    RTF_CLVMRG,
    RTF_CLTXTBRLV,
    RTF_CLTXBTLR,
    RTF_CLTXLRTBV,

    RTF_CLBRDRT = (RTF_BRDRDEF|RTF_TABLEDEF),
    RTF_CLBRDRL,
    RTF_CLBRDRB,
    RTF_CLBRDRR,

    RTF_CLCFPAT = (RTF_SHADINGDEF|RTF_TABLEDEF),
    RTF_CLCBPAT,
    RTF_CLSHDNG,
    RTF_CLBGHORIZ,
    RTF_CLBGVERT,
    RTF_CLBGFDIAG,
    RTF_CLBGBDIAG,
    RTF_CLBGCROSS,
    RTF_CLBGDCROSS,
    RTF_CLBGDKHOR,
    RTF_CLBGDKVERT,
    RTF_CLBGDKFDIAG,
    RTF_CLBGDKBDIAG,
    RTF_CLBGDKCROSS,
    RTF_CLBGDKDCROSS,

/*  */


/*  */

    RTF_DO = RTF_DRAWOBJECTS,
    RTF_DOBXCOLUMN,
    RTF_DOBXMARGIN,
    RTF_DOBXPAGE,
    RTF_DOBYMARGIN,
    RTF_DOBYPAGE,
    RTF_DOBYPARA,
    RTF_DODHGT,
    RTF_DOLOCK,
    RTF_DPAENDHOL,
    RTF_DPAENDL,
    RTF_DPAENDSOL,
    RTF_DPAENDW,
    RTF_DPARC,
    RTF_DPARCFLIPX,
    RTF_DPARCFLIPY,
    RTF_DPASTARTHOL,
    RTF_DPASTARTL,
    RTF_DPASTARTSOL,
    RTF_DPASTARTW,
    RTF_DPCALLOUT,
    RTF_DPCOA,
    RTF_DPCOACCENT,
    RTF_DPCOBESTFIT,
    RTF_DPCOBORDER,
    RTF_DPCODABS,
    RTF_DPCODBOTTOM,
    RTF_DPCODCENTER,
    RTF_DPCODTOP,
    RTF_DPCOLENGTH,
    RTF_DPCOMINUSX,
    RTF_DPCOMINUSY,
    RTF_DPCOOFFSET,
    RTF_DPCOSMARTA,
    RTF_DPCOTDOUBLE,
    RTF_DPCOTRIGHT,
    RTF_DPCOTSINGLE,
    RTF_DPCOTTRIPLE,
    RTF_DPCOUNT,
    RTF_DPELLIPSE,
    RTF_DPENDGROUP,
    RTF_DPFILLBGCB,
    RTF_DPFILLBGCG,
    RTF_DPFILLBGCR,
    RTF_DPFILLBGGRAY,
    RTF_DPFILLBGPAL,
    RTF_DPFILLFGCB,
    RTF_DPFILLFGCG,
    RTF_DPFILLFGCR,
    RTF_DPFILLFGGRAY,
    RTF_DPFILLFGPAL,
    RTF_DPFILLPAT,
    RTF_DPGROUP,
    RTF_DPLINE,
    RTF_DPLINECOB,
    RTF_DPLINECOG,
    RTF_DPLINECOR,
    RTF_DPLINEDADO,
    RTF_DPLINEDADODO,
    RTF_DPLINEDASH,
    RTF_DPLINEDOT,
    RTF_DPLINEGRAY,
    RTF_DPLINEHOLLOW,
    RTF_DPLINEPAL,
    RTF_DPLINESOLID,
    RTF_DPLINEW,
    RTF_DPPOLYCOUNT,
    RTF_DPPOLYGON,
    RTF_DPPOLYLINE,
    RTF_DPPTX,
    RTF_DPPTY,
    RTF_DPRECT,
    RTF_DPROUNDR,
    RTF_DPSHADOW,
    RTF_DPSHADX,
    RTF_DPSHADY,
    RTF_DPTXBX,
    RTF_DPTXBXMAR,
    RTF_DPTXBXTEXT,
    RTF_DPX,
    RTF_DPXSIZE,
    RTF_DPY,
    RTF_DPYSIZE,

    RTF_DPCODESCENT,
    RTF_BACKGROUND,
    RTF_SHPBYPAGE,
    RTF_SHPBYPARA,
    RTF_SHPBYMARGIN,
    RTF_SHPBXCOLUMN,
    RTF_SHPBXMARGIN,
    RTF_SHPBXPAGE,
    RTF_SHPLOCKANCHOR,
    RTF_SHPWR,
    RTF_HLLOC,
    RTF_HLSRC,
    RTF_SHPWRK,
    RTF_SHPTOP,
    RTF_SHPRSLT,
    RTF_HLFR,
    RTF_SHPTXT,
    RTF_SHPFHDR,
    RTF_SHPGRP,
    RTF_SHPRIGHT,
    RTF_SHPFBLWTXT,
    RTF_SHPZ,
    RTF_SHPBOTTOM,
    RTF_SHPLEFT,
    RTF_SHPLID,

/*  */

    RTF_OBJALIAS = RTF_OBJECTS,
    RTF_OBJALIGN,
    RTF_OBJAUTLINK,
    RTF_OBJCLASS,
    RTF_OBJCROPB,
    RTF_OBJCROPL,
    RTF_OBJCROPR,
    RTF_OBJCROPT,
    RTF_OBJDATA,
    RTF_OBJECT,
    RTF_OBJEMB,
    RTF_OBJH,
    RTF_OBJICEMB,
    RTF_OBJLINK,
    RTF_OBJLOCK,
    RTF_OBJNAME,
    RTF_OBJPUB,
    RTF_OBJSCALEX,
    RTF_OBJSCALEY,
    RTF_OBJSECT,
    RTF_OBJSETSIZE,
    RTF_OBJSUB,
    RTF_OBJTIME,
    RTF_OBJTRANSY,
    RTF_OBJUPDATE,
    RTF_OBJW,
    RTF_RESULT,
    RTF_RSLTBMP,
    RTF_RSLTMERGE,
    RTF_RSLTPICT,
    RTF_RSLTRTF,
    RTF_RSLTTXT,
    RTF_OBJOCX,
    RTF_OBJHTML,
    RTF_OBJATTPH,

/*  */

    RTF_PN = RTF_NUMBULLETS,
    RTF_PNACROSS,
    RTF_PNB,
    RTF_PNCAPS,
    RTF_PNCARD,
    RTF_PNCF,
    RTF_PNDEC,
    RTF_PNF,
    RTF_PNFS,
    RTF_PNHANG,
    RTF_PNI,
    RTF_PNINDENT,
    RTF_PNLCLTR,
    RTF_PNLCRM,
    RTF_PNLVL,
    RTF_PNLVLBLT,
    RTF_PNLVLBODY,
    RTF_PNLVLCONT,
    RTF_PNNUMONCE,
    RTF_PNORD,
    RTF_PNORDT,
    RTF_PNPREV,
    RTF_PNQC,
    RTF_PNQL,
    RTF_PNQR,
    RTF_PNRESTART,
    RTF_PNSCAPS,
    RTF_PNSECLVL,
    RTF_PNSP,
    RTF_PNSTART,
    RTF_PNSTRIKE,
    RTF_PNTEXT,
    RTF_PNTXTA,
    RTF_PNTXTB,
    RTF_PNUCLTR,
    RTF_PNUCRM,
    RTF_PNUL,
    RTF_PNULD,
    RTF_PNULDB,
    RTF_PNULNONE,
    RTF_PNULW,
    RTF_LIST,
    RTF_LISTLEVEL,
    RTF_LISTOVERRIDE,
    RTF_LISTOVERRIDETABLE,
    RTF_LISTTABLE,
    RTF_LISTNAME,
    RTF_LEVELNUMBERS,
    RTF_LEVELNORESTART,
    RTF_LEVELNFC,
    RTF_LEVELOLD,
    RTF_LISTOVERRIDECOUNT,
    RTF_LISTTEMPLATEID,
    RTF_LEVELINDENT,
    RTF_LEVELFOLLOW,
    RTF_LEVELLEGAL,
    RTF_LEVELJC,
    RTF_LISTOVERRIDESTART,
    RTF_LISTID,
    RTF_LISTRESTARTHDN,
    RTF_LEVELTEXT,
    RTF_LISTOVERRIDEFORMAT,
    RTF_LEVELPREVSPACE,
    RTF_LEVELPREV,
    RTF_LEVELSPACE,
    RTF_LISTSIMPLE,
    RTF_LEVELSTARTAT,
    RTF_PNAIUEO,
    RTF_PNAIUEOD,
    RTF_PNCHOSUNG,
    RTF_PNDBNUMD,
    RTF_PNDBNUMK,
    RTF_PNDBNUML,
    RTF_PNDBNUMT,
    RTF_PNGANADA,
    RTF_PNGBNUM,
    RTF_PNGBNUMD,
    RTF_PNGBNUMK,
    RTF_PNGBNUML,
    RTF_PNZODIAC,
    RTF_PNZODIACD,
    RTF_PNZODIACL,
    RTF_LFOLEVEL,

/*  */

    RTF_GRF_ALIGNV= RTF_SWGDEFS,
    RTF_GRF_ALIGNH,
    RTF_GRF_MIRROR,
    RTF_SWG_PRTDATA,
    RTF_BKMK_KEY,
    RTF_SHADOW,
    RTF_SHDW_DIST,
    RTF_SHDW_STYLE,
    RTF_SHDW_COL,
    RTF_SHDW_FCOL,
    RTF_PGDSCTBL,
    RTF_PGDSC,
    RTF_PGDSCUSE,
    RTF_PGDSCNXT,
    RTF_PGDSCNO,
    RTF_PGBRK,
    RTF_SOUTLVL

};



/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/svtools/source/svrtf/rtftoken.h,v 1.2 2000-11-10 11:23:04 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 16:59:05  hr
      initial import

      Revision 1.20  2000/09/18 14:13:47  willem.vandorp
      OpenOffice header added.

      Revision 1.19  2000/08/31 13:51:38  willem.vandorp
      Header and footer replaced

      Revision 1.18  2000/03/28 11:47:56  jp
      changes for Unicode

      Revision 1.17  2000/01/07 10:46:18  jp
      Task #71411#: new SW token

      Revision 1.16  1999/11/01 11:27:43  jp
      neue tokens

      Revision 1.15  1999/10/25 14:36:59  jp
      Bug #69464#: new tabstop token tql

      Revision 1.14  1999/07/22 17:44:52  jp
      new RTF-Tokens for Header/Footer height

      Revision 1.13  1999/02/16 11:01:18  JP
      Task #61633#: neue Unter-/Durchstreichungs Werte


      Rev 1.12   16 Feb 1999 12:01:18   JP
   Task #61633#: neue Unter-/Durchstreichungs Werte

      Rev 1.11   27 May 1998 22:15:48   JP
   vertikales Alignment in Zellen ist ein Tabellen- und kein Absatztoken

      Rev 1.10   27 May 1998 21:22:32   JP
   Bug #50585#: fehlendes Token lfolevel zugefuegt

      Rev 1.9   03 Apr 1998 18:55:22   JP
   RTF-Parser um neue Tokens erweitert, nTokenValue wurde ein long

      Rev 1.8   08 Oct 1996 13:51:20   JP
   neue RTF-Token zugefuegt

      Rev 1.7   16 Oct 1995 17:01:16   JP
   neue Tokens

      Rev 1.6   29 Sep 1995 20:03:40   JP
   neues Token: kerning

      Rev 1.5   10 Aug 1995 21:02:28   JP
   neue Token fuer Fonts; Segmentierung

      Rev 1.4   29 Jul 1995 22:40:06   JP
   neue Token lesen/auswerten: expand/-tw

      Rev 1.3   05 Jul 1995 13:41:18   JP
   neue Tokens COLSX/COLNO/COLSR

      Rev 1.2   04 Jan 1995 16:19:46   JP
   Tokengruppierung fuer Tabellen

      Rev 1.1   23 Dec 1994 15:01:52   JP
   jetzt auch mit BrushItem

      Rev 1.0   22 Dec 1994 17:19:52   JP
   Initial revision.

*************************************************************************/


#endif
    // _RTFTOKEN_H

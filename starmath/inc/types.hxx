/*************************************************************************
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:25 $
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
#ifndef TYPES_HXX
#define TYPES_HXX


////////////////////////////////////////
// enum definitions for characters from the 'StarMath' font
// (some chars have more than one alias!)
//! Note: not listed here does not(!) mean "not used"
//!     (see %alpha ... %gamma for example)
//
enum MathSymbol
{
    MS_FACT         = (xub_Unicode) 0xF021,
    MS_INFINITY     = (xub_Unicode) 0xF027,
    MS_SLASH        = (xub_Unicode) 0xF02F,

    MS_NDIVIDES     = (xub_Unicode) 0xF030,
    MS_DRARROW      = (xub_Unicode) 0xF031,
    MS_DLARROW      = (xub_Unicode) 0xF032,
    MS_DLRARROW     = (xub_Unicode) 0xF033,
    MS_UNDERBRACE   = (xub_Unicode) 0xF034,
    MS_OVERBRACE    = (xub_Unicode) 0xF035,
    MS_CIRC         = (xub_Unicode) 0xF036,
    MS_ASSIGN       = (xub_Unicode) 0xF03D,
    MS_ERROR        = (xub_Unicode) 0xF03F,

    MS_NEQ          = (xub_Unicode) 0xF040,
    MS_PLUS         = (xub_Unicode) 0xF041,
    MS_MINUS        = (xub_Unicode) 0xF042,
    MS_MULTIPLY     = (xub_Unicode) 0xF043,
    MS_TIMES        = (xub_Unicode) 0xF044,
    MS_CDOT         = (xub_Unicode) 0xF045,
    MS_DIV          = (xub_Unicode) 0xF046,
    MS_PLUSMINUS    = (xub_Unicode) 0xF047,
    MS_MINUSPLUS    = (xub_Unicode) 0xF048,
    MS_OPLUS        = (xub_Unicode) 0xF049,
    MS_OMINUS       = (xub_Unicode) 0xF04A,
    MS_OTIMES       = (xub_Unicode) 0xF04B,
    MS_ODIVIDE      = (xub_Unicode) 0xF04C,
    MS_ODOT         = (xub_Unicode) 0xF04D,
    MS_UNION        = (xub_Unicode) 0xF04E,
    MS_INTERSECT    = (xub_Unicode) 0xF04F,

    MS_LT           = (xub_Unicode) 0xF050,
    MS_GT           = (xub_Unicode) 0xF051,
    MS_LE           = (xub_Unicode) 0xF052,
    MS_GE           = (xub_Unicode) 0xF053,
    MS_LESLANT      = (xub_Unicode) 0xF054,
    MS_GESLANT      = (xub_Unicode) 0xF055,
    MS_LL           = (xub_Unicode) 0xF056,
    MS_GG           = (xub_Unicode) 0xF057,
    MS_SIM          = (xub_Unicode) 0xF058,
    MS_SIMEQ        = (xub_Unicode) 0xF059,
    MS_APPROX       = (xub_Unicode) 0xF05A,
    MS_DEF          = (xub_Unicode) 0xF05B,
    MS_EQUIV        = (xub_Unicode) 0xF05C,
    MS_PROP         = (xub_Unicode) 0xF05D,
    MS_PARTIAL      = (xub_Unicode) 0xF05E,
    MS_SUBSET       = (xub_Unicode) 0xF05F,

    MS_SUPSET       = (xub_Unicode) 0xF060,
    MS_SUBSETEQ     = (xub_Unicode) 0xF061,
    MS_SUPSETEQ     = (xub_Unicode) 0xF062,
    MS_NSUBSET      = (xub_Unicode) 0xF063,
    MS_NSUPSET      = (xub_Unicode) 0xF064,
    MS_NSUBSETEQ    = (xub_Unicode) 0xF065,
    MS_NSUPSETEQ    = (xub_Unicode) 0xF066,
    MS_IN           = (xub_Unicode) 0xF067,
    MS_NOTIN        = (xub_Unicode) 0xF068,
    MS_EXISTS       = (xub_Unicode) 0xF06A,
    MS_BACKEPSILON  = (xub_Unicode) 0xF06B,
    MS_ALEPH        = (xub_Unicode) 0xF06C,
    MS_IM           = (xub_Unicode) 0xF06D,
    MS_RE           = (xub_Unicode) 0xF06E,
    MS_WP           = (xub_Unicode) 0xF06F,

    MS_LINE         = (xub_Unicode) 0xF073,
    MS_DLINE        = (xub_Unicode) 0xF074,
    MS_ORTHO        = (xub_Unicode) 0xF075,
    MS_DOTSLOW      = (xub_Unicode) 0xF076,
    MS_DOTSAXIS     = (xub_Unicode) 0xF077,
    MS_DOTSVERT     = (xub_Unicode) 0xF078,
    MS_DOTSUP       = (xub_Unicode) 0xF079,
    MS_DOTSDOWN     = (xub_Unicode) 0xF07A,
    MS_TRANSR       = (xub_Unicode) 0xF07B,
    MS_TRANSL       = (xub_Unicode) 0xF07C,
    MS_RIGHTARROW   = (xub_Unicode) 0xF07D,
    MS_BACKSLASH    = (xub_Unicode) 0xF07E,
    MS_NEG          = (xub_Unicode) 0xF07F,

    MS_INT          = (xub_Unicode) 0xF080,
    MS_IINT         = (xub_Unicode) 0xF081,
    MS_IIINT        = (xub_Unicode) 0xF082,
    MS_LINT         = (xub_Unicode) 0xF083,
    MS_LLINT        = (xub_Unicode) 0xF084,
    MS_LLLINT       = (xub_Unicode) 0xF085,
    MS_SQRT         = (xub_Unicode) 0xF087,
    MS_SQRT2        = (xub_Unicode) 0xF089,
    MS_COPROD       = (xub_Unicode) 0xF08A,
    MS_PROD         = (xub_Unicode) 0xF08B,
    MS_SUM          = (xub_Unicode) 0xF08C,
    MS_NABLA        = (xub_Unicode) 0xF08D,
    MS_FORALL       = (xub_Unicode) 0xF08E,

    MS_HAT          = (xub_Unicode) 0xF090,
    MS_CHECK        = (xub_Unicode) 0xF091,
    MS_BREVE        = (xub_Unicode) 0xF092,
    MS_ACUTE        = (xub_Unicode) 0xF093,
    MS_GRAVE        = (xub_Unicode) 0xF094,
    MS_TILDE        = (xub_Unicode) 0xF095,
    MS_BAR          = (xub_Unicode) 0xF096,
    MS_VEC          = (xub_Unicode) 0xF097,
    MS_DOT          = (xub_Unicode) 0xF098,
    MS_DDOT         = (xub_Unicode) 0xF099,
    MS_DDDOT        = (xub_Unicode) 0xF09A,
    MS_CIRCLE       = (xub_Unicode) 0xF09B,
    MS_AND          = (xub_Unicode) 0xF09C,
    MS_OR           = (xub_Unicode) 0xF09D,
    MS_NI           = (xub_Unicode) 0xF09E,
    MS_EMPTYSET     = (xub_Unicode) 0xF09F,

    MS_LBRACE       = (xub_Unicode) 0xF0A0,
    MS_RBRACE       = (xub_Unicode) 0xF0A1,
    MS_LPARENT      = (xub_Unicode) 0xF0A2,
    MS_RPARENT      = (xub_Unicode) 0xF0A3,
    MS_LANGLE       = (xub_Unicode) 0xF0A4,
    MS_RANGLE       = (xub_Unicode) 0xF0A5,
    MS_LBRACKET     = (xub_Unicode) 0xF0A6,
    MS_RBRACKET     = (xub_Unicode) 0xF0A7,

    MS_LDBRACKET    = (xub_Unicode) 0xF0B2,
    MS_RDBRACKET    = (xub_Unicode) 0xF0B3,
    MS_PLACE        = (xub_Unicode) 0xF0BF,

    MS_LCEIL        = (xub_Unicode) 0xF0C0,
    MS_LFLOOR       = (xub_Unicode) 0xF0C1,
    MS_RCEIL        = (xub_Unicode) 0xF0C2,
    MS_RFLOOR       = (xub_Unicode) 0xF0C3,
    MS_SQRT2_X      = (xub_Unicode) 0xF0C5,

    MS_TOP          = (xub_Unicode) 0xF0F5,
    MS_HBAR         = (xub_Unicode) 0xF0F6,
    MS_LAMBDABAR    = (xub_Unicode) 0xF0F7,
    MS_LEFTARROW    = (xub_Unicode) 0xF0F8,
    MS_UPARROW      = (xub_Unicode) 0xF0F9,
    MS_DOWNARROW    = (xub_Unicode) 0xF0FA,
    MS_SETN         = (xub_Unicode) 0xF0FB,
    MS_SETZ         = (xub_Unicode) 0xF0FC,
    MS_SETQ         = (xub_Unicode) 0xF0FD,
    MS_SETR         = (xub_Unicode) 0xF0FE,
    MS_SETC         = (xub_Unicode) 0xF0FF
};


#endif


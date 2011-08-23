/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
* @file
* Backgound object of Wordpro.
************************************************************************/
/*************************************************************************
* Change History
* 2005-01-12 Create and implement.
************************************************************************/
#ifndef		_LWPBACKGROUNDSTUFF_HXX
#define		_LWPBACKGROUNDSTUFF_HXX
#include	"lwpcolor.hxx"

/* background patterns */
#define BACK_TRANSPARENT	0
#define BACK_FILL			1
#define BACK_PATTERN		2
#define BACK_10PCTDOTS		3
#define BACK_30PCTDOTS		4
#define BACK_50PCTDOTS		5
#define BACK_75PCTDOTS		6
#define BACK_90PCTDOTS		7
#define BACK_90PCTINVDOTS	8
#define BACK_75PCTINVDOTS	9
#define BACK_50PCTINVDOTS	10
#define BACK_30PCTINVDOTS	11
#define BACK_10PCTINVDOTS	12
#define BACK_MARBLES		13
#define BACK_STARRYNIGHT	14
#define BACK_DIAGTHICK		15
#define BACK_DIAGTHIN		16
#define BACK_DIAGDOUBLE		17
#define BACK_DIAGDOUBLENARROW 18
#define BACK_DIAGTRIPLE		19
#define BACK_DIAGSINGLEWIDE	20
#define BACK_DIAGTHICKINV	21
#define BACK_DIAGTHININV	22
#define BACK_DIAGDOUBLEINV	23
#define BACK_DIAGDOUBLENARROWINV 24
#define BACK_DIAGTRIPLEINV		25
#define BACK_DIAGSINGLEWIDEINV	26
#define BACK_CHAINLINKSMALLINV	27
#define BACK_CHAINLINKSMALL		28
#define BACK_CHAINLINKMEDIUM	29
#define BACK_CHAINLINKLARGE		30
#define BACK_HORZSTRIPE1		31
#define BACK_HORZSTRIPE2		32
#define BACK_HORZSTRIPE3		33
#define BACK_HORZSTRIPE4		34
#define BACK_VERTSTRIPE1		35
#define BACK_VERTSTRIPE2		36
#define BACK_VERTSTRIPE3		37
#define BACK_VERTSTRIPE4		38
#define BACK_GRIDSMALL			39
#define BACK_GRIDMEDIUM			40
#define BACK_GRIDLARGE			41
#define BACK_BOXDOT				42
#define BACK_GOLFGREEN			43
#define BACK_DITZ				44
#define BACK_HORZWEAVE			45
#define BACK_APPLEPIE			46
#define BACK_BRICK				47
#define BACK_DIAGBRICK			48
#define BACK_CHECKER1			49
#define BACK_CHECKER2			50
#define BACK_CHECKER3			51
#define BACK_CHECKER4			52
#define BACK_CHECKER5			53
#define BACK_SNOWFLAKE			54
#define BACK_MARGARITA			55
#define BACK_SIDEHARP			56
#define BACK_SCALES				57
#define BACK_TAYLOR				58
#define BACK_BUBBLES			59
#define BACK_ARGYLE				60
#define BACK_INDIAN1			61
#define BACK_INDIAN3			62
#define BACK_PEACHPIE			63
#define BACK_GRADIENT1			64
#define BACK_GRADIENT2			65
#define BACK_GRADIENT3			66
#define BACK_GRADIENT4			67
#define BACK_GRADIENT5			68
#define BACK_GRADIENT6			69
#define BACK_GRADIENT7			70
#define BACK_GRADIENT8			71

class XFBGImage;
class LwpBackgroundStuff
{
public:
    LwpBackgroundStuff()
    {
        m_nID = 0;
    }

private:
    void GetPattern(sal_uInt16 btPttnIndex, sal_uInt8* pPttnArray);

public:
    void	Read(LwpObjectStream *pStrm);
    LwpColor* GetFillColor();
    XFBGImage* GetFillPattern();

    inline sal_Bool IsTransparent() {return (m_nID == BACK_TRANSPARENT);}
    inline sal_Bool IsPatternFill() { return (sal_Bool)(m_nID > 2 && m_nID < 72); }

    friend class LwpBackgroundOverride;
private:
    sal_uInt16	m_nID;
    LwpColor	m_aFillColor;
    LwpColor	m_aPatternColor;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

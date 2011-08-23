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
* Border stuff of Wordpro.
************************************************************************/
/*************************************************************************
* Change History
* 2005-01-11 Create and implement.
************************************************************************/
#ifndef		_LWPBORDERSTUFF_HXX
#define		_LWPBORDERSTUFF_HXX

#include	"lwpcolor.hxx"

class LwpBorderStuff
{
public:
    LwpBorderStuff();

    enum BorderType
    {
        NOSIDE = 0,
        LEFT = 1,
        RIGHT = 2,
        TOP = 4,
        BOTTOM = 8,
        ALLSIDE = 15,
        TEMPORARY = 0xff00
    };

public:
    void	Read(LwpObjectStream *pStrm);

    sal_uInt16	GetSide(){ return m_nSides; }
    sal_Bool	HasSide(sal_uInt16 side);
    sal_uInt16	GetSideType(sal_uInt16 side);
    LwpColor	GetSideColor(sal_uInt16 side);
    float		GetSideWidth(sal_uInt16 side);
    //add by , 01/26/2004
    void operator = (const LwpBorderStuff& rOther);
    //end
    friend class LwpParaBorderOverride;
private:
    sal_uInt16		m_nSides;
    sal_uInt16		m_nValid;

    sal_uInt16		m_nBoderGroupIDLeft;
    sal_uInt16		m_nBoderGroupIDRight;
    sal_uInt16		m_nBoderGroupIDTop;
    sal_uInt16		m_nBoderGroupIDBottom;

    sal_Int32		m_nGroupIndent;

    sal_Int32		m_nWidthLeft;
    sal_Int32		m_nWidthTop;
    sal_Int32		m_nWidthRight;
    sal_Int32		m_nWidthBottom;

    LwpColor		m_aColorLeft;
    LwpColor		m_aColorRight;
    LwpColor		m_aColorTop;
    LwpColor		m_aColorBottom;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

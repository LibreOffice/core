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
* 2005-01-11  Create and implement.
* 2005-01-17  Add Get** functions.
************************************************************************/
#include	"lwpborderstuff.hxx"
#include	"lwpstyledef.hxx"
#include	"lwpfilehdr.hxx"
#include	"lwptools.hxx"

LwpBorderStuff::LwpBorderStuff()
{
    m_nSides = 0;
    m_nValid = 0;

    m_nBoderGroupIDLeft = 0;
    m_nBoderGroupIDRight = 0;
    m_nBoderGroupIDTop = 0;
    m_nBoderGroupIDBottom = 0;

    m_nGroupIndent = 0;

    m_nWidthLeft = 0;
    m_nWidthTop = 0;
    m_nWidthRight = 0;
    m_nWidthBottom = 0;
}

void	LwpBorderStuff::Read(LwpObjectStream *pStrm)
{
    pStrm->QuickRead(&m_nSides, sizeof(m_nSides) );
    if( m_nSides&LEFT )
    {
        pStrm->QuickRead(&m_nBoderGroupIDLeft, sizeof(m_nBoderGroupIDLeft));
        pStrm->QuickRead(&m_nWidthLeft, sizeof(m_nWidthLeft));
        m_aColorLeft.Read(pStrm);

        if( LwpFileHeader::m_nFileRevision < 0x000b )
        {
            pStrm->SeekRel(8);
        }
    }

    if( m_nSides&RIGHT )
    {
        pStrm->QuickRead(&m_nBoderGroupIDRight, sizeof(m_nBoderGroupIDRight));
        pStrm->QuickRead(&m_nWidthRight, sizeof(m_nWidthRight));
        m_aColorRight.Read(pStrm);

        if( LwpFileHeader::m_nFileRevision < 0x000b )
        {
            pStrm->SeekRel(8);
        }
    }

    if( m_nSides&TOP )
    {
        pStrm->QuickRead(&m_nBoderGroupIDTop, sizeof(m_nBoderGroupIDTop));
        pStrm->QuickRead(&m_nWidthTop, sizeof(m_nWidthTop));
        m_aColorTop.Read(pStrm);

        if( LwpFileHeader::m_nFileRevision < 0x000b )
        {
            pStrm->SeekRel(8);
        }
    }

    if( m_nSides&BOTTOM )
    {
        pStrm->QuickRead(&m_nBoderGroupIDBottom, sizeof(m_nBoderGroupIDBottom));
        pStrm->QuickRead(&m_nWidthBottom, sizeof(m_nWidthBottom));
        m_aColorBottom.Read(pStrm);

        if( LwpFileHeader::m_nFileRevision < 0x000b )
        {
            pStrm->SeekRel(8);
        }
    }

    pStrm->QuickRead( &m_nGroupIndent, sizeof(m_nGroupIndent) );
    pStrm->QuickRead( &m_nValid, sizeof(m_nValid) );
    pStrm->SkipExtra();

    if( LwpFileHeader::m_nFileRevision < 0x0010 )
    {
        if( m_nBoderGroupIDLeft&EXTERNAL_ID )
        {
            m_nBoderGroupIDLeft = BGRP_SOLID;
        }
        if( m_nBoderGroupIDRight&EXTERNAL_ID )
        {
            m_nBoderGroupIDRight = BGRP_SOLID;
        }
        if( m_nBoderGroupIDTop&EXTERNAL_ID )
        {
            m_nBoderGroupIDTop = BGRP_SOLID;
        }
        if( m_nBoderGroupIDBottom&EXTERNAL_ID )
        {
            m_nBoderGroupIDBottom = BGRP_SOLID;
        }
    }
}

sal_Bool	LwpBorderStuff::HasSide(sal_uInt16 side)
{
    return m_nSides&side;
}

sal_uInt16	LwpBorderStuff::GetSideType(sal_uInt16 side)
{
    switch(side)
    {
    case LEFT:
        return m_nBoderGroupIDLeft;
        break;
    case RIGHT:
        return m_nBoderGroupIDRight;
        break;
    case TOP:
        return m_nBoderGroupIDTop;
        break;
    case BOTTOM:
        return m_nBoderGroupIDBottom;
        break;
    }
    // FIXME: this is needed to avoid warning: control reaches end of non-void function
    //        a better solution would be to enum value for the parameter side
    return 0;
}

LwpColor	LwpBorderStuff::GetSideColor(sal_uInt16 side)
{
    switch(side)
    {
    case LEFT:
        return m_aColorLeft;
        break;
    case RIGHT:
        return m_aColorRight;
        break;
    case TOP:
        return m_aColorTop;
        break;
    case BOTTOM:
        return m_aColorBottom;
        break;
    }
    // FIXME: this is needed to avoid warning: control reaches end of non-void function
    //        a better solution would be to enum value for the parameter side
    return LwpColor();
}

float	LwpBorderStuff::GetSideWidth(sal_uInt16 side)
{
    switch(side)
    {
    case LEFT:
        return LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(m_nWidthLeft));
        break;
    case RIGHT:
        return LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(m_nWidthRight));
        break;
    case TOP:
        return LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(m_nWidthTop));
        break;
    case BOTTOM:
        return LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(m_nWidthBottom));
        break;
    }
    // FIXME: this is needed to avoid warning: control reaches end of non-void function
    //        a better solution would be to enum value for the parameter side
    return 0;
}

void LwpBorderStuff::operator = (const LwpBorderStuff& rOther)
{
    m_nSides = rOther.m_nSides;
    m_nValid = rOther.m_nValid;

    m_nBoderGroupIDLeft = rOther.m_nBoderGroupIDLeft;
    m_nBoderGroupIDRight = rOther.m_nBoderGroupIDRight;
    m_nBoderGroupIDTop = rOther.m_nBoderGroupIDTop;
    m_nBoderGroupIDBottom = rOther.m_nBoderGroupIDBottom;

    m_nGroupIndent = rOther.m_nGroupIndent;

    m_nWidthLeft = rOther.m_nWidthLeft;
    m_nWidthTop = rOther.m_nWidthTop;
    m_nWidthRight = rOther.m_nWidthRight;
    m_nWidthBottom = rOther.m_nWidthBottom;

    m_aColorLeft = rOther.m_aColorLeft;
    m_aColorRight = rOther.m_aColorRight;
    m_aColorTop = rOther.m_aColorTop;
    m_aColorBottom = rOther.m_aColorBottom;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

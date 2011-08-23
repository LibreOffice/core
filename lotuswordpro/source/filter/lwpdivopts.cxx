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
 *  For LWP filter architecture prototype
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#include "lwpdivopts.hxx"
#include "lwpfilehdr.hxx"

LwpHyphenOptions::LwpHyphenOptions()
{}

LwpHyphenOptions::~LwpHyphenOptions()
{}

void LwpHyphenOptions::Read(LwpObjectStream *pStrm)
{
    pStrm->QuickRead(&m_nFlags, sizeof(m_nFlags));
    pStrm->QuickRead(&m_nZoneBefore, sizeof(m_nZoneBefore));
    pStrm->QuickRead(&m_nZoneAfter, sizeof(m_nZoneAfter));
    pStrm->QuickRead(&m_nMaxLines, sizeof(m_nMaxLines));
    pStrm->SkipExtra();
}



LwpTextLanguage::LwpTextLanguage(){};
LwpTextLanguage::~LwpTextLanguage(){}

void LwpTextLanguage::Read(LwpObjectStream *pStrm)
{
    pStrm->QuickRead(&m_nLanguage, sizeof(m_nLanguage));
    pStrm->SkipExtra();

    if( LwpFileHeader::m_nFileRevision <= 0x0a)
        m_nLanguage = ConvertFrom96(m_nLanguage);
}

sal_uInt16 LwpTextLanguage::ConvertFrom96(sal_uInt16 orgLang)
{
    // Reading a Word Pro 96 into Word Pro 97
    switch (orgLang)
    {
        case 0x2809U:						// AMI_MEDICAL 96
            orgLang = 0x8409U;				//0x8409U		97
            break;
        case 0x2C09U:				   		// AMI_BRMEDICAL 96
            orgLang = 0x8809U; 				// 0x8809U       97
            break;
        case 0x3409U:						// AMI_BRITISHMEDIZE 96
            orgLang = 0x8C09U;  				// 0x8C09U           97
            break;
        case 0x3009U:						// AMI_BRITISHIZE 96
            orgLang = 0x1009U; 				// 0x1009U        97
            break;
        case 0x819U: 						// AMI_RUSSIANIO  96
            orgLang = 0x8419U;		  		// 0x8419U		  97
            break;
    }
    return orgLang;
}

LwpDivisionOptions::LwpDivisionOptions(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpObject(objHdr, pStrm)
{}

LwpDivisionOptions::~LwpDivisionOptions()
{}

void LwpDivisionOptions::Read()
{
    m_HyphOpts.Read(m_pObjStrm);
    m_pObjStrm->QuickRead(&m_nOptionFlag, sizeof(m_nOptionFlag));
    m_Lang.Read(m_pObjStrm);
    m_pObjStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

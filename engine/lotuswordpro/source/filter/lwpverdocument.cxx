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
 *  Versioned Pointer object, used for VO_VEGTR and VO_QTR
 * Does not find the difference between CVersionedQointer and
 * CVersionedGointer, so use one class
 ************************************************************************/

#include <lwpglobalmgr.hxx>
#include "lwpverdocument.hxx"
#include "lwplnopts.hxx"
#include "lwpproplist.hxx"
#include <xfilter/xfparastyle.hxx>
#include <lwptools.hxx>

LwpVerDocument::LwpVerDocument(LwpObjectHeader const& objHdr, LwpSvStream* pStrm)
    : LwpObject(objHdr, pStrm)
    , m_nTabSpacing(0)
{
}

void LwpVerDocument::Read()
{
    m_nTabSpacing = m_pObjStrm->QuickReaduInt32();

    if (m_pObjStrm->CheckExtra())
    {
        LwpLineNumberOptions aLineNumberOptions(m_pObjStrm.get());

        if (m_pObjStrm->CheckExtra())
        {
            LwpPropList aPropList;
            aPropList.Read(m_pObjStrm.get());
            m_pObjStrm->SkipExtra();
        }
    }
}

void LwpVerDocument::RegisterStyle()
{
    std::unique_ptr<XFDefaultParaStyle> pDefault(new XFDefaultParaStyle);
    double len = LwpTools::ConvertFromUnits(m_nTabSpacing);
    if (len < 0.001)
    {
        len = 1.27; //0.5 inch
    }
    pDefault->SetTabDistance(len);
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pXFStyleManager->AddStyle(std::move(pDefault));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

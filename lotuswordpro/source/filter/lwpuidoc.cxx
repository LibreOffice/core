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

#include "lwpuidoc.hxx"

LwpUIDocument::LwpUIDocument()
    : m_nFlags(DOC_LOCKED)
{}


LwpUIDocument::LwpUIDocument(LwpObjectStream *pStrm)
    : m_nFlags(DOC_LOCKED)
{
    Read(pStrm);
}

LwpUIDocument::~LwpUIDocument()
{};

void LwpUIDocument::Read(LwpObjectStream *pStrm)
{
    m_NamedProps.Read(pStrm);
    m_ARMacroOpts.Read(pStrm);
    m_MergedOpts.Read(pStrm);
    m_SheetFullPath.ReadPathAtom(pStrm);

    sal_uInt16 saved_flags;
    pStrm->QuickRead(&saved_flags, sizeof(saved_flags));
    m_nFlags  |= saved_flags;

    if(pStrm->CheckExtra())
    {
        m_InitialSaveAsType.Read(pStrm);
        pStrm->SkipExtra();
    }
}
/**
 * @descr		Read macro options from object stream
 **/
void LwpNamedProperties::Read(LwpObjectStream *pStrm)
{
    sal_uInt16 numEntries;
    pStrm->QuickRead(&numEntries, sizeof(numEntries));

    for (sal_uInt16 k = 0 ; k < numEntries; k++)
    {
        assert(false);
        // TODO: Read each NamedProperties
    }
    pStrm->SkipExtra();
}
/**
 * @descr		Read macro options from object stream
 **/
void LwpAutoRunMacroOptions::Read(LwpObjectStream *pStrm)
{
    m_OpenName.ReadPathAtom(pStrm);
    m_CloseName.ReadPathAtom(pStrm);
    m_NewName.ReadPathAtom(pStrm);
    pStrm->QuickRead(&m_OptionFlag, sizeof(m_OptionFlag));
    pStrm->SkipExtra();
}
/**
 * @descr		Read merge options from object stream
 **/
void LwpMergeOptions::Read(LwpObjectStream *pStrm)
{
    m_RecordFile.ReadPathAtom(pStrm);
    m_DescriptionFile.ReadPathAtom(pStrm);
    m_Filter.Read(pStrm);
    pStrm->QuickRead(&m_nType, sizeof(m_nType));

    //Does not process m_nType here. Assume m_nType is 0.
    // TODO: Read the CMergeDataFile
    assert(m_nType==0);

    pStrm->QuickRead(&m_nLastActionFlag, sizeof(m_nLastActionFlag));
    pStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

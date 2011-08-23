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
 *  all classes for LwpUIDocument
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/
#ifndef _LWPUIDOCUMENT_HXX
#define _LWPUIDOCUMENT_HXX

#include "lwpheader.hxx"
#include "lwpdefs.hxx"

class LwpNamedProperties;
class LwpAutoRunMacroOptions;
class LwpMergeOptions;

#include "lwpobjstrm.hxx"
#include "lwpatomholder.hxx"
/**
 * @brief		Properties in UIDocument structure
 *			not parsed yet
*/
class LwpNamedProperties
{
public:
    LwpNamedProperties(){};
    ~LwpNamedProperties(){};
public:
    void Read(LwpObjectStream *pStrm);
};
/**
 * @brief		macro options contained in UIDocument structure
 *			not parsed yet
*/
class LwpAutoRunMacroOptions
{
public:
    LwpAutoRunMacroOptions(){};
    ~LwpAutoRunMacroOptions(){};
private:
    LwpAtomHolder m_OpenName;
    LwpAtomHolder m_CloseName;
    LwpAtomHolder m_NewName;
    sal_uInt16 m_OptionFlag;
public:
    void Read(LwpObjectStream *pStrm);
};
/**
 * @brief		Merget options contained in UIDocument structure
 *			not parsed yet
*/
class LwpMergeOptions
{
public:
    LwpMergeOptions(){};
    ~LwpMergeOptions(){};
private:
    LwpAtomHolder m_RecordFile;
    LwpAtomHolder m_DescriptionFile;
    LwpAtomHolder m_Filter;
    sal_uInt16 m_nType;
    sal_uInt16 m_nLastActionFlag;	// flag remembers last merge action
public:
    void Read(LwpObjectStream *pStrm);
};
/**
 * @brief		UIDocument structure contained in VO_DOCUMENT
 *			not parsed yet
*/
class LwpUIDocument
{
public:
    LwpUIDocument();
    LwpUIDocument(LwpObjectStream *pStrm);
    ~LwpUIDocument();
private:
    LwpNamedProperties m_NamedProps;
    LwpAutoRunMacroOptions m_ARMacroOpts;
    LwpMergeOptions m_MergedOpts;
    LwpAtomHolder m_SheetFullPath;	// full path for style sheet
    sal_uInt16 m_nFlags;
    LwpAtomHolder m_InitialSaveAsType;
    enum
    {
        DOC_READONLY	= 0x01,
        DOC_BLOCKSETS	= 0x02,
        DOC_LOCKED = 0x04,
        DOC_ENVELOPE = 0x08,
        DOC_EXTERNALFILE = 0x10,
        DOC_SANITYCHECK = 0x20,
        DOC_ANNOTATEONLY = 0x40,
        DOC_CANCELED = 0x80
    };
public:
    void Read(LwpObjectStream *pStrm);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

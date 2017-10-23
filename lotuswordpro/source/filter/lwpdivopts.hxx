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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPDIVOPTS_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPDIVOPTS_HXX

#include <lwpobj.hxx>
#include <lwpobjid.hxx>
#include <lwpatomholder.hxx>
#include <lwpcolor.hxx>
#include <lwpheader.hxx>

class LwpHyphenOptions
{
public:
    LwpHyphenOptions();
    ~LwpHyphenOptions();
    void Read(LwpObjectStream *pStrm);
private:
    sal_uInt16 m_nFlags;
    sal_uInt16 m_nZoneBefore;
    sal_uInt16 m_nZoneAfter;
    sal_uInt16 m_nMaxLines;
};

/*
Text languange.
This class which holds identifiers for language and dialect.
DESCRIPTION You can mark text as being a certain language. When we spell
    check or grammar check we'll use this info to determine which dictionary
    to use.
    The only reserved value is 0, which means "default language". The
    default language is specified by the document.
*/
class LwpTextLanguage
{
public:
    LwpTextLanguage();
    ~LwpTextLanguage();
    void Read(LwpObjectStream *pStrm);
private:
    static sal_uInt16 ConvertFrom96(sal_uInt16 orgLang);
    sal_uInt16 m_nLanguage;
};

class LwpDivisionOptions : public LwpObject
{
public:
    LwpDivisionOptions(LwpObjectHeader const & objHdr, LwpSvStream* pStrm);
protected:
    void Read() override;
private:
    virtual ~LwpDivisionOptions() override;

    LwpHyphenOptions m_HyphOpts;
    sal_uInt16 m_nOptionFlag;
    LwpTextLanguage m_Lang;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

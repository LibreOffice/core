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
 * Font declaration for OOo xml file.This is needed for every font that was
 * used.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFFONTDECL_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFFONTDECL_HXX

#include "xfglobal.hxx"

/**
 * @brief
 * Font declaration in OOo xml file.
 * Each font used should be declared.
 */
class   XFFontDecl
{
public:
    XFFontDecl(const OUString& name, const OUString& family);

public:
    /**
     * @descr   Get font name.
     */
    const OUString& GetFontName() { return m_strFontName;}

    /**
     * @descr   Get font family.
     */
    const OUString& GetFontFamily() { return m_strFontFamily;}

    bool        GetFontPitchFixed() { return m_bPitchFixed;}

private:
    OUString   m_strFontName;
    OUString   m_strFontFamily;
    bool        m_bPitchFixed;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

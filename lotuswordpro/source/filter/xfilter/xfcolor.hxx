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
 * Color object to serial to xml filter.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-17 create this file.
 ************************************************************************/
#ifndef		_XFCOLOR_HXX
#define		_XFCOLOR_HXX

#include	"xfglobal.hxx"

/**
 * @brief
 * Color object.
 */
class XFColor
{
public:
    XFColor():m_nRed((sal_Int8)0),m_nGreen((sal_Int8)0),m_nBlue((sal_Int8)0)
    {
        m_bValid = sal_False;
    }
    XFColor(sal_Int32 red, sal_Int32 green, sal_Int32 blue)
        :m_nRed((sal_Int8)red),m_nGreen((sal_Int8)green),m_nBlue((sal_Int8)blue)
    {
        m_bValid = sal_True;
    }
    XFColor(sal_uInt32 color)
    {
        unsigned int	c = color&0x00ffffff;
        unsigned int	temp = c;
        m_nRed = temp&0x000000ff;

        temp = c;
        m_nGreen = (temp&0x0000ff00)>>8;

        temp = c;
        m_nBlue = (temp&0x00ff0000)>>16;

        m_bValid = sal_True;
    }

    /**
     * @descr	return red part of the color.
     */
    sal_uInt8	GetRed() const{ return m_nRed; }

    /**
     * @descr	return green part of the color.
     */
    sal_uInt8	GetGreen() const{ return m_nGreen; }

    /**
     * @descr	return blue part of the color.
     */
    sal_uInt8	GetBlue() const{ return m_nBlue; }

    /**
     * @descr	helper function to assert whether a color is valid.
     */
    sal_Bool	IsValid() const{ return m_bValid;};

    rtl::OUString	ToString ()const;

    friend bool operator==(XFColor& c1, XFColor& c2);
    friend bool operator!=(XFColor& c1, XFColor& c2);
private:
    sal_uInt8	m_nRed;
    sal_uInt8	m_nGreen;
    sal_uInt8	m_nBlue;
    sal_Bool	m_bValid;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

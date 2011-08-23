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
 *  basic classes for Word Pro filter,
 *  PanoseNumber, Point
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/


#ifndef _LWPBASETYPE_HXX
#define _LWPBASETYPE_HXX

#include "lwpobjstrm.hxx"
#include "lwpheader.hxx"

/**
 * @brief	used for lwpfont for font description
*/
class LwpPanoseNumber
{
public:
    LwpPanoseNumber(){};
    ~LwpPanoseNumber(){};
public:
    void Read(LwpObjectStream *pStrm);
private:
    sal_uInt8 m_nFamilyType;
    sal_uInt8 m_nSerifStyle;
    sal_uInt8 m_nWeight;
    sal_uInt8 m_nProportion;
    sal_uInt8 m_nContrast;
    sal_uInt8 m_nStrokeVariation;
    sal_uInt8 m_nArmStyle;
    sal_uInt8 m_nLetterform;
    sal_uInt8 m_nMidline;
    sal_uInt8 m_nXHeight;
};
/**
 * @brief	point class
*/
class LwpPoint
{
public:
    LwpPoint();
    void Read(LwpObjectStream *pStrm);
    void Reset();
    inline sal_Int32 GetX() const;
    inline sal_Int32 GetY() const;
    inline void SetX(sal_Int32 nX);
    inline void SetY(sal_Int32 nY);
protected:
    sal_Int32 m_nX;
    sal_Int32 m_nY;
};

sal_Int32 LwpPoint::GetX() const {return m_nX;}
sal_Int32 LwpPoint::GetY() const {return m_nY;}
void LwpPoint::SetX(sal_Int32 nX){m_nX = nX;}
void LwpPoint::SetY(sal_Int32 nY){m_nY = nY;}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: porhyph.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:25 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _PORHYPH_HXX
#define _PORHYPH_HXX

#include "porexp.hxx"

/*************************************************************************
 *                      class SwHyphPortion
 *************************************************************************/

class SwHyphPortion : public SwExpandPortion
{
public:
    inline  SwHyphPortion( ) { SetWhichPor( POR_HYPH ); }
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwHyphStrPortion
 *************************************************************************/

class SwHyphStrPortion : public SwHyphPortion
{
    XubString aExpand;
public:
    inline  SwHyphStrPortion( const XubString &rStr );
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwSoftHyphPortion
 *************************************************************************/

class SwSoftHyphPortion : public SwHyphPortion
{
    sal_Bool    bExpand;
    KSHORT  nViewWidth;
    KSHORT  nHyphWidth;

public:
    SwSoftHyphPortion();
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
    virtual SwLinePortion *Compress();
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
    inline void SetExpand( const sal_Bool bNew ) { bExpand = bNew; }
    sal_Bool IsExpand() const { return bExpand; }

    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwSoftHyphStrPortion
 *************************************************************************/

class SwSoftHyphStrPortion : public SwHyphStrPortion
{
public:
    SwSoftHyphStrPortion( const XubString &rStr );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    OUTPUT_OPERATOR
};

inline  SwHyphStrPortion::SwHyphStrPortion( const XubString &rStr )
        : aExpand( rStr )
{
    aExpand += '-';
    SetWhichPor( POR_HYPHSTR );
}

CLASSIO( SwHyphPortion )
CLASSIO( SwHyphStrPortion )
CLASSIO( SwSoftHyphPortion )
CLASSIO( SwSoftHyphStrPortion )


#endif

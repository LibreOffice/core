/*************************************************************************
 *
 *  $RCSfile: pormulti.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-23 10:19:54 $
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

#ifndef _PORMULTI_HXX
#define _PORMULTI_HXX

#include "porlay.hxx"

class SwTxtFormatInfo;
class SwFldPortion;
class SwTxtCursor;
class SwLineLayout;

/*-----------------16.10.00 12:45-------------------
 * The SwMultiPortion is line portion inside a line portion
 * to allow double line portions in a line.
 * --------------------------------------------------*/

class SwMultiPortion : public SwLinePortion
{
    SwLineLayout aRoot;     // One or more lines
    SwFldPortion *pFldRest; // a field rest from the previous line
public:
    SwMultiPortion( xub_StrLen nEnd ) : pFldRest( NULL )
        { SetWhichPor( POR_MULTI ); SetLen( nEnd ); }
    ~SwMultiPortion();

    const SwLineLayout& GetRoot() const { return aRoot; }
    SwLineLayout& GetRoot() { return aRoot; }

    SwFldPortion* GetFldRest() { return pFldRest; }
    void SetFldRest( SwFldPortion* pNew ) { pFldRest = pNew; }

    virtual void Paint( const SwTxtPaintInfo &rInf ) const;

    // Summarize the internal lines to calculate the (external) size
    void CalcSize( SwTxtFormatter& rLine );

    OUTPUT_OPERATOR
};

// For cursor travelling in multiportions

class SwTxtCursorSave
{
    SwTxtCursor* pTxtCrsr;
    SwLineLayout* pCurr;
    xub_StrLen nStart;
public:
    SwTxtCursorSave( SwTxtCursor* pTxtCursor, SwMultiPortion* pMulti,
        SwTwips nY, xub_StrLen nCurrStart );
    ~SwTxtCursorSave();
};

/*************************************************************************
 *                  inline - Implementations
 *************************************************************************/

CLASSIO( SwMultiPortion )

#endif

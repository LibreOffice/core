/*************************************************************************
 *
 *  $RCSfile: blink.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:20 $
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

#ifndef _BLINK_HXX
#define _BLINK_HXX

class SwLinePortion;
class SwRootFrm;
class SwTxtFrm;

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

class SwBlinkPortion
{
    const SwLinePortion *pPor;
    const SwRootFrm     *pFrm;
    Point           aPos;
public:
    SwBlinkPortion( const SwLinePortion* pPortion ){ pPor = pPortion; }
    SwBlinkPortion( const SwBlinkPortion* pBlink, const SwLinePortion* pPort )
    {   pPor = pPort; pFrm = pBlink->pFrm; aPos = pBlink->aPos; }
    void SetPos( const Point& aNew ){ aPos = aNew; }
    const Point& GetPos() const{ return aPos; }
    void SetRootFrm( const SwRootFrm* pNew ){ pFrm = pNew; }
    const SwRootFrm* GetRootFrm() const{ return pFrm; }
    const SwLinePortion *GetPortion() const{ return pPor; }
    BOOL operator<( const SwBlinkPortion& rBlinkPortion ) const
    { return (long)pPor < (long)rBlinkPortion.pPor; }
    BOOL operator==( const SwBlinkPortion& rBlinkPortion ) const
    { return (long)pPor == (long)rBlinkPortion.pPor; }
};

typedef SwBlinkPortion* SwBlinkPortionPtr;
SV_DECL_PTRARR_SORT_DEL(SwBlinkList, SwBlinkPortionPtr, 0, 10)

class SwBlink
{
    SwBlinkList     aList;
    AutoTimer       aTimer;
    BOOL            bVisible;
public:
    SwBlink();
    ~SwBlink();

    DECL_LINK( Blinker, Timer * );

    void Insert( const SwLinePortion* pPor, const Point& rPoint,
                 const SwTxtFrm *pTxtFrm );
    void Replace( const SwLinePortion* pOld, const SwLinePortion* pNew );
    void Delete( const SwLinePortion* pPor );
    void FrmDelete( const SwRootFrm* pRoot );
    inline BOOL IsVisible() const { return bVisible ; }
};

// Blink-Manager, globale Variable, in Blink.Cxx angelegt
extern SwBlink *pBlink;


#endif


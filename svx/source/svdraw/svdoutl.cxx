/*************************************************************************
 *
 *  $RCSfile: svdoutl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:25 $
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

#ifndef _SVDOUTL_HXX //autogen
#include "svdoutl.hxx"
#endif
#ifndef _OUTLINER_HXX //autogen
#include "outliner.hxx"
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include "svdotext.hxx"
#endif
#ifndef _EDITSTAT_HXX //autogen wg. EE_CNTRL_STRETCHING
#include <editstat.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen wg. SdrModel
#include <svdmodel.hxx>
#endif
#ifndef _EEITEM_HXX //autogen wg. EE_ITEMS_START
#include <eeitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen wg. SfxItemPool
#include <svtools/itempool.hxx>
#endif


//TYPEINIT1( SdrOutliner, Outliner );

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
SdrOutliner::SdrOutliner( SfxItemPool* pItemPool, USHORT nMode ):
Outliner( pItemPool, nMode ),
pTextObj( NULL )
{
}


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
SdrOutliner::~SdrOutliner()
{
}


/*************************************************************************
|*
|*
|*
\************************************************************************/
void SdrOutliner::SetTextObj( const SdrTextObj* pObj )
{
    if( pObj && pObj != pTextObj )
    {
        SetUpdateMode(FALSE);
        USHORT nOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
        if ( !pObj->IsOutlText() )
            nOutlinerMode = OUTLINERMODE_TEXTOBJECT;
        Init( nOutlinerMode );

        SetGlobalCharStretching(100,100);

        ULONG nStat = GetControlWord();
        nStat &= ~( EE_CNTRL_STRETCHING | EE_CNTRL_AUTOPAGESIZE );
        SetControlWord(nStat);

        Size aNullSize;
        Size aMaxSize( 100000,100000 );
        SetMinAutoPaperSize( aNullSize );
        SetMaxAutoPaperSize( aMaxSize );
        SetPaperSize( aMaxSize );
        ClearPolygon();
    }

    pTextObj = pObj;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
XubString SdrOutliner::CalcFieldValue(const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
                                     Color*& rpTxtColor, Color*& rpFldColor)
{
    FASTBOOL bOk = FALSE;
    XubString aRet;

    if (pTextObj)
        bOk = pTextObj->CalcFieldValue(rField, nPara, nPos, FALSE, rpTxtColor, rpFldColor, aRet);

    if (!bOk)
        aRet = Outliner::CalcFieldValue(rField, nPara, nPos, rpTxtColor, rpFldColor);

    return aRet;
}






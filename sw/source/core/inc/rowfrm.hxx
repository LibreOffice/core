/*************************************************************************
 *
 *  $RCSfile: rowfrm.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 13:32:45 $
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
#ifndef _ROWFRM_HXX
#define _ROWFRM_HXX

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#include "layfrm.hxx"

class SwTableLine;
class SwBorderAttrs;

class SwRowFrm: public SwLayoutFrm
{
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
        //Aendern nur die Framesize, nicht die PrtArea-SSize
    virtual SwTwips ShrinkFrm( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm  ( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );

    const SwTableLine *pTabLine;
    SwRowFrm* pFollowRow;
    // --> collapsing borders FME 2005-05-27 #i29550#
    USHORT mnTopMarginForLowers;
    USHORT mnBottomMarginForLowers;
    USHORT mnBottomLineSize;
    // <-- collapsing
    bool bIsFollowFlowRow;
    bool bIsRepeatedHeadline;

protected:
    virtual void MakeAll();

public:
    SwRowFrm( const SwTableLine &, bool bInsertContent = true );
    ~SwRowFrm();

    virtual void Cut();
    virtual void  Modify( SfxPoolItem*, SfxPoolItem* );

    //Zum Anmelden der Flys nachdem eine Zeile erzeugt _und_ eingefuegt wurde.
    //Muss vom Erzeuger gerufen werden, denn erst nach dem Konstruieren wird
    //Das Teil gepastet; mithin ist auch erst dann die Seite zum Anmelden der
    //Flys erreichbar.
    void RegistFlys( SwPageFrm *pPage = 0 );

    const SwTableLine *GetTabLine() const { return pTabLine; }

    //Passt die Zellen auf die aktuelle Hoehe an, invalidiert die Zellen
    //wenn die Direction nicht der Hoehe entspricht.
    void AdjustCells( const SwTwips nHeight, const BOOL bHeight );

    //
    //
    SwRowFrm* GetFollowRow() const { return pFollowRow; }
    void SetFollowRow( SwRowFrm* pNew ) { pFollowRow = pNew; }

    // --> collapsing borders FME 2005-05-27 #i29550#
    USHORT GetTopMarginForLowers() const { return mnTopMarginForLowers; }
    void   SetTopMarginForLowers( USHORT nNew ) { mnTopMarginForLowers = nNew; }
    USHORT GetBottomMarginForLowers() const { return mnBottomMarginForLowers; }
    void   SetBottomMarginForLowers( USHORT nNew ) { mnBottomMarginForLowers = nNew; }
    USHORT GetBottomLineSize() const { return mnBottomLineSize; }
    void   SetBottomLineSize( USHORT nNew ) { mnBottomLineSize = nNew; }
    // <-- collapsing

    bool IsRepeatedHeadline() const { return bIsRepeatedHeadline; }
    void SetRepeatedHeadline( bool bNew ) { bIsRepeatedHeadline = bNew; }

    // --> split table rows
    bool IsRowSplitAllowed() const;
    bool IsFollowFlowRow() const { return bIsFollowFlowRow; }
    void SetFollowFlowRow( bool bNew ) { bIsFollowFlowRow = bNew; }
    // <-- split table rows

    DECL_FIXEDMEMPOOL_NEWDEL(SwRowFrm)
};

#endif

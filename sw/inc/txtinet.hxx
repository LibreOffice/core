/*************************************************************************
 *
 *  $RCSfile: txtinet.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-15 15:48:26 $
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
#ifndef _TXTINET_HXX
#define _TXTINET_HXX

#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _CALBCK_HXX //autogen
#include <calbck.hxx>
#endif

class SvxFont;
class SwTxtNode;
class SwCharFmt;
class Color;

// ATT_INETFMT *********************************************

class SwTxtINetFmt : public SwTxtAttrEnd, public SwClient
{
    SwTxtNode* pMyTxtNd;
    BOOL bVisited       : 1; // Besuchter Link?
    BOOL bValidVis      : 1; // Ist das bVisited-Flag gueltig?
    BOOL bColor         : 1;

public:
    SwTxtINetFmt( const SwFmtINetFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtINetFmt();
    TYPEINFO();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    // erfrage und setze den TxtNode Pointer
    const SwTxtNode* GetpTxtNode() const { return pMyTxtNd; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }

          SwCharFmt* GetCharFmt();
    const SwCharFmt* GetCharFmt() const
            { return ((SwTxtINetFmt*)this)->GetCharFmt(); }

    const BOOL IsVisited() const { return bVisited; }
    void SetVisited( BOOL bNew ) { bVisited = bNew; }

    const BOOL IsValidVis() const { return bValidVis; }
    void SetValidVis( BOOL bNew ) { bValidVis = bNew; }

    BOOL IsProtect() const;
};
inline const SwTxtNode& SwTxtINetFmt::GetTxtNode() const
{
    ASSERT( pMyTxtNd, "SwTxtINetFmt:: wo ist mein TextNode?" );
    return *pMyTxtNd;
}

#endif


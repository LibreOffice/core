/*************************************************************************
 *
 *  $RCSfile: txtftn.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:29 $
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
#ifndef _TXTFTN_HXX
#define _TXTFTN_HXX

#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif

class SwNodeIndex;
class SwTxtNode;
class SwNodes;
class SwDoc;

// ATT_FTN **********************************************************

class SwTxtFtn : public SwTxtAttr
{
    SwNodeIndex *pStartNode;
    SwTxtNode* pMyTxtNd;
    USHORT nSeqNo;

    // Zuweisung und Copy-Ctor sind nicht erlaubt.
    SwTxtFtn& operator=( const SwTxtFtn &rTxtFtn );
    SwTxtFtn( const SwTxtFtn &rTxtFtn );

public:
    SwTxtFtn( const SwFmtFtn& rAttr, xub_StrLen nStart );
    ~SwTxtFtn();

    inline SwNodeIndex *GetStartNode() const { return pStartNode; }
    void SetStartNode( const SwNodeIndex *pNode, BOOL bDelNodes = TRUE );
    void SetNumber( const USHORT nNumber, const String* = 0 );
    void CopyFtn( SwTxtFtn *pDest );

    // erfrage und setze den TxtNode Pointer
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }

        // lege eine neue leere TextSection fuer diese Fussnote an
    void MakeNewTextSection( SwNodes& rNodes );

        // loesche die FtnFrame aus der Seite
    void DelFrms();
        // bedingten Absatzvorlagen checken
    void CheckCondColl();

        // fuer die Querverweise auf Fussnoten
    USHORT SetSeqRefNo();
    void SetSeqNo( USHORT n )               { nSeqNo = n; } // fuer die Reader
    USHORT GetSeqRefNo() const              { return nSeqNo; }

    static void SetUniqueSeqRefNo( SwDoc& rDoc );
};

inline const SwTxtNode& SwTxtFtn::GetTxtNode() const
{
    ASSERT( pMyTxtNd, "SwTxtFtn:: wo ist mein TextNode?" );
    return *pMyTxtNd;
}

#endif


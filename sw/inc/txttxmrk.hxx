/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txttxmrk.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:23:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _TXTTXMRK_HXX
#define _TXTTXMRK_HXX

#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif

class SwTxtNode;
class SwDoc;

// SWTXT_TOXMARK *******************************
// Attribut fuer die Verzeichnisse

class SwTxtTOXMark : public SwTxtAttrEnd
{
    xub_StrLen* pEnd;       // wenn AlternativText vom SwTOXMark dann 0 !!
    SwTxtNode* pMyTxtNd;
public:
    SwTxtTOXMark( const SwTOXMark& rAttr,
                    xub_StrLen nStart, xub_StrLen * pEnd = 0 );
    virtual ~SwTxtTOXMark();

    virtual xub_StrLen *GetEnd();                   // SwTxtAttr
    inline const xub_StrLen* GetEnd() const { return pEnd; }

    void CopyTOXMark( SwDoc* pDestDoc );

    // erfrage und setze den TxtNode Pointer
    inline const SwTxtNode* GetpTxtNd() const { return pMyTxtNd; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }
};

inline const SwTxtNode& SwTxtTOXMark::GetTxtNode() const
{
    ASSERT( pMyTxtNd, "SwTxtTOXMark:: wo ist mein TextNode?" );
    return *pMyTxtNd;
}

#endif


/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _TXTTXMRK_HXX
#define _TXTTXMRK_HXX

#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
namespace binfilter {

class SwTxtNode;
class SwDoc;

// SWTXT_TOXMARK *******************************
// Attribut fuer die Verzeichnisse

class SwTxtTOXMark : public SwTxtAttrEnd
{
    xub_StrLen* pEnd;		// wenn AlternativText vom SwTOXMark dann 0 !!
    SwTxtNode* pMyTxtNd;
public:
    SwTxtTOXMark( const SwTOXMark& rAttr,
                    xub_StrLen nStart, xub_StrLen * pEnd = 0 );
    virtual ~SwTxtTOXMark();

    virtual xub_StrLen *GetEnd();					// SwTxtAttr
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

} //namespace binfilter
#endif


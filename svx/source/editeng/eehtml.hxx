/*************************************************************************
 *
 *  $RCSfile: eehtml.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
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

#ifndef _EEHTML_HXX
#define _EEHTML_HXX

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#include <editdoc.hxx>

#ifndef _SFXHTML_HXX //autogen wg. SfxHTMLParser
#include <sfx2/sfxhtml.hxx>
#endif

class ImpEditEngine;

#define MAX_NUMBERLEVEL         10

struct AnchorInfo
{
    String  aHRef;
    String  aText;
};

#ifndef SVX_LIGHT

class EditHTMLParser : public SfxHTMLParser
{
private:
    EditSelection           aCurSel;
    ImpEditEngine*          pImpEditEngine;
    AnchorInfo*             pCurAnchor;

    BOOL                    bInPara;
    BOOL                    bWasInPara; // bInPara vor HeadingStart merken, weil sonst hinterher weg
    BOOL                    bFieldsInserted;
    BYTE                    nInTable;
    BYTE                    nInCell;

    USHORT*                 pNumbers;
    BYTE                    nDefListLevel;
    BYTE                    nBulletLevel;
    BYTE                    nNumberingLevel;

    BYTE                    nLastAction;

    void                    StartPara( BOOL bReal );
    void                    EndPara( BOOL bReal );
    void                    AnchorStart();
    void                    AnchorEnd();
    void                    HeadingStart( int nToken );
    void                    HeadingEnd( int nToken );
    void                    SkipGroup( int nEndToken );
    BOOL                    ThrowAwayBlank();
    BOOL                    HasTextInCurrentPara();
    void                    ProcessUnknownControl( BOOL bOn );
    inline USHORT*          GetNumbers();

    void                    ImpInsertParaBreak();
    void                    ImpInsertText( const String& rText );
    void                    ImpSetAttribs( const SfxItemSet& rItems, EditSelection* pSel = 0 );
    void                    ImpSetStyleSheet( USHORT nHeadingLevel );

protected:
    virtual void            NextToken( int nToken );

public:
                            EditHTMLParser( SvStream& rIn, SvKeyValueIterator* pHTTPHeaderAttrs, int bReadNewDoc = TRUE );
                            ~EditHTMLParser();

    virtual SvParserState   CallParser( ImpEditEngine* pImpEE, const EditPaM& rPaM );

    const EditSelection&    GetCurSelection() const { return aCurSel; }
};

inline USHORT* EditHTMLParser::GetNumbers()
{
    if ( !pNumbers )
    {
        pNumbers = new USHORT[MAX_NUMBERLEVEL];
        for ( USHORT n = 0; n < MAX_NUMBERLEVEL; n++ )
            pNumbers[n] = 0;
    }
    return pNumbers;
}


SV_DECL_REF( EditHTMLParser );
SV_IMPL_REF( EditHTMLParser );

#endif

#endif // _EEHTML_HXX

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eehtml.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:37:55 $
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
    using HTMLParser::CallParser;
private:
    EditSelection           aCurSel;
    String                  aBaseURL;
    ImpEditEngine*          pImpEditEngine;
    AnchorInfo*             pCurAnchor;

    BOOL                    bInPara;
    BOOL                    bWasInPara; // bInPara vor HeadingStart merken, weil sonst hinterher weg
    BOOL                    bFieldsInserted;
    BYTE                    nInTable;
    BYTE                    nInCell;

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

    void                    ImpInsertParaBreak();
    void                    ImpInsertText( const String& rText );
    void                    ImpSetAttribs( const SfxItemSet& rItems, EditSelection* pSel = 0 );
    void                    ImpSetStyleSheet( USHORT nHeadingLevel );

protected:
    virtual void            NextToken( int nToken );

public:
                            EditHTMLParser( SvStream& rIn, const String& rBaseURL, SvKeyValueIterator* pHTTPHeaderAttrs );
                            ~EditHTMLParser();

    virtual SvParserState   CallParser( ImpEditEngine* pImpEE, const EditPaM& rPaM );

    const EditSelection&    GetCurSelection() const { return aCurSel; }
};

SV_DECL_REF( EditHTMLParser )
SV_IMPL_REF( EditHTMLParser );

#endif

#endif // _EEHTML_HXX

/*************************************************************************
 *
 *  $RCSfile: txtatr.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-15 15:49:15 $
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
#ifndef _TXTATR_HXX
#define _TXTATR_HXX

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>     // SwTxtAttr/SwTxtAttrEnd
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif

class SwTxtNode;    // fuer SwTxtFld
class SvxFont;
class SwCharSetCol;
class SwCharFmt;
class SvxTwoLinesItem;

// ATT_CHARFMT *********************************************

class SwTxtCharFmt : public SwTxtAttrEnd
{
    SwTxtNode* pMyTxtNd;
    BOOL bPrevNoHyph    : 1;
    BOOL bPrevBlink     : 1;
    BOOL bPrevURL       : 1;
    BOOL bColor         : 1;

public:
    SwTxtCharFmt( const SwFmtCharFmt& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    ~SwTxtCharFmt( );

    // werden vom SwFmtCharFmt hierher weitergeleitet
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );    // SwClient
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    // erfrage und setze den TxtNode Pointer
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }

};

// ATT_HARDBLANK ******************************

class SwTxtHardBlank : public SwTxtAttr
{
    sal_Unicode cChar;
public:
    SwTxtHardBlank( const SwFmtHardBlank& rAttr, xub_StrLen nStart );
    inline sal_Unicode GetChar() const  { return cChar; }
};

// ATT_XNLCONTAINERITEM ******************************

class SwTxtXMLAttrContainer : public SwTxtAttrEnd
{
public:
    SwTxtXMLAttrContainer( const SvXMLAttrContainerItem& rAttr,
                        xub_StrLen nStart, xub_StrLen nEnd );
};

// ******************************

class SwTxtRuby : public SwTxtAttrEnd, public SwClient
{
    SwTxtNode* pMyTxtNd;

public:
    SwTxtRuby( const SwFmtRuby& rAttr, xub_StrLen nStart, xub_StrLen nEnd );
    virtual ~SwTxtRuby();
    TYPEINFO();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    // erfrage und setze den TxtNode Pointer
    const SwTxtNode* GetpTxtNode() const            { return pMyTxtNd; }
    inline const SwTxtNode& GetTxtNode() const;
    void ChgTxtNode( const SwTxtNode* pNew ) { pMyTxtNd = (SwTxtNode*)pNew; }

          SwCharFmt* GetCharFmt();
    const SwCharFmt* GetCharFmt() const
            { return ((SwTxtRuby*)this)->GetCharFmt(); }
};

// ******************************

class SwTxt2Lines : public SwTxtAttrEnd
{
public:
    SwTxt2Lines( const SvxTwoLinesItem& rAttr,
                    xub_StrLen nStart, xub_StrLen nEnd );
};

// --------------- Inline Implementierungen ------------------------

inline const SwTxtNode& SwTxtCharFmt::GetTxtNode() const
{
    ASSERT( pMyTxtNd, "SwTxtCharFmt:: wo ist mein TextNode?" );
    return *pMyTxtNd;
}

inline const SwTxtNode& SwTxtRuby::GetTxtNode() const
{
    ASSERT( pMyTxtNd, "SwTxtRuby:: wo ist mein TextNode?" );
    return *pMyTxtNd;
}

#endif

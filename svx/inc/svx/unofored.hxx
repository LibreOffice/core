/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unofored.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:44:16 $
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

#ifndef _SVX_UNOFORED_HXX
#define _SVX_UNOFORED_HXX

#include <svx/editeng.hxx>
#ifndef _SVX_UNOEDSRC_HXX
#include <svx/unoedsrc.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//  SvxEditEngineForwarder - SvxTextForwarder fuer EditEngine

class SVX_DLLPUBLIC SvxEditEngineForwarder : public SvxTextForwarder
{
private:
    EditEngine&         rEditEngine;

public:
                        SvxEditEngineForwarder( EditEngine& rEngine );
    virtual             ~SvxEditEngineForwarder();

    virtual USHORT      GetParagraphCount() const;
    virtual USHORT      GetTextLen( USHORT nParagraph ) const;
    virtual String      GetText( const ESelection& rSel ) const;
    virtual SfxItemSet  GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib = EditEngineAttribs_All ) const;
    virtual SfxItemSet  GetParaAttribs( USHORT nPara ) const;
    virtual void        SetParaAttribs( USHORT nPara, const SfxItemSet& rSet );
    virtual void        GetPortions( USHORT nPara, SvUShorts& rList ) const;

    virtual USHORT      GetItemState( const ESelection& rSel, USHORT nWhich ) const;
    virtual USHORT      GetItemState( USHORT nPara, USHORT nWhich ) const;

    virtual void        QuickInsertText( const String& rText, const ESelection& rSel );
    virtual void        QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    virtual void        QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    virtual void        QuickInsertLineBreak( const ESelection& rSel );

    virtual SfxItemPool* GetPool() const;

    virtual XubString    CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor );
    virtual BOOL         IsValid() const;

    virtual LanguageType    GetLanguage( USHORT, USHORT ) const;
    virtual USHORT          GetFieldCount( USHORT nPara ) const;
    virtual EFieldInfo      GetFieldInfo( USHORT nPara, USHORT nField ) const;
    virtual EBulletInfo     GetBulletInfo( USHORT nPara ) const;
    virtual Rectangle       GetCharBounds( USHORT nPara, USHORT nIndex ) const;
    virtual Rectangle       GetParaBounds( USHORT nPara ) const;
    virtual MapMode         GetMapMode() const;
    virtual OutputDevice*   GetRefDevice() const;
    virtual sal_Bool        GetIndexAtPoint( const Point&, USHORT& nPara, USHORT& nIndex ) const;
    virtual sal_Bool        GetWordIndices( USHORT nPara, USHORT nIndex, USHORT& nStart, USHORT& nEnd ) const;
    virtual sal_Bool        GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, USHORT nPara, USHORT nIndex ) const;
    virtual USHORT          GetLineCount( USHORT nPara ) const;
    virtual USHORT          GetLineLen( USHORT nPara, USHORT nLine ) const;
    virtual sal_Bool        Delete( const ESelection& );
    virtual sal_Bool        InsertText( const String&, const ESelection& );
    virtual sal_Bool        QuickFormatDoc( BOOL bFull=FALSE );
    virtual USHORT          GetDepth( USHORT nPara ) const;
    virtual sal_Bool        SetDepth( USHORT nPara, USHORT nNewDepth );

};

#endif


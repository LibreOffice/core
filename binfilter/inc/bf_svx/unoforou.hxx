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

#ifndef _SVX_UNOFOROU_HXX
#define _SVX_UNOFOROU_HXX

#ifndef _SVX_UNOEDSRC_HXX
#include <bf_svx/unoedsrc.hxx>
#endif

#include <bf_svx/editdata.hxx>
namespace binfilter {

class Outliner;
class SdrObject;

//	SvxOutlinerForwarder - SvxTextForwarder fuer Outliner

class SvxOutlinerForwarder : public SvxTextForwarder
{
private:
    Outliner&			rOutliner;
    SdrObject*			pSdrObject;

    /** this pointer may be null or point to an item set for the attribs of
        the selection maAttribsSelection */
    mutable SfxItemSet* mpAttribsCache;

    /** if we have a chached attribute item set, this is the selection of it */
    mutable ESelection	maAttribCacheSelection;

    /** this pointer may be null or point to an item set for the paragraph
        mnParaAttribsCache */
    mutable SfxItemSet*	mpParaAttribsCache;

    /** if we have a cached para attribute item set, this is the paragraph of it */
    mutable USHORT		mnParaAttribsCache;

public:
                        SvxOutlinerForwarder( Outliner& rOutl, SdrObject* pSdrObj );
    virtual				~SvxOutlinerForwarder();

    virtual USHORT		GetParagraphCount() const;
    virtual USHORT		GetTextLen( USHORT nParagraph ) const;
    virtual String		GetText( const ESelection& rSel ) const;
    virtual SfxItemSet	GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib = 0 ) const;
    virtual	SfxItemSet	GetParaAttribs( USHORT nPara ) const;
    virtual void		SetParaAttribs( USHORT nPara, const SfxItemSet& rSet );
    virtual void		GetPortions( USHORT nPara, SvUShorts& rList ) const;

    virtual USHORT		GetItemState( const ESelection& rSel, USHORT nWhich ) const;
    virtual USHORT		GetItemState( USHORT nPara, USHORT nWhich ) const;

    virtual void		QuickInsertText( const String& rText, const ESelection& rSel );
    virtual void		QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    virtual void		QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    virtual void		QuickInsertLineBreak( const ESelection& rSel );

    virtual SfxItemPool* GetPool() const;

    virtual XubString    CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor );
    virtual BOOL		IsValid() const;

    Outliner&			GetOutliner() const { return rOutliner; }

    virtual LanguageType 	GetLanguage( USHORT, USHORT ) const;
    virtual USHORT			GetFieldCount( USHORT nPara ) const;
    virtual EFieldInfo		GetFieldInfo( USHORT nPara, USHORT nField ) const;
    virtual EBulletInfo     GetBulletInfo( USHORT nPara ) const;
    virtual Rectangle		GetCharBounds( USHORT nPara, USHORT nIndex ) const;
    virtual Rectangle		GetParaBounds( USHORT nPara ) const;
    virtual MapMode			GetMapMode() const;
    virtual OutputDevice*	GetRefDevice() const;
    virtual sal_Bool		GetIndexAtPoint( const Point&, USHORT& nPara, USHORT& nIndex ) const;
    virtual sal_Bool		GetWordIndices( USHORT nPara, USHORT nIndex, USHORT& nStart, USHORT& nEnd ) const;
    virtual sal_Bool 		GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, USHORT nPara, USHORT nIndex ) const;
    virtual USHORT			GetLineCount( USHORT nPara ) const;
    virtual USHORT			GetLineLen( USHORT nPara, USHORT nLine ) const;
    virtual sal_Bool		Delete( const ESelection& );
    virtual sal_Bool		InsertText( const String&, const ESelection& );
    virtual sal_Bool		QuickFormatDoc( BOOL bFull=FALSE );
    virtual USHORT			GetDepth( USHORT nPara ) const;
    virtual sal_Bool		SetDepth( USHORT nPara, USHORT nNewDepth );

    /* this method flushes internal caches for this forwarder */
    void				flushCache();
};

}//end of namespace binfilter
#endif


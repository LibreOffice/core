/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _EDITOBJ_HXX
#define _EDITOBJ_HXX

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <rsc/rscsfx.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_svx/eeitem.hxx>
namespace binfilter {

class SfxItemPool;
class SfxStyleSheetPool; 

DBG_NAMEEX( EE_EditTextObject )//STRIP008

class SvxFieldItem;
class EECharAttribArray; 

#define EDTOBJ_SETTINGS_ULITEMSUMMATION		0x00000001
#define EDTOBJ_SETTINGS_ULITEMFIRSTPARA		0x00000002

class EditTextObject
{
private:
    USHORT				nWhich;

protected:
                        EditTextObject( USHORT nWhich );
                        EditTextObject( const EditTextObject& r );

    virtual void		StoreData( SvStream& rOStream ) const;
    virtual void		CreateData( SvStream& rIStream );

public:
    virtual				~EditTextObject();

    USHORT				Which() const { return nWhich; }

    virtual USHORT		GetUserType() const;	// Fuer OutlinerMode, der kann das aber nicht kompatibel speichern
    virtual void		SetUserType( USHORT n );


    virtual	BOOL		IsVertical() const;
    virtual
    void				SetVertical( BOOL bVertical );

    virtual USHORT		GetVersion() const;	// Solange der Outliner keine Recordlaenge speichert

    virtual EditTextObject*	Clone() const = 0;

    BOOL					Store( SvStream& rOStream ) const;
    static EditTextObject*	Create( SvStream& rIStream,
                                SfxItemPool* pGlobalTextObjectPool = 0 );

    // Zur 5.1 hat sich die Bedeutung des LRSpaceItems fuer den Outliner geaendert...
    virtual void			AdjustImportedLRSpaceItems( BOOL bTurnOfBullets );
    virtual	void 			PrepareStore( SfxStyleSheetPool* pStyleSheetPool );
    virtual	void 			FinishStore();
    virtual	void 			FinishLoad( SfxStyleSheetPool* pStyleSheetPool );

    virtual USHORT		GetParagraphCount() const;

    virtual XubString	GetText( USHORT nParagraph ) const;
    virtual void		Insert( const EditTextObject& rObj, USHORT nPara );

    virtual void		ClearPortionInfo();




    virtual	void		MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart = EE_CHAR_START, USHORT nEnd = EE_CHAR_END );

    virtual BOOL		HasField( TypeId aType = NULL ) const;

    virtual SfxItemSet	GetParaAttribs( USHORT nPara ) const;

    virtual void		GetStyleSheet( USHORT nPara, XubString& rName, SfxStyleFamily& eFamily ) const;
    virtual void		SetStyleSheet( USHORT nPara, const XubString& rName, const SfxStyleFamily& eFamily );
    virtual BOOL		ChangeStyleSheets( 	const XubString& rOldName, SfxStyleFamily eOldFamily,
                                            const XubString& rNewName, SfxStyleFamily eNewFamily );
    virtual void		ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );
};

}//end of namespace binfilter
#endif	// _EDITOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef _EDITUNDO_HXX
#define _EDITUNDO_HXX

#include <editdoc.hxx>
#include <editund2.hxx>
#include <editdata.hxx>
namespace binfilter {

#define UNDO_NOACTION			0
#define UNDO_NEWUNDO			1
#define UNDO_UNDOSDELETED		2
#define UNDO_EMPTYGROUPDELETED	3
#define UNDO_INVALIDEND			4

class ImpEditEngine;
class EditView;

// -----------------------------------------------------------------------
// EditUndoInsertChars
// ------------------------------------------------------------------------
class EditUndoInsertChars : public EditUndo
{
private:
    EPaM			aEPaM;
    String			aText;

public:
                    TYPEINFO();
                    EditUndoInsertChars( ImpEditEngine* pImpEE, const EPaM& rEPaM, const String& rStr );

    const EPaM&		GetEPaM() { return aEPaM; }
    String&			GetStr() { return aText; }

    virtual void 	Undo();
    virtual void 	Redo();
    virtual void 	Repeat();

    virtual BOOL	Merge( SfxUndoAction *pNextAction );
};

// -----------------------------------------------------------------------
// EditUndoRemoveChars
// ------------------------------------------------------------------------

// -----------------------------------------------------------------------
// EditUndoInsertFeature
// ------------------------------------------------------------------------
class EditUndoInsertFeature : public EditUndo
{
private:
    EPaM			aEPaM;
    SfxPoolItem*	pFeature;

public:
                    EditUndoInsertFeature( ImpEditEngine* pImpEE, const EPaM& rEPaM,
                                            const SfxPoolItem& rFeature);
                    ~EditUndoInsertFeature();

    virtual void 	Undo();
    virtual void 	Redo();
    virtual void 	Repeat();
};

// -----------------------------------------------------------------------
// EditUndoMoveParagraphs
// ------------------------------------------------------------------------

// -----------------------------------------------------------------------
// EditUndoSetStyleSheet
// ------------------------------------------------------------------------
class EditUndoSetStyleSheet: public EditUndo
{
private:
    USHORT			nPara;
    XubString		aPrevName;
    XubString		aNewName;
    SfxStyleFamily	ePrevFamily;
    SfxStyleFamily	eNewFamily;
    SfxItemSet		aPrevParaAttribs;

public:
                    
                    EditUndoSetStyleSheet( ImpEditEngine* pImpEE, USHORT nPara,
                        const XubString& rPrevName, SfxStyleFamily ePrevFamily,
                        const XubString& rNewName, SfxStyleFamily eNewFamily,
                        const SfxItemSet& rPrevParaAttribs );
                    ~EditUndoSetStyleSheet();

    virtual void	Undo();
    virtual void	Redo();
    virtual void	Repeat();
};

// -----------------------------------------------------------------------
// EditUndoSetParaAttribs
// ------------------------------------------------------------------------
class EditUndoSetParaAttribs: public EditUndo
{
private:
    USHORT			nPara;
    SfxItemSet		aPrevItems;
    SfxItemSet		aNewItems;

public:
                    EditUndoSetParaAttribs( ImpEditEngine* pImpEE, USHORT nPara, const SfxItemSet& rPrevItems, const SfxItemSet& rNewItems );
                    ~EditUndoSetParaAttribs();

    virtual void	Undo();
    virtual void	Redo();
    virtual void	Repeat();
};

// -----------------------------------------------------------------------
// EditUndoSetAttribs
// ------------------------------------------------------------------------

// -----------------------------------------------------------------------
// EditUndoTransliteration
// ------------------------------------------------------------------------

// -----------------------------------------------------------------------
// EditUndoMarkSelection
// ------------------------------------------------------------------------


}//end of namespace binfilter
#endif // _EDITUNDO_HXX

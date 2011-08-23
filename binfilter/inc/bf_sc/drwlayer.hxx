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

#ifndef SC_DRWLAYER_HXX
#define SC_DRWLAYER_HXX

#include <vcl/graph.hxx>
#include <bf_svx/fmmodel.hxx>
#include <bf_so3/svstor.hxx>
#include "global.hxx"
namespace binfilter {

class ScDocument;
class SfxViewShell;
class ScDrawObjData;
class ScIMapInfo;
class IMapObject;
class ScMarkData;
class SdrOle2Obj;

// -----------------------------------------------------------------------

class ScTabDeletedHint : public SfxHint
{
private:
    USHORT	nTab;
public:
            TYPEINFO();
    virtual	~ScTabDeletedHint();

    USHORT	GetTab()	{ return nTab; }
};

class ScTabSizeChangedHint : public SfxHint
{
private:
    USHORT	nTab;
public:
            TYPEINFO();
            ScTabSizeChangedHint( USHORT nTabNo = USHRT_MAX );
    virtual	~ScTabSizeChangedHint();

    USHORT	GetTab()	{ return nTab; }
};

// -----------------------------------------------------------------------


class ScDrawLayer: public FmFormModel
{
private:
    SvStorageRef	xPictureStorage;
    String			aName;
    ScDocument*		pDoc;
    SdrUndoGroup*	pUndoGroup;
    BOOL			bRecording;
    BOOL			bAdjustEnabled;
    BOOL			bHyphenatorSet;

private:
    void			MoveAreaTwips( USHORT nTab, const Rectangle& rArea, const Point& rMove,
                                const Point& rTopLeft );

public:
                    ScDrawLayer( ScDocument* pDocument, const String& rName );
    virtual			~ScDrawLayer();

    virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);
    virtual void	SetChanged( FASTBOOL bFlg = TRUE );

    virtual SvStream* GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const;


    void			ReleasePictureStorage();


    void			ScAddPage( USHORT nTab );
    void			ScRenamePage( USHORT nTab, const String& rNewName );
                    // inkl. Inhalt, bAlloc=FALSE -> nur Inhalt

    ScDocument*		GetDocument() const { return pDoc; }

    void			UpdateBasic();				// DocShell-Basic in DrawPages setzen

    void			Load( SvStream& rStream );
    void			Store( SvStream& rStream ) const;

    BOOL			GetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const;

                    //		automatische Anpassungen

    void			EnableAdjust( BOOL bSet = TRUE )	{ bAdjustEnabled = bSet; }

    BOOL			IsRecording()			{ return bRecording; }
    void			AddCalcUndo( SdrUndoAction* pUndo );

    void			WidthChanged( USHORT nTab, USHORT nCol, long nDifTwips );
    void			HeightChanged( USHORT nTab, USHORT nRow, long nDifTwips );

    BOOL			HasObjectsInRows( USHORT nTab, USHORT nStartRow, USHORT nEndRow );



    void			SetPageSize( USHORT nPageNo, const Size& rSize );

                    //	GetVisibleName: name for navigator etc: GetPersistName or GetName
                    //	(ChartListenerCollection etc. must use GetPersistName directly)

    SdrObject*		GetNamedObject( const String& rName, USHORT nId, USHORT& rFoundTab ) const;
                    // if pnCounter != NULL, the search for a name starts with this index + 1,
                    // and the index really used is returned.

    // Verankerung setzen und ermitteln
    static void		SetAnchor( SdrObject*, ScAnchorType );
    static ScAnchorType	GetAnchor( const SdrObject* );

    // Positionen fuer Detektivlinien
    static ScDrawObjData* GetObjData( SdrObject* pObj, BOOL bCreate=FALSE );

    // Image-Map
    static ScIMapInfo* GetIMapInfo( SdrObject* pObj );


private:
    static SvPersist* pGlobalDrawPersist;			// fuer AllocModel
public:
};


} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

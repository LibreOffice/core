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

#ifndef SC_COLROWBAR_HXX
#define SC_COLROWBAR_HXX

#ifndef SC_HDRCONT_HXX
#include "hdrcont.hxx"
#endif

#ifndef SC_VIEWDATA_HXX
#include "viewdata.hxx"
#endif
namespace binfilter {

class ScHeaderFunctionSet;
class ScHeaderSelectionEngine;

// ---------------------------------------------------------------------------


class ScColBar : public ScHeaderControl
{
    ScViewData*				 pViewData;
    ScHSplitPos				 eWhich;
    ScHeaderFunctionSet*	 pFuncSet;
    ScHeaderSelectionEngine* pSelEngine;

public:
                ScColBar( Window* pParent, ScViewData* pData, ScHSplitPos eWhichPos,
                            ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng );
                ~ScColBar();

    virtual USHORT		GetPos();
    virtual USHORT		GetEntrySize( USHORT nEntryNo );
    virtual String 		GetEntryText( USHORT nEntryNo ){DBG_BF_ASSERT(0, "STRIP");String aStr; return aStr;} //STRIP001 virtual String 		GetEntryText( USHORT nEntryNo );

    virtual void		SetEntrySize( USHORT nPos, USHORT nNewSize ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void		SetEntrySize( USHORT nPos, USHORT nNewSize );
    virtual void		HideEntries( USHORT nStart, USHORT nEnd ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void		HideEntries( USHORT nStart, USHORT nEnd );

    virtual void		SetMarking( BOOL bSet );
    virtual void		SelectWindow(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void		SelectWindow();
    virtual BOOL		IsDisabled(){DBG_BF_ASSERT(0, "STRIP");return FALSE;} //STRIP001 virtual BOOL		IsDisabled();
    virtual BOOL		ResizeAllowed(){DBG_BF_ASSERT(0, "STRIP");return FALSE;} //STRIP001 virtual BOOL		ResizeAllowed();

    virtual void		DrawInvert( long nDragPos ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void		DrawInvert( long nDragPos );

    virtual String		GetDragHelp( long nVal ){DBG_BF_ASSERT(0, "STRIP"); String aStr; return aStr;}//STRIP001 virtual String		GetDragHelp( long nVal );
};


class ScRowBar : public ScHeaderControl
{
    ScViewData*				 pViewData;
    ScVSplitPos				 eWhich;
    ScHeaderFunctionSet*	 pFuncSet;
    ScHeaderSelectionEngine* pSelEngine;

public:
                ScRowBar( Window* pParent, ScViewData* pData, ScVSplitPos eWhichPos,
                            ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng );
                ~ScRowBar();

    virtual USHORT		GetPos();
    virtual USHORT		GetEntrySize( USHORT nEntryNo );
    virtual String 		GetEntryText( USHORT nEntryNo ){DBG_BF_ASSERT(0, "STRIP"); String s; return s;}//STRIP001 virtual String 		GetEntryText( USHORT nEntryNo );

    virtual USHORT		GetHiddenCount( USHORT nEntryNo ){DBG_BF_ASSERT(0, "STRIP"); return 0;}//STRIP001 virtual USHORT		GetHiddenCount( USHORT nEntryNo );	// nur fuer Zeilen

    virtual void		SetEntrySize( USHORT nPos, USHORT nNewSize ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void		SetEntrySize( USHORT nPos, USHORT nNewSize );
    virtual void		HideEntries( USHORT nStart, USHORT nEnd ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void		HideEntries( USHORT nStart, USHORT nEnd );

    virtual void		SetMarking( BOOL bSet );
    virtual void		SelectWindow(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void		SelectWindow();
    virtual BOOL		IsDisabled(){DBG_BF_ASSERT(0, "STRIP"); return FALSE;}//STRIP001 virtual BOOL		IsDisabled();
    virtual BOOL		ResizeAllowed(){DBG_BF_ASSERT(0, "STRIP"); return FALSE;}//STRIP001 virtual BOOL		ResizeAllowed();

    virtual void		DrawInvert( long nDragPos ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void		DrawInvert( long nDragPos );

    virtual String		GetDragHelp( long nVal ){DBG_BF_ASSERT(0, "STRIP"); String s; return s;}//STRIP001 virtual String		GetDragHelp( long nVal );
};



} //namespace binfilter
#endif



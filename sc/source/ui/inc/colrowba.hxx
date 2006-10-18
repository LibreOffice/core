/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colrowba.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-10-18 12:27:21 $
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

#ifndef SC_COLROWBAR_HXX
#define SC_COLROWBAR_HXX

#ifndef SC_HDRCONT_HXX
#include "hdrcont.hxx"
#endif

#ifndef SC_VIEWDATA_HXX
#include "viewdata.hxx"
#endif

class ScHeaderFunctionSet;
class ScHeaderSelectionEngine;

// ---------------------------------------------------------------------------


class ScColBar : public ScHeaderControl
{
    ScViewData*              pViewData;
    ScHSplitPos              eWhich;
    ScHeaderFunctionSet*     pFuncSet;
    ScHeaderSelectionEngine* pSelEngine;

public:
                ScColBar( Window* pParent, ScViewData* pData, ScHSplitPos eWhichPos,
                            ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng );
                ~ScColBar();

    virtual SCCOLROW    GetPos();
    virtual USHORT      GetEntrySize( SCCOLROW nEntryNo );
    virtual String      GetEntryText( SCCOLROW nEntryNo );

    virtual BOOL        IsLayoutRTL();                      // only for columns

    virtual void        SetEntrySize( SCCOLROW nPos, USHORT nNewSize );
    virtual void        HideEntries( SCCOLROW nStart, SCCOLROW nEnd );

    virtual void        SetMarking( BOOL bSet );
    virtual void        SelectWindow();
    virtual BOOL        IsDisabled();
    virtual BOOL        ResizeAllowed();

    virtual void        DrawInvert( long nDragPos );

    virtual String      GetDragHelp( long nVal );

            BOOL        UseNumericHeader() const;
};


class ScRowBar : public ScHeaderControl
{
    ScViewData*              pViewData;
    ScVSplitPos              eWhich;
    ScHeaderFunctionSet*     pFuncSet;
    ScHeaderSelectionEngine* pSelEngine;

public:
                ScRowBar( Window* pParent, ScViewData* pData, ScVSplitPos eWhichPos,
                            ScHeaderFunctionSet* pFunc, ScHeaderSelectionEngine* pEng );
                ~ScRowBar();

    virtual SCCOLROW    GetPos();
    virtual USHORT      GetEntrySize( SCCOLROW nEntryNo );
    virtual String      GetEntryText( SCCOLROW nEntryNo );

    virtual BOOL        IsMirrored();                       // only for columns
    virtual SCROW       GetHiddenCount( SCROW nEntryNo );   // only for columns

    virtual void        SetEntrySize( SCCOLROW nPos, USHORT nNewSize );
    virtual void        HideEntries( SCCOLROW nStart, SCCOLROW nEnd );

    virtual void        SetMarking( BOOL bSet );
    virtual void        SelectWindow();
    virtual BOOL        IsDisabled();
    virtual BOOL        ResizeAllowed();

    virtual void        DrawInvert( long nDragPos );

    virtual String      GetDragHelp( long nVal );
};



#endif



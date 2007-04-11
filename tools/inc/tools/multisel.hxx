/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: multisel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:14:17 $
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
#ifndef _SV_MULTISEL_HXX
#define _SV_MULTISEL_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

//------------------------------------------------------------------

#ifdef _SV_MULTISEL_CXX
DECLARE_LIST( ImpSelList, Range* )
#else
#define ImpSelList List
#endif

#define SFX_ENDOFSELECTION      CONTAINER_ENTRY_NOTFOUND

//------------------------------------------------------------------

// ------------------
// - MultiSelection -
// ------------------

class TOOLS_DLLPUBLIC MultiSelection
{
private:
    ImpSelList      aSels;      // array of SV-selections
    Range           aTotRange;  // total range of indexes
    ULONG           nCurSubSel; // index in aSels of current selected index
    long            nCurIndex;  // current selected entry
    ULONG           nSelCount;  // number of selected indexes
    BOOL            bInverseCur;// inverse cursor
    BOOL            bCurValid;  // are nCurIndex and nCurSubSel valid
    BOOL            bSelectNew; // auto-select newly inserted indexes

#ifdef _SV_MULTISEL_CXX
    TOOLS_DLLPRIVATE void           ImplClear();
    TOOLS_DLLPRIVATE ULONG          ImplFindSubSelection( long nIndex ) const;
    TOOLS_DLLPRIVATE BOOL           ImplMergeSubSelections( ULONG nPos1, ULONG nPos2 );
    TOOLS_DLLPRIVATE long           ImplFwdUnselected();
    TOOLS_DLLPRIVATE long           ImplBwdUnselected();
#endif

public:
                    MultiSelection();
                    MultiSelection( const MultiSelection& rOrig );
                    MultiSelection( const Range& rRange );
                    MultiSelection( const UniString& rString,
                                    sal_Unicode cRange = '-',
                                    sal_Unicode cSep = ';' );
                    ~MultiSelection();

    MultiSelection& operator= ( const MultiSelection& rOrig );
    BOOL            operator== ( MultiSelection& rOrig );
    BOOL            operator!= ( MultiSelection& rOrig )
                        { return !operator==( rOrig ); }
    BOOL            operator !() const
                        { return nSelCount == 0; }

    void            SelectAll( BOOL bSelect = TRUE );
    BOOL            Select( long nIndex, BOOL bSelect = TRUE );
    void            Select( const Range& rIndexRange, BOOL bSelect = TRUE );
    BOOL            IsSelected( long nIndex ) const;
    BOOL            IsAllSelected() const
                        { return nSelCount == ULONG(aTotRange.Len()); }
    long            GetSelectCount() const { return nSelCount; }

    void            SetTotalRange( const Range& rTotRange );
    void            Insert( long nIndex, long nCount = 1 );
    void            Remove( long nIndex );
    void            Append( long nCount = 1 );

    const Range&    GetTotalRange() const { return aTotRange; }
    BOOL            IsCurValid() const { return bCurValid; }
    long            GetCurSelected() const { return nCurIndex; }
    long            FirstSelected( BOOL bInverse = FALSE );
    long            LastSelected();
    long            NextSelected();
    long            PrevSelected();

    ULONG           GetRangeCount() const { return aSels.Count(); }
    const Range&    GetRange( ULONG nRange ) const { return *(const Range*)aSels.GetObject(nRange); }
};

#endif  // _SV_MULTISEL_HXX

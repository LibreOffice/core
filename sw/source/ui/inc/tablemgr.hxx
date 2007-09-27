/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablemgr.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:11:56 $
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
#ifndef _TABLEMGR_HXX
#define _TABLEMGR_HXX

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif
#ifndef _TABCOL_HXX
#include "tabcol.hxx"
#endif

class SwFrmFmt;
class SwWrtShell;
class Window;
class SfxItemSet;
class SwFlyFrmFmt;

namespace com { namespace sun { namespace star {
    namespace frame {
        class XModel; }
    namespace chart2 {
    namespace data {
        class XDataProvider; } }
}}}

const SwTwips lAutoWidth = INVALID_TWIPS;
const char cParaDelim = 0x0a;

class SW_DLLPUBLIC SwTableFUNC
{
    SwFrmFmt    *pFmt;
    SwWrtShell  *pSh;
    BOOL        bCopy;
    SwTabCols   aCols;

private:
    SW_DLLPRIVATE int GetRightSeparator(int nNum) const;

public:
    inline SwTableFUNC(SwFrmFmt &);
           SwTableFUNC(SwWrtShell *pShell, BOOL bCopyFmt = FALSE);
           ~SwTableFUNC();

    void    InitTabCols();
    void    ColWidthDlg(Window *pParent );
    SwTwips GetColWidth(USHORT nNum) const;
    SwTwips GetMaxColWidth(USHORT nNum) const;
    void    SetColWidth(USHORT nNum, SwTwips nWidth );
    USHORT  GetColCount() const;
    USHORT  GetCurColNum() const;

    BOOL IsTableSelected() const { return pFmt != 0; }

    const SwFrmFmt *GetTableFmt() const { return pFmt; }

    SwWrtShell* GetShell() const { return pSh; }

    // @deprecated
    void UpdateChart();

    /// @return the XModel of the newly inserted chart if successfull
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        InsertChart( ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider > &rxDataProvider, sal_Bool bFillWithData, const rtl::OUString &rCellRange, SwFlyFrmFmt** ppFlyFrmFmt = 0 );
};

inline SwTableFUNC::SwTableFUNC(SwFrmFmt &rFmt) :
    pFmt(&rFmt),
    pSh(0),
    bCopy(FALSE)
{
}

#endif

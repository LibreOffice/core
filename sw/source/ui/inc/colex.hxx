/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colex.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:55:11 $
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
#ifndef _COLEX_HXX
#define _COLEX_HXX

#ifndef _SVX_PAGECTRL_HXX //autogen
#include <svx/pagectrl.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FRMATR_HXX
#include "frmatr.hxx"
#endif

class SwColMgr;
class SfxItemSet;

/* -----------------------------08.02.2002 11:32------------------------------

 ---------------------------------------------------------------------------*/
class SW_DLLPUBLIC SwPageExample : public SvxPageWindow
{
public:
    SwPageExample(Window* pPar, const ResId& rResId ) :
                                SvxPageWindow(pPar, rResId )
                                {SetSize(Size(11907, 16433));/*DIN A4*/}

    void UpdateExample( const SfxItemSet& rSet );
};
/* -----------------------------08.02.2002 11:34------------------------------

 ---------------------------------------------------------------------------*/
class SwTextGridItem;
class SW_DLLPUBLIC SwPageGridExample : public SwPageExample
{
    SwTextGridItem*     pGridItem;
    sal_Bool            m_bVertical;
protected:
    virtual void DrawPage( const Point& rPoint,
                           const BOOL bSecond,
                           const BOOL bEnabled );
public:
    SwPageGridExample(Window* pPar, const ResId& rResId ) :
                                SwPageExample(pPar, rResId ),
                                pGridItem(0),
                                m_bVertical(sal_False){}
    ~SwPageGridExample();
    void UpdateExample( const SfxItemSet& rSet );
};
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwColExample : public SwPageExample
{
    SwColMgr*   pColMgr;
protected:
    virtual void DrawPage( const Point& rPoint,
                           const BOOL bSecond,
                           const BOOL bEnabled );

public:
        SwColExample(Window* pPar, const ResId& rResId ) :
                                SwPageExample(pPar, rResId ),
                                pColMgr(0){}

    using SwPageExample::UpdateExample;
    void UpdateExample( const SfxItemSet& rSet, SwColMgr* pMgr  )
        {   pColMgr = pMgr;
            SwPageExample::UpdateExample(rSet);
        }
};

/*-----------------25.10.96 08.23-------------------

--------------------------------------------------*/
class SW_DLLPUBLIC SwColumnOnlyExample : public Window
{
private:
    Size        m_aWinSize;

    Size        m_aFrmSize;
    SwFmtCol    m_aCols;

protected:
    virtual void Paint( const Rectangle& rRect );

public:
    SwColumnOnlyExample( Window* , const ResId& );

    void        SetColumns(const SwFmtCol& rCol);

};

#endif // _COLEX_HXX

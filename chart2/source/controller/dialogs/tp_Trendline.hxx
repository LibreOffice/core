/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_Trendline.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:52:36 $
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
#ifndef _CHART2_TP_TRENDLINE_HXX
#define _CHART2_TP_TRENDLINE_HXX

#include "res_Trendline.hxx"

#include <sfx2/tabdlg.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

class TrendlineTabPage : public SfxTabPage
{
public:
    TrendlineTabPage ( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~TrendlineTabPage ();

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual BOOL FillItemSet( SfxItemSet& rOutAttrs );
    virtual void Reset( const SfxItemSet& rInAttrs );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

private:
    TrendlineResources   m_aTrendlineResources;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

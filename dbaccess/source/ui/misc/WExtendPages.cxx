/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WExtendPages.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:52:12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#include "WExtendPages.hxx"
#endif
#ifndef DBAUI_RTFREADER_HXX
#include "RtfReader.hxx"
#endif
#ifndef DBAUI_HTMLREADER_HXX
#include "HtmlReader.hxx"
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif

using namespace dbaui;
//========================================================================
SvParser* OWizHTMLExtend::createReader(sal_Int32 _nRows)
{
    return new OHTMLReader(*m_pParserStream,
                            _nRows,
                            m_pParent->GetColumnPositions(),
                            m_pParent->GetFormatter(),
                            m_pParent->GetFactory(),
                            m_pParent->getDestVector(),
                            m_pParent->getTypeInfo(),
                            m_pParent->shouldCreatePrimaryKey());
}
//========================================================================
SvParser* OWizRTFExtend::createReader(sal_Int32 _nRows)
{
    return new ORTFReader(*m_pParserStream,
                            _nRows,
                            m_pParent->GetColumnPositions(),
                            m_pParent->GetFormatter(),
                            m_pParent->GetFactory(),
                            m_pParent->getDestVector(),
                            m_pParent->getTypeInfo(),
                            m_pParent->shouldCreatePrimaryKey());
}
//========================================================================
OWizNormalExtend::OWizNormalExtend(Window* pParent) : OWizTypeSelect( pParent )
{
    EnableAuto(sal_False);
    Size aflSize( m_flColumns.GetSizePixel() );
    Point aPos(m_flColumns.GetPosPixel());

    m_flColumns.SetPosSizePixel(aPos, aflSize );

    sal_Int32 nHeight = m_lbColumnNames.GetSizePixel().Height() +6;
    aPos = m_aTypeControl.GetPosPixel();
    Size aNewSize(m_aTypeControl.GetSizePixel().Width(),nHeight - aPos.Y()-6);

    aflSize = m_aTypeControl.GetSizePixel();
    m_aTypeControl.SetPosSizePixel(aPos,aNewSize);
}
// -----------------------------------------------------------------------------
SvParser* OWizNormalExtend::createReader(sal_Int32 /*_nRows*/)
{
    return NULL;
}
// -----------------------------------------------------------------------------




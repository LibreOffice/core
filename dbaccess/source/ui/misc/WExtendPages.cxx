/*************************************************************************
 *
 *  $RCSfile: WExtendPages.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 15:07:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
void OWizHTMLExtend::fillColumnList(sal_uInt32 nRows)
{
    sal_uInt32 nTell = m_pParserStream->Tell(); // verändert vielleicht die Position des Streams
    OHTMLReader *pReader = new OHTMLReader(*m_pParserStream,
                                            nRows,
                                            m_pParent->GetColumnPositions(),
                                            m_pParent->GetFormatter(),
                                            m_pParent->GetFactory());
    pReader->AddRef();
    pReader->CallParser();
    pReader->SetColumnTypes(m_pParent->getDestVector(),m_pParent->getTypeInfo());
    pReader->ReleaseRef();
    m_pParserStream->Seek(nTell);
}
//========================================================================
void OWizRTFExtend::fillColumnList(sal_uInt32 nRows)
{
    sal_uInt32 nTell = m_pParserStream->Tell(); // verändert vielleicht die Position des Streams
    ORTFReader *pReader = new ORTFReader(*m_pParserStream,
                                            nRows,
                                            m_pParent->GetColumnPositions(),
                                            m_pParent->GetFormatter(),
                                            m_pParent->GetFactory());
    pReader->AddRef();
    pReader->CallParser();
    pReader->SetColumnTypes(m_pParent->getDestVector(),m_pParent->getTypeInfo());
    pReader->ReleaseRef();
    m_pParserStream->Seek(nTell);
}
//========================================================================
OWizNormalExtend::OWizNormalExtend(Window* pParent) : OWizTypeSelect( pParent)
{
    EnableAuto(sal_False);
    Size aSize( GetSizePixel() );
    Size agpSize( m_gpColumns.GetSizePixel() );
    Point aPos(m_gpColumns.GetPosPixel());

    sal_Int32 nHeight = m_lbColumnNames.GetSizePixel().Height() +6;
    m_gpColumns.SetPosSizePixel(aPos,Size(agpSize.Width(),nHeight));

    aPos = m_aTypeControl.GetPosPixel();
    Size aNewSize(m_aTypeControl.GetSizePixel().Width(),nHeight - aPos.Y()-6);

    agpSize = m_aTypeControl.GetSizePixel();
    m_aTypeControl.SetPosSizePixel(aPos,aNewSize);
}
// -----------------------------------------------------------------------------




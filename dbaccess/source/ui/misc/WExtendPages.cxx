/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
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




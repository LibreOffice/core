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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "WExtendPages.hxx"
#include "RtfReader.hxx"
#include "HtmlReader.hxx"
#include "WCopyTable.hxx"

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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

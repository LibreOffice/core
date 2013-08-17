/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "WExtendPages.hxx"
#include "RtfReader.hxx"
#include "HtmlReader.hxx"
#include "WCopyTable.hxx"

using namespace dbaui;
using namespace com::sun::star;
SvParser* OWizHTMLExtend::createReader(sal_Int32 _nRows)
{
    return new OHTMLReader(*m_pParserStream,
                            _nRows,
                            m_pParent->GetColumnPositions(),
                            m_pParent->GetFormatter(),
                            m_pParent->GetComponentContext(),
                            m_pParent->getDestVector(),
                            m_pParent->getTypeInfo(),
                            m_pParent->shouldCreatePrimaryKey());
}

SvParser* OWizRTFExtend::createReader(sal_Int32 _nRows)
{
    return new ORTFReader(*m_pParserStream,
                            _nRows,
                            m_pParent->GetColumnPositions(),
                            m_pParent->GetFormatter(),
                            m_pParent->GetComponentContext(),
                            m_pParent->getDestVector(),
                            m_pParent->getTypeInfo(),
                            m_pParent->shouldCreatePrimaryKey());
}

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

SvParser* OWizNormalExtend::createReader(sal_Int32 /*_nRows*/)
{
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

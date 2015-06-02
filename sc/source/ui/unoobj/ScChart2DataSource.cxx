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

#include "ScChart2DataSource.hxx"

#include "vcl/svapp.hxx"
#include "svl/smplhint.hxx"

#include "document.hxx"
#include "miscuno.hxx"

SC_SIMPLE_SERVICE_INFO( ScChart2DataSource, "ScChart2DataSource",
        "com.sun.star.chart2.data.DataSource")

using namespace com::sun::star;

ScChart2DataSource::ScChart2DataSource( ScDocument* pDoc)
    : m_pDocument( pDoc)
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
}

ScChart2DataSource::~ScChart2DataSource()
{
    SolarMutexGuard g;

    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}

void ScChart2DataSource::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        m_pDocument = NULL;
    }
}

uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> > SAL_CALL
ScChart2DataSource::getDataSequences() throw ( uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    LabeledList::const_iterator aItr(m_aLabeledSequences.begin());
    LabeledList::const_iterator aEndItr(m_aLabeledSequences.end());

    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aRet(m_aLabeledSequences.size());

    sal_Int32 i = 0;
    while (aItr != aEndItr)
    {
        aRet[i] = *aItr;
        ++i;
        ++aItr;
    }

    return aRet;
}

void ScChart2DataSource::AddLabeledSequence(const uno::Reference < chart2::data::XLabeledDataSequence >& xNew)
{
    m_aLabeledSequences.push_back(xNew);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

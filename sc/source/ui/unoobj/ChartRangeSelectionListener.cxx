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

#include "ChartRangeSelectionListener.hxx"

#include <com/sun/star/chart2/data/XRangeHighlighter.hpp>

#include <sfx2/viewfrm.hxx>
#include "tabvwsh.hxx"
#include "unonames.hxx"
#include "miscuno.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

SC_SIMPLE_SERVICE_INFO( ScChartRangeSelectionListener, "ScChartRangeSelectionListener",
                        SC_SERVICENAME_CHRANGEHILIGHT )

ScChartRangeSelectionListener::ScChartRangeSelectionListener( ScTabViewShell * pViewShell ) :
        ScChartRangeSelectionListener_Base( m_aMutex ),
        m_pViewShell( pViewShell )
{}

ScChartRangeSelectionListener::~ScChartRangeSelectionListener()
{}

// ____ XModifyListener ____
void SAL_CALL ScChartRangeSelectionListener::selectionChanged( const lang::EventObject& aEvent )
    throw (uno::RuntimeException, std::exception)
{
    Reference< chart2::data::XRangeHighlighter > xRangeHighlighter( aEvent.Source, uno::UNO_QUERY );
    if( xRangeHighlighter.is())
    {
        Sequence< chart2::data::HighlightedRange > aRanges( xRangeHighlighter->getSelectedRanges());

        // search the view on which the chart is active

        if( m_pViewShell )
        {
            m_pViewShell->DoChartSelection( aRanges );
        }
    }
}

// ____ XEventListener ____
void SAL_CALL ScChartRangeSelectionListener::disposing( const lang::EventObject& /*Source*/ )
    throw (uno::RuntimeException, std::exception)
{
}

// ____ WeakComponentImplHelperBase ____
void SAL_CALL ScChartRangeSelectionListener::disposing()
{
    m_pViewShell = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

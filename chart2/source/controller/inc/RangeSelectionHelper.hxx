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
#pragma once

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::chart2 { class XChartDocument; }

namespace com::sun::star {
    namespace sheet{
        class XRangeSelection;
        class XRangeSelectionListener;
    }
}

namespace chart
{

class RangeSelectionListenerParent;

class RangeSelectionHelper
{
public:
    explicit RangeSelectionHelper(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDocument );
    ~RangeSelectionHelper();

    bool hasRangeSelection();
    css::uno::Reference< css::sheet::XRangeSelection > const & getRangeSelection();
    void raiseRangeSelectionDocument();
    bool chooseRange(
        const OUString & aCurrentRange,
        const OUString & aUIString,
        RangeSelectionListenerParent & rListenerParent );
    void stopRangeListening( bool bRemoveListener = true );
    bool verifyCellRange( const OUString & rRangeStr );
    bool verifyArguments( const css::uno::Sequence< css::beans::PropertyValue >& rArguments );

private:
    css::uno::Reference< css::sheet::XRangeSelection >
        m_xRangeSelection;

    css::uno::Reference< css::chart2::XChartDocument >
        m_xChartDocument;

    css::uno::Reference< css::sheet::XRangeSelectionListener >
        m_xRangeSelectionListener;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

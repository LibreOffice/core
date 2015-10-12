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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTDROPTARGETHELPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTDROPTARGETHELPER_HXX

#include <svtools/transfer.hxx>

namespace com { namespace sun { namespace star {
namespace chart2 {
    class XChartDocument;
}
}}}

namespace chart
{

class ChartDropTargetHelper : public DropTargetHelper
{
public:
    explicit ChartDropTargetHelper(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >& rxDropTarget,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > & xChartDocument );
    virtual ~ChartDropTargetHelper();

protected:

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

private:
    // not available
    ChartDropTargetHelper();

    bool satisfiesPrerequisites() const;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDocument;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_CHARTDROPTARGETHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xdatapilotfieldgrouping.hxx>

#include <com/sun/star/sheet/XDataPilotFieldGrouping.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest {

void XDataPilotFieldGrouping::testCreateNameGroup()
{
    uno::Reference< sheet::XDataPilotFieldGrouping > xDataPilotFieldGrouping(init(),UNO_QUERY_THROW);
    uno::Reference< sheet::XDataPilotField > xDataPilotField( xDataPilotFieldGrouping, UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xNameAccess( xDataPilotField->getItems(), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xNameAccess->hasElements());

    uno::Sequence< OUString > aElements = xNameAccess->getElementNames();
    xDataPilotFieldGrouping->createNameGroup( aElements );
}

void XDataPilotFieldGrouping::testCreateDateGroup()
{
    uno::Reference< sheet::XDataPilotFieldGrouping > xDataPilotFieldGrouping(init(),UNO_QUERY_THROW);
    sheet::DataPilotFieldGroupInfo aGroupInfo;
    aGroupInfo.GroupBy = sheet::DataPilotFieldGroupBy::MONTHS;
    aGroupInfo.HasDateValues = true;
    xDataPilotFieldGrouping->createDateGroup(aGroupInfo);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

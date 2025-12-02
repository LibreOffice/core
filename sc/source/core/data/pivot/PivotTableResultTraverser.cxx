/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pivot/PivotTableResultTraverser.hxx>

#include <miscuno.hxx>
#include <unonames.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldLayoutMode.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#include <com/sun/star/sheet/XHierarchiesSupplier.hpp>
#include <com/sun/star/sheet/XLevelsSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotResults.hpp>
#include <com/sun/star/sheet/XDataPilotMemberResults.hpp>
#include <com/sun/star/sheet/MemberResultFlags.hpp>
#include <com/sun/star/sheet/XMembersSupplier.hpp>

using namespace css;

namespace sc
{
namespace pivot
{
uno::Any getMemberForLevel(uno::Reference<uno::XInterface> const& xLevel, OUString const& rName)
{
    uno::Reference<sheet::XMembersSupplier> xMembersSupplier(xLevel, uno::UNO_QUERY);
    if (xMembersSupplier.is())
    {
        uno::Reference<sheet::XMembersAccess> xMembers = xMembersSupplier->getMembers();
        if (xMembers.is())
        {
            if (xMembers->hasByName(rName))
                return xMembers->getByName(rName);
        }
    }
    return uno::Any();
}
}

// PivotTableResultTraverser

void PivotTableResultTraverser::traverse()
{
    uno::Reference<sheet::XDimensionsSupplier> xDimensionsSupplier(mrDPObject.GetSource());
    uno::Reference<container::XIndexAccess> xDimensions
        = new ScNameToIndexAccess(xDimensionsSupplier->getDimensions());

    for (sal_Int32 nDimension = 0; nDimension < xDimensions->getCount(); nDimension++)
    {
        uno::Reference<uno::XInterface> xDimension(xDimensions->getByIndex(nDimension),
                                                   uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDimensionPropertySet(xDimension, uno::UNO_QUERY);
        uno::Reference<sheet::XHierarchiesSupplier> xDimensionSupplier(xDimension, uno::UNO_QUERY);

        if (xDimensionPropertySet.is() && xDimensionSupplier.is())
        {
            sheet::DataPilotFieldOrientation eDimensionOrientation
                = ScUnoHelpFunctions::GetEnumProperty(xDimensionPropertySet, SC_UNO_DP_ORIENTATION,
                                                      sheet::DataPilotFieldOrientation_HIDDEN);
            mrVisitor.startDimension(nDimension, xDimension, eDimensionOrientation);

            if (!mrVisitor.filterOrientation(eDimensionOrientation))
            {
                uno::Reference<container::XIndexAccess> xHierarchies
                    = new ScNameToIndexAccess(xDimensionSupplier->getHierarchies());
                sal_Int32 nHierarchy = ScUnoHelpFunctions::GetLongProperty(xDimensionPropertySet,
                                                                           SC_UNO_DP_USEDHIERARCHY);
                if (nHierarchy >= xHierarchies->getCount())
                    nHierarchy = 0;

                uno::Reference<sheet::XLevelsSupplier> xLevelsSupplier(
                    xHierarchies->getByIndex(nHierarchy), uno::UNO_QUERY);
                if (xLevelsSupplier.is())
                {
                    uno::Reference<container::XIndexAccess> xLevels
                        = new ScNameToIndexAccess(xLevelsSupplier->getLevels());
                    if (xLevels.is())
                    {
                        for (sal_Int32 nLevel = 0; nLevel < xLevels->getCount(); nLevel++)
                        {
                            uno::Reference<uno::XInterface> xLevel(xLevels->getByIndex(nLevel),
                                                                   uno::UNO_QUERY);
                            uno::Reference<container::XNamed> xLevelName(xLevel, uno::UNO_QUERY);
                            uno::Reference<sheet::XDataPilotMemberResults> xLevelResult(
                                xLevel, uno::UNO_QUERY);

                            if (xLevelName.is() && xLevelResult.is())
                            {
                                mrVisitor.startLevel(nLevel, xLevel, xLevelName->getName());

                                const uno::Sequence<sheet::MemberResult> aSequence
                                    = xLevelResult->getResults();
                                sal_Int32 nCurrent = 0;
                                sal_Int32 nSize = aSequence.getLength();
                                for (sheet::MemberResult const& rMemberResult : aSequence)
                                {
                                    mrVisitor.memberResult(rMemberResult, nCurrent, nSize);
                                    nCurrent++;
                                }
                                mrVisitor.endLevel();
                            }
                        }
                    }
                }
            }
            mrVisitor.endDimension();
        }
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

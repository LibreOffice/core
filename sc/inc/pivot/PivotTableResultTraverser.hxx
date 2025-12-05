/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <dpobject.hxx>
#include <scdllapi.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/sheet/MemberResult.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

namespace sc
{
namespace pivot
{
SC_DLLPUBLIC css::uno::Any
getMemberForLevel(css::uno::Reference<css::uno::XInterface> const& xLevel, OUString const& rName);
}

/** Visitor interface for traversing the pivot table result tree structure.
 *
 * Defines the functions that are triggered with data, when the tree is traversed.
 **/
class SC_DLLPUBLIC PivotTableResultVisitor
{
public:
    virtual bool filterOrientation(css::sheet::DataPilotFieldOrientation eOrientation)
    {
        return eOrientation == css::sheet::DataPilotFieldOrientation_HIDDEN;
    }

    virtual void startDimension(sal_Int32 /*nDimensionIndex*/,
                                css::uno::Reference<css::uno::XInterface> const& /*xDimension*/,
                                css::sheet::DataPilotFieldOrientation /*eOrientation*/)
    {
    }

    virtual void endDimension() {}

    virtual void startLevel(sal_Int32 /*nLevelIndex*/,
                            css::uno::Reference<css::uno::XInterface> const& /*xLevel*/,
                            OUString const& /*rLevelName*/)
    {
    }

    virtual void endLevel() {}

    virtual void memberResult(css::sheet::MemberResult const& rResult, sal_Int32 nIndex,
                              sal_Int32 nSize)
        = 0;
};

/// Traverses the pivot table result tree and triggers visitiors functions with data
class SC_DLLPUBLIC PivotTableResultTraverser
{
private:
    ScDPObject& mrDPObject;
    PivotTableResultVisitor& mrVisitor;

public:
    PivotTableResultTraverser(ScDPObject& rDPObject, PivotTableResultVisitor& rVisitor)
        : mrDPObject(rDPObject)
        , mrVisitor(rVisitor)
    {
    }

    void traverse();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

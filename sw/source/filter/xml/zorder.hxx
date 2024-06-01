/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <IDocumentDrawModelAccess.hxx>

#include <o3tl/any.hxx>
#include <o3tl/unreachable.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

namespace sw
{
struct GetZOrderLayer
{
    GetZOrderLayer(IDocumentDrawModelAccess const& rIDDMA)
        : m_nHeavenId(rIDDMA.GetHeavenId().get())
        , m_nHellId(rIDDMA.GetHellId().get())
        , m_nControlsId(rIDDMA.GetControlsId().get())
        , m_nInvisibleHeavenId(rIDDMA.GetInvisibleHeavenId().get())
        , m_nInvisibleHellId(rIDDMA.GetInvisibleHellId().get())
        , m_nInvisibleControlsId(rIDDMA.GetInvisibleControlsId().get())
    {
    }

    auto operator()(css::uno::Reference<css::beans::XPropertySet> const& xShape) -> unsigned int
    {
        sal_Int16 nLayerID(0);
        if (xShape->getPropertySetInfo()->hasPropertyByName(u"LayerID"_ustr))
        {
            xShape->getPropertyValue(u"LayerID"_ustr) >>= nLayerID;
            if (nLayerID == m_nHellId || nLayerID == m_nInvisibleHellId)
            {
                return 0;
            }
            else if (nLayerID == m_nHeavenId || nLayerID == m_nInvisibleHeavenId)
            {
                return 1;
            }
            else if (nLayerID == m_nControlsId || nLayerID == m_nInvisibleControlsId)
            {
                return 2;
            }
            O3TL_UNREACHABLE;
        }
        else // SwXFrame only has "Opaque"
        {
            if (*o3tl::doAccess<bool>(xShape->getPropertyValue(u"Opaque"_ustr)))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }

private:
    sal_Int16 m_nHeavenId;
    sal_Int16 m_nHellId;
    sal_Int16 m_nControlsId;
    sal_Int16 m_nInvisibleHeavenId;
    sal_Int16 m_nInvisibleHellId;
    sal_Int16 m_nInvisibleControlsId;
};

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

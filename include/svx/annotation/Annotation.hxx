/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <svx/svdpage.hxx>
#include <svx/svxdllapi.h>

namespace sdr::annotation
{
class SVXCORE_DLLPUBLIC Annotation
{
private:
    static sal_uInt32 m_nLastId;
    static sal_uInt32 nextID() { return m_nLastId++; }

protected:
    SdrPage* mpSdrPage;
    sal_uInt32 m_nId;

    css::geometry::RealPoint2D m_Position;
    css::geometry::RealSize2D m_Size;
    OUString m_Author;
    OUString m_Initials;
    css::util::DateTime m_DateTime;
    bool m_bIsFreeText = false;

public:
    Annotation(SdrPage* pPage)
        : mpSdrPage(pPage)
        , m_nId(nextID())
    {
    }

    SdrModel* GetModel()
    {
        return mpSdrPage != nullptr ? &mpSdrPage->getSdrModelFromSdrPage() : nullptr;
    }

    sal_uInt32 GetId() const { return m_nId; }
};

//typedef std::vector<rtl::Reference<Annotation>> AnnotationVector;

} // namespace sdr::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

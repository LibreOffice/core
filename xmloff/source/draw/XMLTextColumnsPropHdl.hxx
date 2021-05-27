/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <com/sun/star/uno/Any.h>

#include <rtl/ustring.hxx>
#include <xmloff/xmlprhdl.hxx>

class SvXMLUnitConverter;

// PropertyHandler for style::columns
class XMLTextColumnsPropHdl : public XMLPropertyHandler
{
public:
    explicit XMLTextColumnsPropHdl();

    virtual bool equals(const css::uno::Any& r1, const css::uno::Any& r2) const override;

    virtual bool importXML(const OUString& rStrImpValue, css::uno::Any& rValue,
                           const SvXMLUnitConverter& rUnitConverter) const override;
    virtual bool exportXML(OUString& rStrExpValue, const css::uno::Any& rValue,
                           const SvXMLUnitConverter& rUnitConverter) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

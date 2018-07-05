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

#ifndef INCLUDED_EDITENG_MEASFLD_HXX
#define INCLUDED_EDITENG_MEASFLD_HXX

#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editengdllapi.h>
#include <com/sun/star/text/textfield/Type.hpp>

class SdrMeasureObj;

enum class SdrMeasureFieldKind { Value, Unit, Rotate90Blanks };

class EDITENG_DLLPUBLIC SdrMeasureField: public SvxFieldData {
    SdrMeasureFieldKind eMeasureFieldKind;
public:
    virtual sal_Int32  GetClassId() const override { return css::text::textfield::Type::MEASURE; }
    SdrMeasureField(SdrMeasureFieldKind eNewKind): eMeasureFieldKind(eNewKind) {}
    virtual ~SdrMeasureField() override;
    virtual std::unique_ptr<SvxFieldData> Clone() const override;
    virtual bool operator==(const SvxFieldData&) const override;
    SdrMeasureFieldKind GetMeasureFieldKind() const { return eMeasureFieldKind; }

    SdrMeasureField(SdrMeasureField const &) = default;
    SdrMeasureField(SdrMeasureField &&) = default;
    SdrMeasureField & operator =(SdrMeasureField const &) = default;
    SdrMeasureField & operator =(SdrMeasureField &&) = default;
};


#endif // INCLUDED_EDITENG_MEASFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

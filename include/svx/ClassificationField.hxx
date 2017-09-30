/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_CLASSIFICATIONFIELD_HXX
#define INCLUDED_SVX_CLASSIFICATIONFIELD_HXX

#include <sal/config.h>
#include <svx/svxdllapi.h>
#include <editeng/flditem.hxx>

namespace svx {

enum class ClassificationType
{
    CATEGORY,
    MARKING,
    TEXT,
    INTELLECTUAL_PROPERTY_PART
};

class ClassificationField : public SvxFieldData
{
public:
    ClassificationType meType;
    OUString msDescription;

    ClassificationField(ClassificationType eType, OUString const & sDescription)
        : SvxFieldData()
        , meType(eType)
        , msDescription(sDescription)
    {}

    ClassificationField* Clone() const override
    {
        return new ClassificationField(meType, msDescription);
    }

    bool operator==(const SvxFieldData& rOther) const override
    {
        if (typeid(rOther) != typeid(*this))
            return false;

        const ClassificationField& rOtherField = static_cast<const ClassificationField&>(rOther);
        return (meType == rOtherField.meType &&
                msDescription == rOtherField.msDescription);
    }
};

struct ClassificationResult
{
    ClassificationType meType;
    OUString msString;
    sal_Int32 mnParagraph;
};

} // end svx namespace

#endif // INCLUDED_SVX_CLASSIFICATIONFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

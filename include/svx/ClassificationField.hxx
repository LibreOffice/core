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
    INTELLECTUAL_PROPERTY_PART,
    PARAGRAPH,
};

class SVX_DLLPUBLIC ClassificationField : public SvxFieldData
{
public:
    ClassificationType meType;
    OUString msDescription;
    OUString msFullClassName;
    OUString msIdentifier;

    ClassificationField(ClassificationType eType, OUString const & sDescription, OUString const & sFullClassName, OUString const & sIdentifier = "")
        : SvxFieldData()
        , meType(eType)
        , msDescription(sDescription)
        , msFullClassName(sFullClassName)
        , msIdentifier(sIdentifier)
    {}

    ClassificationField* Clone() const override
    {
        return new ClassificationField(meType, msDescription, msFullClassName, msIdentifier);
    }

    bool operator==(const SvxFieldData& rOther) const override
    {
        if (typeid(rOther) != typeid(*this))
            return false;

        const ClassificationField& rOtherField = static_cast<const ClassificationField&>(rOther);
        return (meType == rOtherField.meType &&
                msDescription == rOtherField.msDescription &&
                msFullClassName == rOtherField.msFullClassName &&
                msIdentifier == rOtherField.msIdentifier);
    }
};

struct SVX_DLLPUBLIC ClassificationResult
{
    ClassificationType meType;
    OUString msString;  //< Display text or 'Name' field (from example.xml).
    OUString msAbbreviatedString; //< Abbreviated name, displayed instead of msString.
    OUString msIdentifier; //< The identifier of this entry (from example.xml).
};

} // end svx namespace

#endif // INCLUDED_SVX_CLASSIFICATIONFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

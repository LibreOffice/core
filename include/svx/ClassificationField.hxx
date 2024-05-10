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
#include <utility>

namespace svx {

enum class ClassificationType
{
    CATEGORY,
    MARKING,
    TEXT,
    INTELLECTUAL_PROPERTY_PART,
    PARAGRAPH,
};

class SVX_DLLPUBLIC ClassificationResult
{
public:
    ClassificationType meType;
    OUString msName;            //< Display text or 'Name' field (from example.xml).
    OUString msAbbreviatedName; //< Abbreviated name, displayed instead of Name.
    OUString msIdentifier;      //< The identifier of this entry (from example.xml).

    ClassificationResult(ClassificationType eType, OUString sName,
                         OUString sAbbreviatedName, OUString sIdentifier = u""_ustr)
        : meType(eType)
        , msName(std::move(sName))
        , msAbbreviatedName(std::move(sAbbreviatedName))
        , msIdentifier(std::move(sIdentifier))
    {
    }

    /// Returns the text to display, which is the Abbreviated Name, if provided, otherwise Name.
    OUString const & getDisplayText() const
    {
        return !msAbbreviatedName.isEmpty() ? msAbbreviatedName : msName;
    }

    bool operator==(const ClassificationResult& rOther) const
    {
        return (meType == rOther.meType &&
                msName == rOther.msName &&
                msAbbreviatedName == rOther.msAbbreviatedName &&
                msIdentifier == rOther.msIdentifier);
    }
};

class SVX_DLLPUBLIC ClassificationField final : public SvxFieldData
{
public:
    ClassificationType meType;
    OUString msDescription;
    OUString msFullClassName;
    OUString msIdentifier;

    ClassificationField(ClassificationType eType, OUString sDescription, OUString sFullClassName, OUString sIdentifier)
        : meType(eType)
        , msDescription(std::move(sDescription))
        , msFullClassName(std::move(sFullClassName))
        , msIdentifier(std::move(sIdentifier))
    {}

    std::unique_ptr<SvxFieldData> Clone() const override
    {
        return std::make_unique<ClassificationField>(meType, msDescription, msFullClassName, msIdentifier);
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

} // end svx namespace

#endif // INCLUDED_SVX_CLASSIFICATIONFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

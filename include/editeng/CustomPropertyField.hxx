/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_EDITENG_CUSTOMPROPERTYFIELD_HXX
#define INCLUDED_EDITENG_CUSTOMPROPERTYFIELD_HXX

#include <editeng/editengdllapi.h>

#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>

#include <com/sun/star/text/textfield/Type.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>


namespace editeng
{

class EDITENG_DLLPUBLIC CustomPropertyField : public SvxFieldData
{
private:
    OUString msName;
    OUString msCurrentPresentation;

public:
    explicit CustomPropertyField(OUString const & rName, OUString const & rCurrentPresentation);

    virtual ~CustomPropertyField() override;

    SV_DECL_PERSIST1(CustomPropertyField, css::text::textfield::Type::DOCINFO_CUSTOM)

    virtual SvxFieldData* Clone() const override;
    virtual bool operator==(const SvxFieldData&) const override;

    virtual MetaAction* createBeginComment() const override;

    OUString GetFormatted(css::uno::Reference<css::document::XDocumentProperties> const & xDocumentProperties);

    OUString const & GetName() const
    {
        return msName;
    }

    OUString const & GetCurrentPresentation() const
    {
        return msCurrentPresentation;
    }
};

} // end editeng namespace

#endif // INCLUDED_EDITENG_CUSTOMPROPERTYFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

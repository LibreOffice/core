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

#ifndef INCLUDED_COMPHELPER_ATTRIBUTELIST_HXX
#define INCLUDED_COMPHELPER_ATTRIBUTELIST_HXX

#include <sal/config.h>

#include <vector>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

class COMPHELPER_DLLPUBLIC AttributeList final :
    public ::cppu::WeakImplHelper<css::xml::sax::XAttributeList, css::util::XCloneable>
{
    struct TagAttribute
    {
        OUString sName;
        OUString sValue;
    };
    std::vector<TagAttribute> mAttributes;
public:
    AttributeList();
    AttributeList(const AttributeList &r) = default;
    AttributeList(const css::uno::Reference<css::xml::sax::XAttributeList>& rAttrList);
    AttributeList(AttributeList&&) = delete;

    virtual ~AttributeList() override;

    // methods that are not contained in any interface
    void AddAttribute(const OUString &sName, const OUString &sValue);
    void Clear()
    {
        mAttributes.clear();
    }
    void RemoveAttribute(const OUString& sName);
    void AppendAttributeList(const css::uno::Reference< css::xml::sax::XAttributeList >&);
    void SetValueByIndex(sal_Int16 i, const OUString& rValue);
    void RemoveAttributeByIndex(sal_Int16 i);
    void RenameAttributeByIndex(sal_Int16 i, const OUString& rNewName);
    sal_Int16 GetIndexByName(const OUString& rName) const;

    // css::xml::sax::XAttributeList
    virtual sal_Int16 SAL_CALL getLength() override
    {
        return static_cast<sal_Int16>(mAttributes.size());
    }
    virtual OUString SAL_CALL getNameByIndex(sal_Int16 i) override
    {
        return mAttributes[i].sName;
    }
    virtual OUString SAL_CALL getTypeByIndex(sal_Int16) override { return u"CDATA"_ustr; }
    virtual OUString SAL_CALL getTypeByName(const OUString&) override { return u"CDATA"_ustr; }
    virtual OUString SAL_CALL getValueByIndex(sal_Int16 i) override
    {
        return mAttributes[i].sValue;
    }
    virtual OUString SAL_CALL getValueByName(const OUString& aName) override;

    // css::util::XCloneable
    virtual css::uno::Reference< XCloneable > SAL_CALL
        createClone() override;
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_ATTRIBUTELIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

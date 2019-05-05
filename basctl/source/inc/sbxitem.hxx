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
#ifndef INCLUDED_BASCTL_SOURCE_INC_SBXITEM_HXX
#define INCLUDED_BASCTL_SOURCE_INC_SBXITEM_HXX

#include "scriptdocument.hxx"
#include <svl/poolitem.hxx>

// I2TM
#include <item/base/ItemBuffered.hxx>
#include <item/base/ItemControlBlock.hxx>
#include <item/base/ItemAdministrator.hxx>
// ~I2TM

namespace basctl
{

enum ItemType
{
    TYPE_UNKNOWN,
    TYPE_SHELL,
    TYPE_LIBRARY,
    TYPE_MODULE,
    TYPE_DIALOG,
    TYPE_METHOD
};

// I2TM
namespace Item
{
    class Sbx : public ::Item::ItemBuffered
    {
    public:
        static ::Item::ItemControlBlock& GetStaticItemControlBlock();

    protected:
        class SbxData : public ::Item::ItemBuffered::ItemData
        {
        private:
            const ScriptDocument    m_aDocument;
            const OUString          m_aLibName;
            const OUString          m_aName;
            const OUString          m_aMethodName;
            ItemType                m_eType;

        protected:
            virtual ::Item::ItemAdministrator& getItemAdministrator() const;

        public:
            SbxData(
                const ScriptDocument& rDocument,
                const OUString& aLibName = OUString(),
                const OUString& aName = OUString(),
                const OUString& aMethodName = OUString(),
                ItemType eType = TYPE_UNKNOWN);

            virtual bool operator==(const ItemData& rRef) const;

            ScriptDocument const& GetDocument() const { return m_aDocument; }
            OUString const& GetLibName() const { return m_aLibName; }
            OUString const& GetName() const { return m_aName; }
            OUString const& GetMethodName() const { return m_aMethodName; }
            ItemType GetType() const { return m_eType; }
        };

    public:
        Sbx(
            const ScriptDocument& rDocument,
            const OUString& aLibName = OUString(),
            const OUString& aName = OUString(),
            const OUString& aMethodName = OUString(),
            ItemType eType = TYPE_UNKNOWN);

        ScriptDocument const& GetDocument() const { return static_cast<SbxData const&>(getItemData()).GetDocument(); }
        OUString const& GetLibName() const { return static_cast<SbxData const&>(getItemData()).GetLibName(); }
        OUString const& GetName() const { return static_cast<SbxData const&>(getItemData()).GetName(); }
        OUString const& GetMethodName() const { return static_cast<SbxData const&>(getItemData()).GetMethodName(); }
        ItemType GetType() const { return static_cast<SbxData const&>(getItemData()).GetType(); }
    };
}
// ~I2TM
} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_SBXITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

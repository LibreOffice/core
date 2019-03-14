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

#ifndef INCLUDED_ITEM_BASE_IBASE_HXX
#define INCLUDED_ITEM_BASE_IBASE_HXX

#include <memory>
#include <sal/types.h>
#include <item/itemdllapi.h>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // predefine IAdministrator - no need to include
    class IAdministrator;

    class ITEM_DLLPUBLIC IBase : public std::enable_shared_from_this<IBase>
    {
    public:
        // SharedPtr typedef to be used handling instances of this type
        typedef std::shared_ptr<const IBase> SharedPtr;

    private:
        // flag to mark this instance being administared by an
        // IAdministrator. Not urgently needed due to also being
        // able to check being administarted in the HintExpired
        // calls. But that is the point - when using this flag
        // at adding the instance and thus making it being actively
        // administrated it is not necessary to do that check
        // if it is administrated which means a 'find' access
        // to a kind of list which may have varying costs...
        friend class IAdministrator;
        bool        m_bAdministrated;

    protected:
        // constructor - protected BY DEFAULT - do NOT CHANGE (!)
        // Use ::Create(...) methods in derived classes instead
        IBase();

        // basic RTTI TypeCheck to secure e.g. operator== and similar
        bool CheckSameType(const IBase& rCmp) const;

        // basic access to Adminiatrator, default returns nullptr and is *not*
        // designed to be used/called, only exists to have simple Item
        // representations for special purposes, e.g. InvalidateItem/DisableItem
        virtual IAdministrator* GetIAdministrator() const;

    public:
        virtual ~IBase();

        // noncopyable
        IBase(const IBase&) = delete;
        IBase& operator=(const IBase&) = delete;

        // operators potentially used by IAdministrator implementations, so have to be defined
        virtual bool operator==(const IBase& rCmp) const;
        virtual bool operator<(const IBase& rCmp) const;
        virtual size_t GetHash() const;

        // Interface for global default value support. These non-static and
        // non-virtual non-typed local versions can/may work more direct. The
        // typed static versions are not capable of working with 'const IBase::SharedPtr&'
        // due to using std::static_pointer_cast, thus these may be faster and
        // use less ressources when the type is not needed.
        // These will use the callback to static administrator and it's administrated
        // single global default instance.
        // Remember that there *will* also be static, typed versions of this call
        // in derived Item(s), see IBaseStaticHelper for reference
        bool IsDefault() const;
        const SharedPtr& GetDefault() const;

        // check Administrated flag
        bool IsAdministrated() const
        {
            return m_bAdministrated;
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_IBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

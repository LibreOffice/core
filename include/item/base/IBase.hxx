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

    // Base class for IBase and thus for all new implementation of
    // Items. Items are in general read-only instances. The constructor
    // is protected now so that no one is able to create own instances.
    // The reason for that is that for all Items only a single value-
    // specific instance shall exist at any time, guaranteed by being
    // administrated by an instance of IAdministrator associated with
    // each implemented type of IBase Item.
    // To construct, use the ::Create methods. These are in general
    // static member functions. These have to use the protected
    // constructor(s) from which also any number may be defined to
    // support the ::Create methods, but all shall be protected.
    // It is also possible to use a single constructor and set values
    // as needed at the created instance in any local implementation
    // of a ::Create method.
    // That instance created and filled (value-set) in a ::Create method
    // will then be handed over to the single static global IAdministrator
    // instance of the derived Item-type. That method will decide if
    // to use the given instance or delete it again - if already an instance
    // of that Item exists (or is identical to the global default of
    // this Item-type).
    // The static ::Create methods only return shared_ptr's of instances,
    // thus any usage of Items will be based on using shared_ptr's and
    // thus making runtime existance of Items safe. It also uses typed
    // derivations and templates to ensure throghout usage of typed
    // Items.
    // That Items are in general read-only instances also means that there
    // will no longer be set-methods like ::SetAny(Any& aAny) or similar.
    // Instead, implement a ::Create method for this that creates an instance
    // using a/the protected constructor, set all members from the Any (in
    // this exapmle) and hand over to the IAdministrator. Result will in all
    // cases be a shared_ptr of the Item.
    // Items are no longer identified by any global to-be-somewhere-defined
    // (collision-free, global, incompatible, hard to maintain) ItemID, but
    // by their Type only internally using C++'s typeid() and
    // typeid(<ItemType>).hash_code().
    // The efficency the administration can be done highly depends on the
    // type of operators defined at the Item-Implementation, see
    // comments at IAdministrator and it's three default implementations for
    // details - also discussing speed compared to SfxItem implementations.
    // When implementing an Item, you choose one of these three
    // predefined implementations to use for the Item-type, based on the
    // operators available at your Item-implementation.
    // For an example, see 'CntMultiValue' in the test cases, or any other
    // Item-implementation (e.g. Item::CntInt16 or Item::CntOUString).
    // Note: Be aware that some static local functions will be needed for
    // any Item-implementation. To support this and full typed accesses
    // to implemented Items, check out the implementation helper template
    // class 'IBaseStaticHelper' which should be used to implement Items.
    // This Base-Class IBase exists as minimal achor point, it sometimes
    // is useful, see e.g. helper classes ImplInvalidateItem and
    // ImplDisableItem used in ::Iset to represent ItemStates in ISet's.
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
        virtual size_t GetUniqueKey() const;

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

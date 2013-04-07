/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <vector>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "unoidl/legacyprovider.hxx"
#include "unoidl/unoidl.hxx"
#include "unoidl/unoidlprovider.hxx"

namespace unoidl {

NoSuchFileException::~NoSuchFileException() throw () {}

FileFormatException::~FileFormatException() throw () {}

Entity::~Entity() throw () {}

MapCursor::~MapCursor() throw () {}

ModuleEntity::~ModuleEntity() throw () {}

PublishableEntity::~PublishableEntity() throw () {}

EnumTypeEntity::~EnumTypeEntity() throw () {}

PlainStructTypeEntity::~PlainStructTypeEntity() throw () {}

PolymorphicStructTypeTemplateEntity::~PolymorphicStructTypeTemplateEntity()
    throw ()
{}

ExceptionTypeEntity::~ExceptionTypeEntity() throw () {}

InterfaceTypeEntity::~InterfaceTypeEntity() throw () {}

TypedefEntity::~TypedefEntity() throw () {}

ConstantGroupEntity::~ConstantGroupEntity() throw () {}

SingleInterfaceBasedServiceEntity::~SingleInterfaceBasedServiceEntity() throw ()
{}

AccumulationBasedServiceEntity::~AccumulationBasedServiceEntity() throw () {}

InterfaceBasedSingletonEntity::~InterfaceBasedSingletonEntity() throw () {}

ServiceBasedSingletonEntity::~ServiceBasedSingletonEntity() throw () {}

Provider::~Provider() throw () {}

rtl::Reference< Provider > loadProvider(
    rtl::Reference< Manager > const & manager, OUString const & uri)
{
    try {
        return new UnoidlProvider(uri);
    } catch (FileFormatException & e) {
        SAL_INFO(
            "unoidl",
            "FileFormatException \"" << e.getDetail() << "\", retrying <" << uri
                << "> as legacy format");
        return new LegacyProvider(manager, uri);
    }
}

void Manager::addProvider(rtl::Reference< Provider > const & provider) {
    assert(provider.is());
    providers_.push_back(provider);
}

rtl::Reference< Entity > Manager::findEntity(OUString const & name) const {
    //TODO: add caching
    for (std::vector< rtl::Reference< Provider > >::const_iterator i(
             providers_.begin());
         i != providers_.end(); ++i)
    {
        rtl::Reference< Entity > ent((*i)->findEntity(name));
        if (ent.is()) {
            return ent;
        }
    }
    return rtl::Reference< Entity >();
}

Manager::~Manager() throw () {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

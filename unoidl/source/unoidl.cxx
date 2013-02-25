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

rtl::Reference< Provider > loadProvider(OUString const & uri) {
    return new UnoidlProvider(uri);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

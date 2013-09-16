/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <set>
#include <vector>

#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "unoidl/unoidl.hxx"

#include "legacyprovider.hxx"
#include "sourcefileprovider.hxx"
#include "sourcetreeprovider.hxx"
#include "unoidlprovider.hxx"

namespace unoidl {

namespace {

class AggregatingCursor: public MapCursor {
public:
    AggregatingCursor(
        std::vector< rtl::Reference< MapCursor > > const & cursors):
        cursors_(cursors), iterator_(cursors_.begin())
    {}

private:
    virtual ~AggregatingCursor() throw () {}

    virtual rtl::Reference< Entity > getNext(rtl::OUString * name);

    std::vector< rtl::Reference< MapCursor > > cursors_;
    std::vector< rtl::Reference< MapCursor > >::iterator iterator_;
    std::set< rtl::OUString > seenMembers;
};

rtl::Reference< Entity > AggregatingCursor::getNext(rtl::OUString * name) {
    for (;;) {
        if (iterator_ == cursors_.end()) {
            return rtl::Reference< Entity >();
        }
        rtl::OUString n;
        rtl::Reference< Entity > ent((*iterator_)->getNext(&n));
        if (ent.is()) {
            if (seenMembers.insert(n).second) {
                if (name != 0) {
                    *name = n;
                }
                return ent;
            }
        } else {
            ++iterator_;
        }
    }
}

}

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
    osl::DirectoryItem item;
    if (osl::DirectoryItem::get(uri, item) == osl::FileBase::E_None) {
        osl::FileStatus status(osl_FileStatus_Mask_Type);
        if (item.getFileStatus(status) == osl::FileBase::E_None
            && status.getFileType() == osl::FileStatus::Directory)
        {
            return new detail::SourceTreeProvider(manager, uri);
        }
    }
    if (uri.endsWith(".idl")) {
        return new detail::SourceFileProvider(manager, uri);
    }
    try {
        return new detail::UnoidlProvider(uri);
    } catch (FileFormatException & e) {
        SAL_INFO(
            "unoidl",
            "FileFormatException \"" << e.getDetail() << "\", retrying <" << uri
                << "> as legacy format");
        return new detail::LegacyProvider(manager, uri);
    }
}

void Manager::addProvider(rtl::Reference< Provider > const & provider) {
    assert(provider.is());
    osl::MutexGuard g(mutex_);
    providers_.push_back(provider);
}

rtl::Reference< Entity > Manager::findEntity(rtl::OUString const & name) const {
    //TODO: caching? (here or in cppuhelper::TypeManager?)
    osl::MutexGuard g(mutex_);
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

rtl::Reference< MapCursor > Manager::createCursor(rtl::OUString const & name)
    const
{
    std::vector< rtl::Reference< MapCursor > > curs;
    for (std::vector< rtl::Reference< Provider > >::const_iterator i(
             providers_.begin());
         i != providers_.end(); ++i)
    {
        if (name.isEmpty()) {
            curs.push_back((*i)->createRootCursor());
        } else {
            rtl::Reference< Entity > ent((*i)->findEntity(name));
            if (ent.is() && ent->getSort() == Entity::SORT_MODULE) {
                curs.push_back(
                    static_cast< ModuleEntity * >(ent.get())->createCursor());
            }
        }
    }
    return curs.empty()
        ? rtl::Reference< MapCursor >() : new AggregatingCursor(curs);
}

Manager::~Manager() throw () {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

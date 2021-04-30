/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <set>
#include <vector>

#include <osl/file.h>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <unoidl/unoidl.hxx>

#include "legacyprovider.hxx"
#include "sourcefileprovider.hxx"
#include "sourcetreeprovider.hxx"
#include "unoidlprovider.hxx"

namespace unoidl {

namespace {

class AggregatingModule: public ModuleEntity {
public:
    AggregatingModule(
        std::vector< rtl::Reference< Provider > > const & providers,
        OUString const & name):
        providers_(providers), name_(name)
    {}

private:
    virtual ~AggregatingModule() noexcept override {}

    virtual std::vector< OUString > getMemberNames() const override;

    virtual rtl::Reference< MapCursor > createCursor() const override;

    std::vector< rtl::Reference< Provider > > providers_;
    OUString name_;
};

std::vector< OUString > AggregatingModule::getMemberNames() const {
    std::set< OUString > names;
    for (auto & i: providers_) {
        rtl::Reference< Entity > ent(i->findEntity(name_));
        if (ent.is() && ent->getSort() == Entity::SORT_MODULE) {
            std::vector< OUString > ns(
                static_cast< ModuleEntity * >(ent.get())->getMemberNames());
            names.insert(ns.begin(), ns.end());
        }
    }
    return std::vector< OUString >(names.begin(), names.end());
}

class AggregatingCursor: public MapCursor {
public:
    AggregatingCursor(
        std::vector< rtl::Reference< Provider > > const & providers,
        OUString const & name):
        providers_(providers), name_(name), iterator_(providers_.begin())
    { findCursor(); }

private:
    virtual ~AggregatingCursor() noexcept override {}

    virtual rtl::Reference< Entity > getNext(OUString * name) override;

    void findCursor();

    std::vector< rtl::Reference< Provider > > providers_;
    OUString name_;
    std::vector< rtl::Reference< Provider > >::iterator iterator_;
    rtl::Reference< MapCursor > cursor_;
    std::set< OUString > seen_;
};

rtl::Reference< Entity > AggregatingCursor::getNext(OUString * name) {
    while (cursor_.is()) {
        OUString n;
        rtl::Reference< Entity > ent(cursor_->getNext(&n));
        if (ent.is()) {
            if (seen_.insert(n).second) {
                if (name != nullptr) {
                    *name = n;
                }
                return ent->getSort() == Entity::SORT_MODULE
                    ? new AggregatingModule(
                        providers_, (name_.isEmpty() ? name_ : name_ + ".") + n)
                    : ent;
            }
        } else {
            cursor_.clear();
            findCursor();
        }
    }
    return rtl::Reference< Entity >();
}

void AggregatingCursor::findCursor() {
    for (; !cursor_.is() && iterator_ != providers_.end(); ++iterator_) {
        if (name_.isEmpty()) {
            cursor_ = (*iterator_)->createRootCursor();
        } else {
            rtl::Reference< Entity > ent((*iterator_)->findEntity(name_));
            if (ent.is() && ent->getSort() == Entity::SORT_MODULE) {
                cursor_ = static_cast< ModuleEntity * >(ent.get())->
                    createCursor();
            }
        }
    }
}

rtl::Reference< MapCursor > AggregatingModule::createCursor() const {
    return new AggregatingCursor(providers_, name_);
}

}

NoSuchFileException::~NoSuchFileException() noexcept {}

FileFormatException::~FileFormatException() noexcept {}

Entity::~Entity() noexcept {}

MapCursor::~MapCursor() noexcept {}

ModuleEntity::~ModuleEntity() noexcept {}

PublishableEntity::~PublishableEntity() noexcept {}

EnumTypeEntity::~EnumTypeEntity() noexcept {}

PlainStructTypeEntity::~PlainStructTypeEntity() noexcept {}

PolymorphicStructTypeTemplateEntity::~PolymorphicStructTypeTemplateEntity()
    noexcept
{}

ExceptionTypeEntity::~ExceptionTypeEntity() noexcept {}

InterfaceTypeEntity::~InterfaceTypeEntity() noexcept {}

TypedefEntity::~TypedefEntity() noexcept {}

ConstantGroupEntity::~ConstantGroupEntity() noexcept {}

SingleInterfaceBasedServiceEntity::~SingleInterfaceBasedServiceEntity() noexcept
{}

AccumulationBasedServiceEntity::~AccumulationBasedServiceEntity() noexcept {}

InterfaceBasedSingletonEntity::~InterfaceBasedSingletonEntity() noexcept {}

ServiceBasedSingletonEntity::~ServiceBasedSingletonEntity() noexcept {}

Provider::~Provider() noexcept {}

rtl::Reference< Provider > Manager::addProvider(OUString const & uri) {
    rtl::Reference< Provider > p(loadProvider(uri));
    assert(p.is());
    {
        osl::MutexGuard g(mutex_);
        providers_.push_back(p);
    }
    return p;
}

rtl::Reference< Entity > Manager::findEntity(OUString const & name) const {
    //TODO: caching? (here or in cppuhelper::TypeManager?)
    osl::MutexGuard g(mutex_);
    for (auto & i: providers_) {
        rtl::Reference< Entity > ent(i->findEntity(name));
        if (ent.is()) {
            return ent;
        }
    }
    return rtl::Reference< Entity >();
}

rtl::Reference< MapCursor > Manager::createCursor(OUString const & name)
    const
{
    return new AggregatingCursor(providers_, name);
}

Manager::~Manager() noexcept {}

rtl::Reference< Provider > Manager::loadProvider(OUString const & uri) {
    osl::DirectoryItem item;
    if (osl::DirectoryItem::get(uri, item) == osl::FileBase::E_None) {
        osl::FileStatus status(osl_FileStatus_Mask_Type);
        if (item.getFileStatus(status) == osl::FileBase::E_None
            && status.getFileType() == osl::FileStatus::Directory)
        {
            return new detail::SourceTreeProvider(*this, uri);
        }
    }
    if (uri.endsWith(".idl")) {
        return new detail::SourceFileProvider(this, uri);
    }
    try {
        return new detail::UnoidlProvider(uri);
    } catch (FileFormatException & e) {
        SAL_INFO(
            "unoidl",
            "FileFormatException \"" << e.getDetail() << "\", retrying <" << uri
                << "> as legacy format");
        return new detail::LegacyProvider(*this, uri);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <map>
#include <utility>
#include <vector>

#include "sourcefileprovider.hxx"
#include "sourceprovider-scanner.hxx"

namespace unoidl { namespace detail {

namespace {

class Cursor: public MapCursor {
public:
    explicit Cursor(std::map< OUString, rtl::Reference<Entity> > const & map):
        map_(map), iterator_(map_.begin())
    {}

private:
    virtual ~Cursor() throw () {}

    virtual rtl::Reference< Entity > getNext(OUString * name);

    std::map< OUString, rtl::Reference<Entity> > const & map_; //TODO: extent
    std::map< OUString, rtl::Reference<Entity> >::const_iterator iterator_;
};

rtl::Reference< Entity > Cursor::getNext(OUString * name) {
    assert(name != 0);
    rtl::Reference< Entity > ent;
    if (iterator_ != map_.end()) {
        *name = iterator_->first;
        ent = iterator_->second;
        ++iterator_;
    }
    return ent;
}

class Module: public ModuleEntity {
public:
    Module() {}

    std::map< OUString, rtl::Reference<Entity> > map;

private:
    virtual ~Module() throw () {}

    virtual std::vector<rtl::OUString> getMemberNames() const;

    virtual rtl::Reference<MapCursor> createCursor() const
    { return new Cursor(map); }
};

std::vector<rtl::OUString> Module::getMemberNames() const {
    std::vector<rtl::OUString> names;
    for (std::map< OUString, rtl::Reference<Entity> >::const_iterator i(
             map.begin());
         i != map.end(); ++i)
    {
        names.push_back(i->first);
    }
    return names;
}

}

SourceFileProvider::SourceFileProvider(
    rtl::Reference<Manager> const & manager, OUString const & uri)
{
    SourceProviderScannerData data(manager);
    if (!parse(uri, &data)) {
        throw NoSuchFileException(uri);
    }
    for (std::map<OUString, SourceProviderEntity>::iterator i(
             data.entities.begin());
         i != data.entities.end(); ++i)
    {
        if (i->second.kind == SourceProviderEntity::KIND_LOCAL) {
            assert(i->second.entity.is());
            assert(i->second.entity->getSort() != Entity::SORT_MODULE);
            std::map< OUString, rtl::Reference<Entity> > * map = &rootMap_;
            for (sal_Int32 j = 0;;) {
                OUString id(i->first.getToken(0, '.', j));
                if (j == -1) {
                    map->insert(std::make_pair(id, i->second.entity));
                    break;
                }
                std::map< OUString, rtl::Reference<Entity> >::const_iterator k(
                    map->find(id));
                if (k == map->end()) {
                    k = map->insert(std::make_pair(id, new Module)).first;
                }
                Module * mod = dynamic_cast< Module * >(k->second.get());
                assert(mod != 0);
                map = &mod->map;
            }
        }
    }
}

rtl::Reference<MapCursor> SourceFileProvider::createRootCursor() const {
    return new Cursor(rootMap_);
}

rtl::Reference<Entity> SourceFileProvider::findEntity(OUString const & name)
    const
{
    std::map< OUString, rtl::Reference<Entity> > const * map = &rootMap_;
    for (sal_Int32 i = 0;;) {
        OUString id(name.getToken(0, '.', i));
        std::map< OUString, rtl::Reference<Entity> >::const_iterator j(
            map->find(id));
        if (j == map->end()) {
            return rtl::Reference<Entity>();
        }
        if (i == -1) {
            return j->second;
        }
        if (j->second->getSort() != Entity::SORT_MODULE) {
            return rtl::Reference<Entity>();
        }
        Module * mod = dynamic_cast< Module * >(j->second.get());
        assert(mod != 0);
        map = &mod->map;
    }
}

SourceFileProvider::~SourceFileProvider() throw () {}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

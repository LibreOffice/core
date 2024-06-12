/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <unordered_set>
#include <string_view>

#include <codemaker/typemanager.hxx>
#include <rtl/string.hxx>
#include <unoidl/unoidl.hxx>

#include "netoptions.hxx"

class NetProducer
{
public:
    NetProducer()
        : m_manager(new TypeManager())
    {
    }

public:
    void initProducer(const NetOptions& options);
    void produceAll();

private:
    void produceType(const OString& name);
    void produceModule(std::string_view name, const rtl::Reference<unoidl::MapCursor>& cursor);
    void produceEnum(std::string_view name, const rtl::Reference<unoidl::EnumTypeEntity>& entity);
    void producePlainStruct(std::string_view name,
                            const rtl::Reference<unoidl::PlainStructTypeEntity>& entity);
    void
    producePolyStruct(std::string_view name,
                      const rtl::Reference<unoidl::PolymorphicStructTypeTemplateEntity>& entity);
    void produceException(std::string_view name,
                          const rtl::Reference<unoidl::ExceptionTypeEntity>& entity);
    void produceInterface(std::string_view name,
                          const rtl::Reference<unoidl::InterfaceTypeEntity>& entity);
    void produceTypedef(std::string_view name, const rtl::Reference<unoidl::TypedefEntity>& entity);
    void produceConstantGroup(std::string_view name,
                              const rtl::Reference<unoidl::ConstantGroupEntity>& entity);
    void produceService(std::string_view name,
                        const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity);
    void produceSingleton(std::string_view name,
                          const rtl::Reference<unoidl::InterfaceBasedSingletonEntity>& entity);

    OString getNetName(std::string_view name);
    OString getNetName(std::u16string_view name);

private:
    rtl::Reference<TypeManager> m_manager;

    std::unordered_set<OString> m_startingTypes;
    std::unordered_set<OString> m_typesProduced;
    std::unordered_map<OString, OString> m_typedefs;

    OString m_outputDir;
    bool m_verbose;
    bool m_dryRun;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

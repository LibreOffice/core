/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <headless/svpinst.hxx>
#include <headless/svpdummies.hxx>
#include <headless/svpdata.hxx>
#include <unistd.h>

class HeadlessSalInstance : public SvpSalInstance
{
public:
    explicit HeadlessSalInstance(std::unique_ptr<SalYieldMutex> pMutex);

    virtual SalSystem* CreateSalSystem() override;
};

HeadlessSalInstance::HeadlessSalInstance(std::unique_ptr<SalYieldMutex> pMutex)
    : SvpSalInstance(std::move(pMutex))
{
}

class HeadlessSalSystem : public SvpSalSystem {
public:
    HeadlessSalSystem() : SvpSalSystem() {}
    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::vector< OUString >& rButtons ) override
    {
        (void)rButtons;
        SAL_INFO("vcl.headless",
                "LibreOffice - dialog '"
                << rTitle << "': '"
                << rMessage << "'");
        return 0;
    }
};

SalSystem *HeadlessSalInstance::CreateSalSystem()
{
    return new HeadlessSalSystem();
}

extern "C" SalInstance *create_SalInstance()
{
    HeadlessSalInstance* pInstance = new HeadlessSalInstance(std::make_unique<SvpSalYieldMutex>());
    new SvpSalData();
    return pInstance;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdlib.h>
#include <algorithm>
#include <vector>

#include <comphelper/processfactory.hxx>

#include "DiscoveryService.hxx"

using namespace sd;

DiscoveryService::DiscoveryService()
    :
    Thread( "sd::DiscoveryService" ),
    mSocket()
{
}

DiscoveryService::~DiscoveryService()
{
}



void DiscoveryService::execute()
{

}

DiscoveryService *sd::DiscoveryService::spService = NULL;

void DiscoveryService::setup()
{
  if (spService)
    return;

  spService = new DiscoveryService();
  spService->launch();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
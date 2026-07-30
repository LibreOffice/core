/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * The /livez and /readyz health check endpoints.
 * See healthchecks.txt for the format and the individual checks.
 */

#pragma once

#include <memory>

class StreamSocket;

namespace Poco
{
namespace Net
{
class HTTPRequest;
}
}

namespace HealthCheck
{

/// Serve a /livez or /readyz request, including per-check sub-paths such as
/// /livez/forkit. Sends the complete response synchronously and returns true.
bool handleRequest(const Poco::Net::HTTPRequest& request,
                   const std::shared_ptr<StreamSocket>& socket);

} // namespace HealthCheck

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

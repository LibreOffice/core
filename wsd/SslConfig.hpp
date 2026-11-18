/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * SSL/TLS configuration and initialization.
 * Classes: SslConfig
 */

#pragma once

#if ENABLE_SSL
#include <net/SslSocket.hpp>
#include <Poco/Net/SSLManager.h>
#endif

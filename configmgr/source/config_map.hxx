/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef CONFIG_MAP_HXX
#define CONFIG_MAP_HXX

#include <unordered_map>
#include <rtl/ustring.hxx>

template< class T > struct config_map : public std::unordered_map< OUString, T > { };

#endif // CONFIG_MAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

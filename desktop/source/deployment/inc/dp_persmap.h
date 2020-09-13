/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <unordered_map>

namespace dp_misc
{

typedef std::unordered_map<
    OString, OString > t_string2string_map;

// Class to read obsolete registered extensions
// should be removed for LibreOffice 4.0
class PersistentMap final
{
    ::osl::File m_MapFile;
    t_string2string_map m_entries;
    bool m_bIsOpen;
    bool m_bToBeCreated;
    bool m_bIsDirty;

public:
    ~PersistentMap();
    PersistentMap( OUString const & url );
    /** in mem db */
    PersistentMap();

    bool has( OString const & key ) const;
    bool get( OString * value, OString const & key ) const;
    const t_string2string_map& getEntries() const { return m_entries; }
    void put( OString const & key, OString const & value );
    bool erase( OString const & key );

private:
    void open();
    void readAll();
    void add( OString const & key, OString const & value );
    void flush();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

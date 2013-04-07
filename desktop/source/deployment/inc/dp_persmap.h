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

#ifndef INCLUDED_DP_PERSMAP_H
#define INCLUDED_DP_PERSMAP_H

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <boost/unordered_map.hpp>

namespace dp_misc
{

typedef ::boost::unordered_map<
    OString, OString, OStringHash > t_string2string_map;

// Class to read obsolete registered extensions
// should be removed for LibreOffice 4.0
class PersistentMap
{
    ::osl::File m_MapFile;
    t_string2string_map m_entries;
    bool m_bReadOnly;
    bool m_bIsOpen;
    bool m_bToBeCreated;
    bool m_bIsDirty;

public:
    ~PersistentMap();
    PersistentMap( OUString const & url, bool readOnly );
    /** in mem db */
    PersistentMap();

    bool has( OString const & key ) const;
    bool get( OString * value, OString const & key ) const;
    t_string2string_map getEntries() const;
    void put( OString const & key, OString const & value );
    bool erase( OString const & key, bool flush_immediately = true );

protected:
    bool open();
    bool readAll();
    void add( OString const & key, OString const & value );
    void flush();

#ifndef DISABLE_BDB2PMAP
    bool importFromBDB( void);
    OUString m_MapFileName;
#endif
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

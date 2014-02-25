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

#ifndef INCLUDED_SHELL_SOURCE_UNIX_SYSSHELL_RECENTLY_USED_FILE_HXX
#define INCLUDED_SHELL_SOURCE_UNIX_SYSSHELL_RECENTLY_USED_FILE_HXX

#include <stdio.h>
#include <unistd.h>

// simple wrapper around the recently_used_file

class recently_used_file
{
public:

    recently_used_file();


    ~recently_used_file();


    // set file pointer to the start of file
    void reset() const;


    void truncate(off_t length = 0);


    size_t read(
        char* buffer,
        size_t size) const;


    void write(const char* buffer, size_t size) const;


    bool eof() const;

private:
    FILE* file_;
};

#endif // INCLUDED_SHELL_SOURCE_UNIX_SYSSHELL_RECENTLY_USED_FILE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

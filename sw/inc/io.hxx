/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// Special class for IO. Used for system-independent representation
// (change of byte-order, conversion of characters)
// Writes in binary format for efficiency.
#ifndef _IO_HXX
#define _IO_HXX

#ifdef UNX
#include <unistd.h>
#else
#include <io.h>
#endif

#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>


#include <vcl/keycod.hxx>
#include <tools/stream.hxx>


class SwIOin {
private:
        SvFileStream aStr; //$ ifstream
public:
    // Stream is created in respective mode.
    SwIOin(const String &rFilename, StreamMode nMode =
                                    STREAM_READ | STREAM_NOCREATE );

    SwIOin& operator>>(char& val);
    SwIOin& operator>>(unsigned char& val);
    SwIOin& operator>>(char* val);
    SwIOin& operator>>(unsigned char* val);
    SwIOin& operator>>(short& val);
    SwIOin& operator>>(unsigned short& val);
    SwIOin& operator>>(long& val);
    SwIOin& operator>>(unsigned long& val);
    String ReadString();
    KeyCode ReadKeyCode();
    // Can be extended for more arrays of base types.
    // nLen is count of elements.
    SwIOin& Read(char *buf, unsigned nLen);

    int operator!() { return aStr.GetError() != SVSTREAM_OK; }
        SvFileStream &operator()() {
        return aStr;
    }
};

class SwIOout {
private:
    void _write(const char *buf, unsigned size);
    SvFileStream aStr; //$ ofstream
public:
    // Stream is created in respective mode.
    SwIOout( const String &rFilename, StreamMode nMode =
                                      STREAM_WRITE | STREAM_NOCREATE );
    SwIOout& operator<<(char val);
    SwIOout& operator<<(unsigned char val);
    SwIOout& operator<<(char* val);
    SwIOout& operator<<(unsigned char* val);
    SwIOout& operator<<(short val);
    SwIOout& operator<<(unsigned short val);
    SwIOout& operator<<(long val);
    SwIOout& operator<<(unsigned long val);
    SwIOout& operator<<(const String &);
    SwIOout& operator<<(const KeyCode &);
    // Can be extended for more arrays of base types.
    // nLen is count of elements.
    SwIOout& Write(const char *buf, unsigned nLen);

    int operator!() { return aStr.GetError() != SVSTREAM_OK; }
    SvFileStream &operator()() {
        return aStr;
    }
};


class SwIOinout {
private:
    SvFileStream aStr; //$ fstream

public:
    // Stream is created in respective mode.
    SwIOinout(const String &rFilename, StreamMode nMode =
                                       STREAM_READWRITE  | STREAM_NOCREATE );

    SwIOinout& operator>>(char& val);
    SwIOinout& operator>>(unsigned char& val);
    SwIOinout& operator>>(char* val);
    SwIOinout& operator>>(unsigned char* val);
    SwIOinout& operator>>(short& val);
    SwIOinout& operator>>(unsigned short& val);
    SwIOinout& operator>>(long& val);
    SwIOinout& operator>>(unsigned long& val);
    String ReadString();
    KeyCode ReadKeyCode();
    // Can be extended for more arrays of base types.
    // nLen is count of elements.
    SwIOinout& Read(char *buf, unsigned nLen);
    SwIOinout& Read(unsigned short *buf, unsigned nLen );

    SwIOinout& operator<<(char val);
    SwIOinout& operator<<(unsigned char val);
    SwIOinout& operator<<(char* val);
    SwIOinout& operator<<(unsigned char* val);
    SwIOinout& operator<<(short val);
    SwIOinout& operator<<(unsigned short val);
    SwIOinout& operator<<(long val);
    SwIOinout& operator<<(unsigned long val);
    SwIOinout& operator<<(const String &);
    SwIOinout& operator<<(const KeyCode &);
    // Can be extended for more arrays of base types.
    // nLen is count of elements.
    SwIOinout& Write(const char *buf, unsigned nLen);

    int operator!() { return aStr.GetError() != SVSTREAM_OK; }
        SvFileStream &operator()() {
        return aStr;
    }

    BOOL Ok();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <tools/string.hxx>
#include <vector>
#define PATH_SEP ":"
#define DIR_SEP "/"

typedef ::std::vector< ByteString* > ByteStringList;

class CppDep
{
    ByteString          aSourceFile;
    ByteStringList      *pSearchPath;

protected:
    ByteStringList      *pFileList;
    ByteStringList      *pSources;

    BOOL            Search( ByteString aFileName );
    ByteString      Exists( ByteString aFileName );

    ByteString      IsIncludeStatement( ByteString aLine );
public:
                    CppDep( ByteString aFileName );
                    CppDep();
                    virtual ~CppDep();
    virtual void    Execute();

    ByteStringList*     GetDepList(){return pFileList;}
    BOOL            AddSearchPath( const char* aPath );
    BOOL            AddSource( const char * aSource );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

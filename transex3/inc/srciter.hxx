/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: srciter.hxx,v $
 * $Revision: 1.7 $
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

#ifndef TRANSEX_DIRECTORY_HXX
#define TRANSEX_DIRECTORY_HXX
#include <transex3/directory.hxx>
#endif

// class SourceTreeIterator
//

class SourceTreeIterator
{
private:
    transex::Directory aRootDirectory;
    bool bInExecute;

    void ExecuteDirectory( transex::Directory& pDirectory );

protected:
    bool bLocal;
    bool bSkipLinks;

public:
    SourceTreeIterator( const ByteString &rRootDirectory, const ByteString &rVersion , bool bLocal_in = false);
    virtual ~SourceTreeIterator();

    BOOL StartExecute();
    void EndExecute();

    virtual void OnExecuteDirectory( const rtl::OUString &rDirectory );
};



/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cppdep.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:28:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <list.hxx>
#include <string.hxx>
#define PATH_SEP ":"
#define DIR_SEP "/"

DECLARE_LIST( ByteStringList, ByteString * );

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
                    ~CppDep();
    virtual void    Execute();

    ByteStringList*     GetDepList(){return pFileList;}
    BOOL            AddSearchPath( const char* aPath );
    BOOL            AddSource( const char * aSource );
};


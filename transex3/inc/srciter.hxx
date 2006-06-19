/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srciter.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 17:19:37 $
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

#include <bootstrp/listmacr.hxx>
#include <bootstrp/sstring.hxx>
#include <bootstrp/prj.hxx>
#include <bootstrp/mkcreate.hxx>

//
// class SourceTreeIterator
//

class SourceTreeIterator
{
private:
    SourceDirectory *pRootDirectory;
    BOOL bInExecute;

    void ExecuteDirectory( SourceDirectory *pDirectory );

protected:
    bool bLocal;

public:
    SourceTreeIterator( const ByteString &rRootDirectory, const ByteString &rVersion , bool bLocal_in = false);
    virtual ~SourceTreeIterator();

    BOOL StartExecute();
    void EndExecute();

    virtual void OnExecuteDirectory( const ByteString &rDirectory );
};



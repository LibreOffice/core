/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srciter.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:02:21 $
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

#include "srciter.hxx"
#include <stdio.h>

//
// class SourceTreeIterator
//

/*****************************************************************************/
SourceTreeIterator::SourceTreeIterator(
    const ByteString &rRootDirectory, const ByteString &rVersion , bool bLocal )
/*****************************************************************************/
                : pRootDirectory( NULL ),
                bInExecute( FALSE ) , bLocal( bLocal )
{
    if(!bLocal){
        pRootDirectory = SourceDirectory::CreateRootDirectory(
            rRootDirectory, rVersion, TRUE );
        //fprintf( stderr, "\n" );
    }
    else
        pRootDirectory = 0;
}

/*****************************************************************************/
SourceTreeIterator::~SourceTreeIterator()
/*****************************************************************************/
{
    delete pRootDirectory;
}

/*****************************************************************************/
void SourceTreeIterator::ExecuteDirectory( SourceDirectory *pDirectory )
/*****************************************************************************/
{
    if ( bInExecute ) {
        OnExecuteDirectory( pDirectory->GetFullPath());
        if ( pDirectory->GetSubDirectories())
            for ( ULONG i=0;i < pDirectory->GetSubDirectories()->Count();i++ )
                ExecuteDirectory(( SourceDirectory * )
                    pDirectory->GetSubDirectories()->GetObject( i ));
    }
}

/*****************************************************************************/
BOOL SourceTreeIterator::StartExecute()
/*****************************************************************************/
{
    if ( pRootDirectory ) {
        bInExecute = TRUE;

        ExecuteDirectory( pRootDirectory );
    }

    if ( bInExecute ) {
        bInExecute = FALSE;
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************/
void SourceTreeIterator::EndExecute()
/*****************************************************************************/
{
    bInExecute = FALSE;
}

/*****************************************************************************/
void SourceTreeIterator::OnExecuteDirectory( const ByteString &rDirectory )
/*****************************************************************************/
{
    fprintf( stdout, "%s\n", rDirectory.GetBuffer());
}

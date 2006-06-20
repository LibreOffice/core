/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filelist.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:52:50 $
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

#include<tools/list.hxx>
#include<tools/stream.hxx>
#include<tools/string.hxx>
#include<tools/rtti.hxx>
#include<exchange.hxx>
#include<filelist.hxx>

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

TYPEINIT1_AUTOFACTORY( FileList, SvDataCopyStream );

// String-Liste zum Speichern der Namen deklarieren
DECLARE_LIST( FileStringList, String* )


/*************************************************************************
|*
|*    FileList - Ctor/Dtor
|*
\*************************************************************************/

FileList::FileList()
{
    pStrList = new FileStringList();
}

FileList::~FileList()
{
    ClearAll();
}

void FileList::ClearAll( void )
{
    // Strings in der Liste loeschen
    ULONG nCount = pStrList->Count();
    for( ULONG i = 0 ; i < nCount ; i++ )
        delete pStrList->GetObject( i );

    // Liste loeschen
    delete pStrList;
}

/*************************************************************************
|*
|*    FileList - Zuweisungsoperator
|*
\*************************************************************************/

FileList& FileList::operator=( const FileList& rFileList )
{
    // Liste duplizieren
    *pStrList = *rFileList.pStrList;

    // Strings in der Liste kopieren
    ULONG nCount = pStrList->Count();
    for( ULONG i = 0 ; i < nCount ; i++ )
        pStrList->Replace( new String( *rFileList.pStrList->GetObject( i ) ), i );

    return *this;
}

/*************************************************************************
|*
|*    FileList::GetFormatName()
|*
\*************************************************************************/

ULONG FileList::GetFormat()
{
    return FORMAT_FILE_LIST;
}

/******************************************************************************
|*
|*  virtuelle SvData-Methoden
|*
\******************************************************************************/

void FileList::Load( SvStream& rIStm )
{
    rIStm >> *this;
}

void FileList::Save( SvStream& rOStm )
{
    rOStm << *this;
}

void FileList::Assign( const SvDataCopyStream& rCopyStream )
{
    *this = (const FileList&)rCopyStream;
}

/******************************************************************************
|*
|*  Stream-Operatoren
|*
\******************************************************************************/

/*
 * NOTE: to correctly handle this Protocol with Unicode, native Win32 must be called:
 * e.g. DropQueryFile
 */

SvStream& operator<<( SvStream& rOStm, const FileList& /*rFileList*/ )
{
    OSL_ENSURE(false, "Not implemented!");
    return rOStm;
}

/* #i28176#
   The Windows clipboard bridge now provides a double '\0'
   terminated list of file names for format SOT_FORMAT_FILE_LIST
   instead of the original Windows Sv_DROPFILES structure. All strings
   in this list are UTF16 strings. Shell link files will be already
   resolved by the Windows clipboard bridge.*/
SvStream& operator>>( SvStream& rIStm, FileList& rFileList )
{
    rFileList.ClearAll();
    rFileList.pStrList = new FileStringList();

    String aStr;
    sal_uInt16 c;

    while (!rIStm.IsEof())
    {
        aStr.Erase();

        // read first character of filepath; c==0 > reach end of stream
        rIStm >> c;
        if (!c)
            break;

        // read string till c==0
        while (c && !rIStm.IsEof())
        {
            aStr += (sal_Unicode)c;
            rIStm >> c;
        }

        // append the filepath
        rFileList.AppendFile(aStr);
    }
    return rIStm;
}

/******************************************************************************
|*
|*  Liste fuellen/abfragen
|*
\******************************************************************************/

void FileList::AppendFile( const String& rStr )
{
    pStrList->Insert( new String( rStr ) , pStrList->Count() );
}

String FileList::GetFile( ULONG i ) const
{
    String aStr;
    if( i < pStrList->Count() )
        aStr = *pStrList->GetObject( i );
    return aStr;
}

ULONG FileList::Count( void ) const
{
    return pStrList->Count();
}


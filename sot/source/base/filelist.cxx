/*************************************************************************
 *
 *  $RCSfile: filelist.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:56:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/



#include<tools/list.hxx>
#include<tools/stream.hxx>
#include<tools/string.hxx>
#include<tools/rtti.hxx>
#include<exchange.hxx>
#include<filelist.hxx>
#pragma hdrstop

TYPEINIT1_AUTOFACTORY( FileList, SvDataCopyStream );


// String-Liste zum Speichern der Namen deklarieren
DECLARE_LIST( FileStringList, String* );


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

// Windows-Struktur nachdefinieren
// Typen entsprechend der Groesse der Original-Typen gewaehlt
// In den Kommentaren sind jeweils die Original-Typen angegeben.
// Hier werden immer die 32-Bit-Varianten der Typen gewaehlt, um
// eine einheitliche Struktur fuer alle Plattformen zu erhalten.
// Diese Struktor ist zwar nicht Win16-kompatibel, dort kann aber
// die Struktur auch nicht direkt per Drag&Drop uebertragen werden.
struct Sv_DROPFILES
{
    /*DWORD = unsigned long*/   UINT32 pFiles;  // offset of file list
    /*POINT = { long, long }*/  INT32 pt_x;     // drop point
                                INT32 pt_y;
    /*BOOL = int*/              INT32 fNC;
    /*BOOL = int*/              INT32 fWide;    // wide character-flag

    // Konstruktor
    Sv_DROPFILES( void )
    {
        pFiles  = 20;   // 5 x 4 Bytes, sizeof gefaehrlich wegen alignment
        pt_x    = 0L;
        pt_y    = 0L;
        fNC     = FALSE;
        fWide   = FALSE; // NOTE: Unicode not allowed for Windows 95 !!
    }

};

// Stream-Operatoren fuer Sv_DROPFILES
SvStream& operator<<( SvStream& rOStm, const Sv_DROPFILES& r )
{
    rOStm << r.pFiles << r.pt_x << r.pt_y << r.fNC << r.fWide;
    return rOStm;
}
SvStream& operator>>( SvStream& rIStm, Sv_DROPFILES& r )
{
    rIStm >> r.pFiles >> r.pt_x >> r.pt_y >> r.fNC >> r.fWide;
    return rIStm;
}

/*
 * NOTE: to correctly handle this Protocol with Unicode, native Win32 must be called:
 * e.g. DropQueryFile
 */

SvStream& operator<<( SvStream& rOStm, const FileList& rFileList )
{
    // Windows-kompatible Struktur anlegen und streamen
    Sv_DROPFILES aSv_DROPFILES;
    rOStm << aSv_DROPFILES;

    // String-Liste anhaengen
    ULONG nCount = rFileList.pStrList->Count();
    for( ULONG i = 0 ; i < nCount ; i++ )
    {
        String* pStr = rFileList.pStrList->GetObject( i );
//      rOStm << pStr->GetBuffer();
        rOStm << "invalid.txt"; //

        // Noch eine 0 anhaengen
        rOStm << (char)0;
    }
    // Noch eine 0 anhaengen
    rOStm << (char)0;

    return rOStm;
}

SvStream& operator>>( SvStream& rIStm, FileList& rFileList )
{
    // Windows-kompatible Struktur anlegen und aus Stream lesen
    Sv_DROPFILES aSv_DROPFILES;
    rIStm >> aSv_DROPFILES;

    // Bestehende Liste loeschen und neue anlegen
    rFileList.ClearAll();
    rFileList.pStrList = new FileStringList();

    // String-Liste aufbauen
    ByteString aStr;
    char c, cLast;

    // 1. Zeichen lesen
    rIStm >> c;
    do
    {
        aStr = ByteString();

        // String bis '\0' lesen
        do
        {
            if( c )
                aStr += c;
            cLast = c;      // Zeichen merken

            // Bei Unicode eins mehr weg
            if( aSv_DROPFILES.fWide )
                rIStm >> c;

            rIStm >> c;
        }
        while( cLast );

        // String in die Liste stopfen
        rFileList.AppendFile( String(aStr, RTL_TEXTENCODING_ASCII_US));
    }
    while( c );     // c == 0 && cLast == 0 -> Ende

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




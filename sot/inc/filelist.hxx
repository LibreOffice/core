/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filelist.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:35:13 $
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

#ifndef _FILELIST_HXX
#define _FILELIST_HXX

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef INCLUDED_SOTDLLAPI_H
#include "sot/sotdllapi.h"
#endif

class FileStringList;

class SOT_DLLPUBLIC FileList : public SvDataCopyStream
{
    FileStringList*     pStrList;

protected:

    // SvData-Methoden
    virtual void        Load( SvStream& );
    virtual void        Save( SvStream& );
    virtual void        Assign( const SvDataCopyStream& );

    // Liste loeschen;
    void ClearAll( void );

public:

    TYPEINFO();
                        FileList();
                        ~FileList();

    // Zuweisungsoperator
    FileList&           operator=( const FileList& rFileList );


    // Im-/Export
    SOT_DLLPUBLIC friend SvStream&  operator<<( SvStream& rOStm, const FileList& rFileList );
    SOT_DLLPUBLIC friend SvStream&  operator>>( SvStream& rIStm, FileList& rFileList );

    // Clipboard, D&D usw.
    static ULONG        GetFormat();


    // Liste fuellen/abfragen
    void AppendFile( const String& rStr );
    String GetFile( ULONG i ) const;
    ULONG Count( void ) const;

};

#endif // _FILELIST_HXX


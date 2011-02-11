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

#ifndef _FILELIST_HXX
#define _FILELIST_HXX

#include <tools/stream.hxx>
#include "sot/sotdllapi.h"

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
    static sal_uLong        GetFormat();


    // Liste fuellen/abfragen
    void AppendFile( const String& rStr );
    String GetFile( sal_uLong i ) const;
    sal_uLong Count( void ) const;

};

#endif // _FILELIST_HXX


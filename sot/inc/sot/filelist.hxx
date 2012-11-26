/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


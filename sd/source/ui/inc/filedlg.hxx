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




#ifndef _FILEDLG_HXX
#define _FILEDLG_HXX

#include <tools/string.hxx>
#include <tools/errcode.hxx>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#include "sddllapi.h"

class SdFileDialog_Imp;


/******************************************************************************/

/**
   The class SdOpenSoundFileDialog wraps the FileDialogHelper, displaying the
   FILEOPEN_PLAY dialog template and performing the 'preview' functionality
   (playing the selected sound file). The interface is a downstripped version
   of the aforementioned class, with similar semantics.
 */
class SD_DLLPUBLIC SdOpenSoundFileDialog
{
    const std::auto_ptr< SdFileDialog_Imp > mpImpl;

    // forbidden and not implemented
    SdOpenSoundFileDialog (const SdOpenSoundFileDialog &);
    SdOpenSoundFileDialog & operator= (const SdOpenSoundFileDialog &);

public:
                            SdOpenSoundFileDialog();
                            ~SdOpenSoundFileDialog();

    ErrCode                  Execute();
    String                   GetPath() const;
    void                     SetPath( const String& rPath );

    String                   ReqDisplayDirectory() const;
};

#endif // _FILEDLG_HXX


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



#ifndef _SVT_FILEDLG_HXX
#define _SVT_FILEDLG_HXX

#include "svtools/svtdllapi.h"

#ifndef _DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

class Edit;
class ImpSvFileDlg;

// --------------
// - SvPathDialog -
// --------------

class SVT_DLLPUBLIC PathDialog : public ModalDialog
{
private:
    friend class FileDialog;    // Imp...

    ImpSvFileDlg*       pImpFileDlg;    // Implementation
    Link                aOKHdlLink;     // Link zum OK-Handler

protected:
    UniString           aDfltExt;       // Default - Extension

public:
                        PathDialog( Window* pParent, WinBits nWinStyle = 0, sal_Bool bCreateDir = sal_True );
                        ~PathDialog();

    virtual long        OK();

    void                SetPath( const UniString& rNewPath );
    void                SetPath( const Edit& rEdit );
    UniString               GetPath() const;

    void                SetOKHdl( const Link& rLink ) { aOKHdlLink = rLink; }
    const Link&         GetOKHdl() const { return aOKHdlLink; }

    virtual short       Execute();
};

// --------------
// - SvFileDialog -
// --------------

class SVT_DLLPUBLIC FileDialog : public PathDialog
{
private:
    Link                aFileHdlLink;   // Link zum FileSelect-Handler
    Link                aFilterHdlLink; // Link zum FilterSelect-Handler

public:
                        FileDialog( Window* pParent, WinBits nWinStyle );
                        ~FileDialog();

    virtual void        FileSelect();
    virtual void        FilterSelect();

    void                SetDefaultExt( const UniString& rExt ) { aDfltExt = rExt; }
    const UniString&        GetDefaultExt() const { return aDfltExt; }
    void                AddFilter( const UniString& rFilter, const UniString& rType );
    void                RemoveFilter( const UniString& rFilter );
    void                RemoveAllFilter();
    void                SetCurFilter( const UniString& rFilter );
    UniString               GetCurFilter() const;
    sal_uInt16              GetFilterCount() const;
    UniString               GetFilterName( sal_uInt16 nPos ) const;
    UniString               GetFilterType( sal_uInt16 nPos ) const;

    void                SetFileSelectHdl( const Link& rLink ) { aFileHdlLink = rLink; }
    const Link&         GetFileSelectHdl() const { return aFileHdlLink; }
    void                SetFilterSelectHdl( const Link& rLink ) { aFilterHdlLink = rLink; }
    const Link&         GetFilterSelectHdl() const { return aFilterHdlLink; }

    void                SetOkButtonText( const UniString& rText );
    void                SetCancelButtonText( const UniString& rText );
};

#endif  // _FILEDLG_HXX

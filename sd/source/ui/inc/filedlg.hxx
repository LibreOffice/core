/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filedlg.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:28:39 $
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


#ifndef _FILEDLG_HXX
#define _FILEDLG_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

class SdFileDialog_Imp;

/******************************************************************************/

/**
   The class SdExportFileDialog wraps the FileDialogHelper, displaying the
   FILESAVE_AUTOEXTENSION_SELECTION dialog template. The interface is a downstripped
   version of the aforementioned class, with similar semantics.
 */
class SdExportFileDialog
{
    const std::auto_ptr< SdFileDialog_Imp > mpImpl;

    // forbidden and not implemented
    SdExportFileDialog ();
    SdExportFileDialog (const SdExportFileDialog &);
    SdExportFileDialog & operator= (const SdExportFileDialog &);

public:
    explicit                SdExportFileDialog( BOOL haveCheckbox );
                            ~SdExportFileDialog();

    ErrCode                 Execute();
    String                  GetPath() const;
    void                    SetPath( const String& rPath );

    String                  ReqDisplayDirectory() const;

    String                  ReqCurrentFilter() const;
    BOOL                    IsExportSelection() const;  // whether the "selection" checkbox is checked.
};


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


/*************************************************************************
 *
 *  $RCSfile: filedlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2001-07-02 16:11:08 $
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


#ifndef _FILEDLG_HXX
#define _FILEDLG_HXX

#ifndef  _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif


#define FILE_OPEN_SERVICE_NAME      "com.sun.star.ui.dialogs.FilePicker"


namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace ui
            {
                namespace dialogs
                {
                    class XFilterManager;
                    class XFilePicker;
                    class XFilePickerListener;
                    class XFilePickerControlAccess;
                }
            }
        }
    }
}



const short SDFILEDIALOG_EXPORT             = 0;
const short SDFILEDIALOG_EXPORT_SELECTION   = 1;
const short SDFILEDIALOG_OPEN_SOUND         = 2;

class SdFileDialogHelper;


/******************************************************************************/


class SdExportFileDialog
{
    ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePickerListener > mxImp;
    SdFileDialogHelper      *mpImp;

public:
                            SdExportFileDialog( BOOL haveCheckbox );
                            ~SdExportFileDialog();

    ErrCode                 Execute();
    String                  GetPath() const;
    void                    SetPath( const String& rPath );

    String                  ReqDisplayDirectory() const;

    String                  ReqCurrFilter() const;
    BOOL                    IsSelectedBoxChecked() const;
    BOOL                    IsExtensionBoxChecked() const;
};


/******************************************************************************/


class SdOpenSoundFileDialog
{
    ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePickerListener > mxImp;
    SdFileDialogHelper      *mpImp;

public:
                            SdOpenSoundFileDialog();
                            ~SdOpenSoundFileDialog();

    ErrCode                  Execute();
    String                   GetPath() const;
    void                     SetPath( const String& rPath );

    String                   ReqDisplayDirectory() const;
};

#endif // _FILEDLG_HXX


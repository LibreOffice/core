/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pickerhelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 10:03:46 $
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

#ifndef _PICKERHELPER_HXX
#define _PICKERHELPER_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef  _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif

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
                    class XFilePicker;
                    class XFolderPicker;
                }
            }
        }
    }
}


namespace svt
{

    SVL_DLLPUBLIC void SetDialogHelpId(
        ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > _mxFileDlg,
        sal_Int32 _nHelpId );

    SVL_DLLPUBLIC void SetDialogHelpId(
        ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFolderPicker > _mxFileDlg,
        sal_Int32 _nHelpId );

}

//-----------------------------------------------------------------------------

#endif

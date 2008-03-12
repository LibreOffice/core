/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vistatypes.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:35:47 $
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

#ifndef FPICKER_WIN32_VISTA_TYPES_HXX
#define FPICKER_WIN32_VISTA_TYPES_HXX

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "comptr.hxx"
#include <shobjidl.h>

//-----------------------------------------------------------------------------
// namespace
//-----------------------------------------------------------------------------

#ifdef css
    #error "Clash on using CSS as namespace define."
#else
    #define css ::com::sun::star
#endif

namespace fpicker{
namespace win32{
namespace vista{

//-----------------------------------------------------------------------------
// types, const etcpp.
//-----------------------------------------------------------------------------

typedef ComPtr< IFileDialog         , IID_IFileDialog                             > TFileDialog;
typedef ComPtr< IFileOpenDialog     , IID_IFileOpenDialog  , CLSID_FileOpenDialog > TFileOpenDialog;
typedef ComPtr< IFileSaveDialog     , IID_IFileSaveDialog  , CLSID_FileSaveDialog > TFileSaveDialog;
typedef ComPtr< IFileDialogEvents   , IID_IFileDialogEvents                       > TFileDialogEvents;
typedef ComPtr< IFileDialogCustomize, IID_IFileDialogCustomize                    > TFileDialogCustomize;

} // namespace vista
} // namespace win32
} // namespace fpicker

#undef css

#endif // FPICKER_WIN32_VISTA_TYPES_HXX

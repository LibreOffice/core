/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: getfilenamewrapper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:53:35 $
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

#ifndef _GETFILENAMEWRAPPER_HXX_
#define _GETFILENAMEWRAPPER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <commdlg.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

/*
    A simple wrapper around the GetOpenFileName/GetSaveFileName API.
    Because currently the Win32 API functions GetOpenFileName/GetSaveFileName
    work only properly in an Single Threaded Appartment.
*/

class CGetFileNameWrapper
{
public:
    CGetFileNameWrapper();

    bool getOpenFileName(LPOPENFILENAME lpofn);
    bool getSaveFileName(LPOPENFILENAME lpofn);
    int  commDlgExtendedError();

private:
    int m_ExtendedDialogError;
};

#endif

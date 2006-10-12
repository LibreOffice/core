/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customcontrolfactory.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:51:30 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

#include <tchar.h>

#ifndef _CUSTOMCONTROLFACTORY_HXX_
#include "customcontrolfactory.hxx"
#endif

#ifndef _CUSTOMCONTROLCONTAINER_HXX_
#include "customcontrolcontainer.hxx"
#endif

#ifndef _DIALOGCUSTOMCONTROLS_HXX_
#include "dialogcustomcontrols.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//-----------------------------------
//
//-----------------------------------

CCustomControl* CCustomControlFactory::CreateCustomControl(HWND aControlHandle, HWND aParentHandle)
{
    OSL_PRECOND(IsWindow(aControlHandle),"Invalid control handle");
    OSL_PRECOND(IsWindow(aControlHandle),"Invalid parent handle");

    // get window class
    // if static text create static text control etc.

    TCHAR aClsName[256];
    ZeroMemory(aClsName,sizeof(aClsName));
    if (GetClassName(aControlHandle,aClsName,sizeof(aClsName)) == 0) {
        OSL_ENSURE(false,"Invalid window handle");
    }

    if (0 == _tcsicmp(aClsName,TEXT("button")))
    {
        // button means many things so we have
        // to find out what button it is
        LONG lBtnStyle = GetWindowLong(aControlHandle,GWL_STYLE);

        if (lBtnStyle & BS_CHECKBOX)
            return new CCheckboxCustomControl(aControlHandle,aParentHandle);

        if ( ((lBtnStyle & BS_PUSHBUTTON) == 0) || (lBtnStyle & BS_DEFPUSHBUTTON))
            return new CPushButtonCustomControl(aControlHandle,aParentHandle);

        return new CDummyCustomControl(aControlHandle,aParentHandle);
    }

    if (0 == _tcsicmp(aClsName,TEXT("listbox")) || 0 == _tcsicmp(aClsName,TEXT("combobox")))
        return new CComboboxCustomControl(aControlHandle,aParentHandle);

    if (0 == _tcsicmp(aClsName,TEXT("static")))
        return new CStaticCustomControl(aControlHandle,aParentHandle);

    return new CDummyCustomControl(aControlHandle,aParentHandle);
}

//-----------------------------------
//
//-----------------------------------

CCustomControl* CCustomControlFactory::CreateCustomControlContainer()
{
    return new CCustomControlContainer();
}

/*************************************************************************
 *
 *  $RCSfile: customcontrolfactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:54:53 $
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
    int nRet = GetClassName(aControlHandle,aClsName,sizeof(aClsName));

    OSL_ENSURE(nRet,"Invalid window handle");

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

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVDVITER_HXX
#define _SVDVITER_HXX

#include <tools/solar.h>
#include <sal/types.h>
#include "svx/svxdllapi.h"

/*

Using this class, you can find out:
- SdrView* First/NextView()
  - All Views in which a model is displayed
  - All Views in which a certain page is visible (also as MasterPage)
  - All Views in which a certain object is visible (also as MasterPage)
- SdrPageView* First/NextPageView()
  - All PageViews in which a model is displayed
  - All PageViews in which a certain page is visible (also as MasterPage)
  - All PageViews in which a certain object is visible (also as MasterPage)
- OutputDevice* First/NextOutDev()
  - All OutputDevices in which a model is displayed
  - All OutputDevices in which a certain page is visible (also as MasterPage)
  - All OutputDevices in which a certain object is visible (also as MasterPage)
- Window* First/NextWindow()
  - All Windows in which a model is displayed
  - All Windows in which a certain page is visible (also as MasterPage)
  - All Windows in which a certain object is visible (also as MasterPage)
You can specify if the selection is restricted to a certain page/object by
choosing the corresponding constructor.

Among others, the following are considered:
- Visibility state of the layer
- Visible Layer of MasterPages
- Multiple layer for group objects

Not considered are:
- Whether the pages/objects were already painted or if an Invalidate is still pending
- Whether the pages/objects in a Window are in a visible region

*/

////////////////////////////////////////////////////////////////////////////////////////////////////

class OutputDevice;
class Window;
class SdrView;
class SdrPageView;
class SdrModel;
class SdrPage;
class SdrObject;
class SetOfByte;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrViewIter
{
    const SdrModel*                                     mpModel;
    const SdrPage*                                      mpPage;
    const SdrObject*                                    mpObject;
    SdrView*                                            mpAktView;

    sal_uInt32                                          mnListenerNum;
    sal_uInt32                                          mnPageViewNum;
    sal_uInt32                                          mnOutDevNum;

    // bitfield
    unsigned                                            mbNoMasterPage : 1;

private:
    SVX_DLLPRIVATE void          ImpInitVars();
    SVX_DLLPRIVATE SdrView*      ImpFindView();
    SVX_DLLPRIVATE SdrPageView*  ImpFindPageView();
    SVX_DLLPRIVATE OutputDevice* ImpFindOutDev();
    SVX_DLLPRIVATE Window*       ImpFindWindow();
    SVX_DLLPRIVATE sal_Bool      ImpCheckPageView(SdrPageView* pPV) const;

public:
    SdrViewIter(const SdrModel* pModel);
    SdrViewIter(const SdrPage* pPage, sal_Bool bNoMasterPage = sal_False);
    SdrViewIter(const SdrObject* pObject, sal_Bool bNoMasterPage = sal_False);

    SdrView* FirstView();
    SdrView* NextView();

    SdrPageView* FirstPageView();
    SdrPageView* NextPageView();

    OutputDevice* FirstOutDev();
    OutputDevice* NextOutDev();

    Window* FirstWindow();
    Window* NextWindow();
};

#endif //_SVDVITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

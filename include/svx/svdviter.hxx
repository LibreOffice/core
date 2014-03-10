/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SVDVITER_HXX
#define INCLUDED_SVX_SVDVITER_HXX

#include <sal/types.h>
#include <svx/svxdllapi.h>

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

class OutputDevice;
class Window;
class SdrView;
class SdrPageView;
class SdrModel;
class SdrPage;
class SdrObject;
class SetOfByte;

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
    bool                                                mbNoMasterPage : 1;

private:
    SVX_DLLPRIVATE void          ImpInitVars();
    SVX_DLLPRIVATE SdrView*      ImpFindView();
    SVX_DLLPRIVATE bool          ImpCheckPageView(SdrPageView* pPV) const;

public:
    SdrViewIter(const SdrPage* pPage, bool bNoMasterPage = false);
    SdrViewIter(const SdrObject* pObject, bool bNoMasterPage = false);

    SdrView* FirstView();
    SdrView* NextView();
};

#endif // INCLUDED_SVX_SVDVITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

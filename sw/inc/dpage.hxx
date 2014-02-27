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
#ifndef INCLUDED_SW_INC_DPAGE_HXX
#define INCLUDED_SW_INC_DPAGE_HXX

#include <svx/fmpage.hxx>
#include <svx/svdobj.hxx>

class SdrPageGridFrameList;
class SwDrawDocument;
class SwDoc;

class SwDPage : public FmFormPage, public SdrObjUserCall
{
    SdrPageGridFrameList*   pGridLst;
    SwDoc&                  rDoc;

public:
    SwDPage(SwDrawDocument& rNewModel, sal_Bool bMasterPage=sal_False);
    ~SwDPage();

    // #i3694#
    // This GetOffset() method is not needed anymore, it even leads to errors.
    // virtual Point GetOffset() const;
    virtual SdrObject* ReplaceObject( SdrObject* pNewObj, sal_uLong nObjNum );

    virtual const SdrPageGridFrameList* GetGridFrameList(const SdrPageView* pPV,
                                    const Rectangle *pRect) const;

    sal_Bool RequestHelp( Window* pWindow, SdrView* pView, const HelpEvent& rEvt );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();
};

#endif // INCLUDED_SW_INC_DPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

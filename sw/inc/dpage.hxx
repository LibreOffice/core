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
#ifndef _DPAGE_HXX
#define _DPAGE_HXX

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

#endif     // _DPAGE_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

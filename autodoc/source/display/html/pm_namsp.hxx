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

#ifndef ADC_DISPLAY_HTML_PM_NAMSP_HXX
#define ADC_DISPLAY_HTML_PM_NAMSP_HXX



// USED SERVICES
    // BASE CLASSES
#include "pm_base.hxx"
    // COMPONENTS
    // PARAMETERS

class ChildList_Display;
class NavigationBar;

class PageMaker_Namespace : public SpecializedPageMaker
{
  public:
                        PageMaker_Namespace(
                            PageDisplay &       io_rPage );

    virtual				~PageMaker_Namespace();

    virtual void        MakePage();

  private:
    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
    virtual void        Write_ChildList(
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_nListTitle,
                            const char *        i_nLabel );
    void                Write_ChildLists_forClasses(
                            const char *         i_sListTitle,
                            const char *         i_sLabel,
                            ary::cpp::E_ClassKey i_eFilter );

    const ary::cpp::Namespace &
                        Me() const              { return *pMe; }
    // DATA
    const ary::cpp::Namespace *
                        pMe;
    Dyn<ChildList_Display>
                        pChildDisplay;
    Dyn<NavigationBar>  pNavi;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

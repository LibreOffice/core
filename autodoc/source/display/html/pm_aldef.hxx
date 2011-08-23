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

#ifndef ADC_DISPLAY_HTML_PM_ALDEF_HXX
#define ADC_DISPLAY_HTML_PM_ALDEF_HXX

// BASE CLASSES
#include "pm_base.hxx"
// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>
using ary::cpp::De_id;

namespace ary
{
    namespace cpp
    {
        class DefineEntity;
    }
}

class Docu_Display;
class NavigationBar;





class PageMaker_AllDefs : public SpecializedPageMaker
{
  public:
                        PageMaker_AllDefs(
                            PageDisplay &       io_rPage );

    virtual				~PageMaker_AllDefs();

    virtual void        MakePage();

  private:
    typedef std::vector<De_id>              List_Ids;
    typedef List_Ids::const_iterator        ids_iterator;

    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
    virtual void        Write_DefinesList();
    virtual void        Write_MacrosList();
    void                Write_Define(
                            De_id               i_nId );
    void                Write_Macro(
                            De_id               i_nId );
    void                Write_DefsDocu(
                            csi::xml::Element & o_rOut,
                            const ary::cpp::DefineEntity &
                                                i_rTextReplacement );

    // DATA
    Dyn<Docu_Display>   pDocuDisplay;
    Dyn<NavigationBar>  pNavi;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

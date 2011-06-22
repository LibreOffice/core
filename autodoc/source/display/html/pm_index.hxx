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

#ifndef ADC_DISPLAY_HTML_PM_INDEX_HXX
#define ADC_DISPLAY_HTML_PM_INDEX_HXX

// BASE CLASSES
#include "pm_base.hxx"
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
// USED SERVICES
namespace adcdisp
{
     class IndexList;
}
namespace ary
{
    namespace cpp
    {
        class Namespace;
        class Class;
        class Enum;
        class Typedef;
        class Function;
        class Variable;
        class EnumValue;
        class Define;
        class Macro;
    }
}

class NavigationBar;




class PageMaker_Index : public SpecializedPageMaker,
                        public ary::Display,
                        public csv::ConstProcessor<ary::cpp::Namespace>,
                        public csv::ConstProcessor<ary::cpp::Class>,
                        public csv::ConstProcessor<ary::cpp::Enum>,
                        public csv::ConstProcessor<ary::cpp::Typedef>,
                        public csv::ConstProcessor<ary::cpp::Function>,
                        public csv::ConstProcessor<ary::cpp::Variable>,
                        public csv::ConstProcessor<ary::cpp::EnumValue>,
                        public csv::ConstProcessor<ary::cpp::Define>,
                        public csv::ConstProcessor<ary::cpp::Macro>
{
  public:
                        PageMaker_Index(
                            PageDisplay &       io_rPage,
                            char                i_c );

    virtual             ~PageMaker_Index();

    virtual void        MakePage();

  private:
    // Interface csv::ConstProcessor<>
    virtual void        do_Process(
                            const ary::cpp::Namespace &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Typedef &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Function &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Variable &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::EnumValue &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Define &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Macro &
                                                i_rData );
    // Interface ary::cpp::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;
    // Locals
    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_CompleteAlphabeticalList();

    void                Write_CeIndexEntry(
                            const ary::cpp::CodeEntity &
                                                i_rCe,
                            const char *        i_sType,
                            const char *        i_sOwnerType );

    // DATA
    Dyn<NavigationBar>  pNavi;
    char                c;
    adcdisp::IndexList *
                        pCurIndex;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

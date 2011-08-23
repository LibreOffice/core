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

#ifndef ADC_DISPLAY_HTML_HD_PAGE_HXX
#define ADC_DISPLAY_HTML_HD_PAGE_HXX

// BASE CLASSES
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
// USED SERVICES
#include <ary/types.hxx>

class OuputPage_Environment;

namespace ary
{
namespace cpp
{
    class Class;
}
}

class PageDisplay;




class ClassDisplayer : public ary::Display,
                       public csv::ConstProcessor<ary::cpp::Class>
{
  public:
                        ClassDisplayer(            // TODO
                            OuputPage_Environment &
                                                io_rEnv );
    virtual				~ClassDisplayer();

 private:
    // Interface csv::ConstProcessor<>:
    virtual void	  	do_Process(
                            const ary::cpp::Class &
                                                i_data );
    // Interface ary::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;

    // Locals
    void                DisplayFiles_InClass(
                            const ary::cpp::Class &
                                                i_rData,
                            PageDisplay &       io_rPageMaker );

    OuputPage_Environment &
                        Env()                   { return *pEnv; }

    // DATA
    OuputPage_Environment *
                        pEnv;
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

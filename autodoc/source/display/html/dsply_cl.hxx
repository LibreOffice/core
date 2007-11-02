/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsply_cl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:25:03 $
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
    virtual             ~ClassDisplayer();

 private:
    // Interface csv::ConstProcessor<>:
    virtual void        do_Process(
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

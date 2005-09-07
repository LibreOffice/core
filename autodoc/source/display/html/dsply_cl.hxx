/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsply_cl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:26:03 $
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



// USED SERVICES
    // BASE CLASSES
#include <ary/ary_disp.hxx>
#include <ary/cpp/cpp_disp.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/idlists.hxx>



class OuputPage_Environment;

namespace ary
{
    namespace cpp
    {
        class Class;
    }
}


class PageDisplay;

class ClassDisplayer : public ary::cpp::Display
{
  public:
                        ClassDisplayer(            // TODO
                            OuputPage_Environment &
                                                io_rEnv );
    virtual             ~ClassDisplayer();

    virtual void        Display_Class(
                            const ary::cpp::Class &
                                                i_rData );
 private:
    // Interface ary::cpp::Display:
    virtual const ary::DisplayGate *
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


/*************************************************************************
 *
 *  $RCSfile: dsply_cl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


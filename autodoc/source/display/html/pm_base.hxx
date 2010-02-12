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

#ifndef ADC_DISPLAY_PM_BASE_HXX
#define ADC_DISPLAY_PM_BASE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include "hdimpl.hxx"



class OuputPage_Environment;
namespace csi
{
    namespace xml
    {
        class Element;
    }
}
class PageDisplay;


/** Interface for making a special kind of HTML-Page
*/
class SpecializedPageMaker
{
  public:
    virtual             ~SpecializedPageMaker() {}

    virtual void        MakePage() = 0;

    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
//    virtual void        Write_ChildList(
//                            ary::SlotAccessId   i_nSlot,
//                            const char *        i_nListTitle,
//                            const char *        i_nLabel );

  protected:
                        SpecializedPageMaker(
                            PageDisplay &       io_rPage );

    OuputPage_Environment &
                        Env() const             { return *pEnv; }
    csi::xml::Element & CurOut();
    PageDisplay &       Page()                  { return *pPage; }

  private:
    OuputPage_Environment *
                        pEnv;
    csi::xml::Element * pCurOut;
    PageDisplay *       pPage;
};



#endif


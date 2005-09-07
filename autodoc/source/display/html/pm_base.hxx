/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_base.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:34:52 $
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

#ifndef ADC_DISPLAY_PM_BASE_HXX
#define ADC_DISPLAY_PM_BASE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include "hdimpl.hxx"



class OuputPage_Environment;
class csi::xml::Element;
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
    virtual void        Write_ChildList(
                            ary::SlotAccessId   i_nSlot,
                            const char *        i_nListTitle,
                            const char *        i_nLabel );

  protected:
                        SpecializedPageMaker(
                            PageDisplay &       io_rPage );

    OuputPage_Environment &
                        Env()                   { return *pEnv; }
    csi::xml::Element & CurOut();
    PageDisplay &       Page()                  { return *pPage; }

  private:
    OuputPage_Environment *
                        pEnv;
    csi::xml::Element * pCurOut;
    PageDisplay *       pPage;
};



#endif


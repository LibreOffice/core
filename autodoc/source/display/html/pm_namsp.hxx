/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_namsp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:37:06 $
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

    virtual             ~PageMaker_Namespace();

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


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_aldef.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:34:19 $
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

#ifndef ADC_DISPLAY_HTML_PM_ALDEF_HXX
#define ADC_DISPLAY_HTML_PM_ALDEF_HXX



// USED SERVICES
    // BASE CLASSES
#include "pm_base.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/crog_def.hxx>


namespace ary
{
    namespace cpp
    {
        class CppDefinition;
    }
}

class Docu_Display;
class NavigationBar;

class PageMaker_AllDefs : public SpecializedPageMaker
{
  public:
                        PageMaker_AllDefs(
                            PageDisplay &       io_rPage );

    virtual             ~PageMaker_AllDefs();

    virtual void        MakePage();

  private:
    typedef ary::cpp::RoGate_Defines::List_DefIds   List_Ids;
    typedef List_Ids::const_iterator                ids_iterator;

    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
    virtual void        Write_DefinesList();
    virtual void        Write_MacrosList();
    void                Write_Define(
                            const ary::Rid &    i_nId );
    void                Write_Macro(
                            const ary::Rid &    i_nId );
    void                Write_DefsDocu(
                            csi::xml::Element & o_rOut,
                            const ary::cpp::CppDefinition &
                                                i_rTextReplacement );

    // DATA
    Dyn<Docu_Display>   pDocuDisplay;
    Dyn<NavigationBar>  pNavi;
};



#endif


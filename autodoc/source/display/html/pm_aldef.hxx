/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_aldef.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:31:26 $
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

    virtual             ~PageMaker_AllDefs();

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

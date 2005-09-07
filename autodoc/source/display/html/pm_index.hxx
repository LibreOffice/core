/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_index.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:36:34 $
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

#ifndef ADC_DISPLAY_HTML_PM_INDEX_HXX
#define ADC_DISPLAY_HTML_PM_INDEX_HXX



// USED SERVICES
    // BASE CLASSES
#include "pm_base.hxx"
#include <ary/cpp/cpp_disp.hxx>
    // COMPONENTS
    // PARAMETERS

namespace adcdisp
{
     class IndexList;
}


class ChildList_Display;
class NavigationBar;

class PageMaker_Index : public SpecializedPageMaker,
                        public ary::cpp::Display      // Must be 'public' because of a Solaris-Compiler-bug with dynamic_cast in hierarchies with private inheritance.
{
  public:
                        PageMaker_Index(
                            PageDisplay &       io_rPage,
                            char                i_c );

    virtual             ~PageMaker_Index();

    virtual void        MakePage();

    // Interface ary::cpp::Display:
    virtual void        Display_Namespace(
                            const ary::cpp::Namespace &
                                                i_rData );
    virtual void        Display_Class(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        Display_Enum(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        Display_Typedef(
                            const ary::cpp::Typedef &
                                                i_rData );
    virtual void        Display_Function(
                            const ary::cpp::Function &
                                                i_rData );
    virtual void        Display_Variable(
                            const ary::cpp::Variable &
                                                i_rData );
    virtual void        Display_EnumValue(
                            const ary::cpp::EnumValue &
                                                i_rData );

    virtual void        Display_Define(
                            const ary::cpp::Define &
                                                i_rData );
    virtual void        Display_Macro(
                            const ary::cpp::Macro &
                                                i_rData );

  private:
    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_CompleteAlphabeticalList();

    void                Write_CeIndexEntry(
                            const ary::CodeEntity &
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


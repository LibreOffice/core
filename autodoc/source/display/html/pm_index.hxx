/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_index.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:32:46 $
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

// BASE CLASSES
#include "pm_base.hxx"
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
// USED SERVICES
namespace adcdisp
{
     class IndexList;
}
namespace ary
{
    namespace cpp
    {
        class Namespace;
        class Class;
        class Enum;
        class Typedef;
        class Function;
        class Variable;
        class EnumValue;
        class Define;
        class Macro;
    }
}

class NavigationBar;




class PageMaker_Index : public SpecializedPageMaker,
                        public ary::Display,
                        public csv::ConstProcessor<ary::cpp::Namespace>,
                        public csv::ConstProcessor<ary::cpp::Class>,
                        public csv::ConstProcessor<ary::cpp::Enum>,
                        public csv::ConstProcessor<ary::cpp::Typedef>,
                        public csv::ConstProcessor<ary::cpp::Function>,
                        public csv::ConstProcessor<ary::cpp::Variable>,
                        public csv::ConstProcessor<ary::cpp::EnumValue>,
                        public csv::ConstProcessor<ary::cpp::Define>,
                        public csv::ConstProcessor<ary::cpp::Macro>
{
  public:
                        PageMaker_Index(
                            PageDisplay &       io_rPage,
                            char                i_c );

    virtual             ~PageMaker_Index();

    virtual void        MakePage();

  private:
    // Interface csv::ConstProcessor<>
    virtual void        do_Process(
                            const ary::cpp::Namespace &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Class &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Enum &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Typedef &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Function &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Variable &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::EnumValue &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Define &
                                                i_rData );
    virtual void        do_Process(
                            const ary::cpp::Macro &
                                                i_rData );
    // Interface ary::cpp::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;
    // Locals
    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_CompleteAlphabeticalList();

    void                Write_CeIndexEntry(
                            const ary::cpp::CodeEntity &
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

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsply_da.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:25:33 $
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

#ifndef ADC_DISPLAY_HTML_DSPLY_DA_HXX
#define ADC_DISPLAY_HTML_DSPLY_DA_HXX

// BASE CLASSES
#include <ary/ary_disp.hxx>
#include <cosv/tpl/processor.hxx>
// USED SERVICES
#include <ary/cpp/c_ce.hxx>

namespace ary
{
    namespace cpp
    {
        class Variable;
    }
}
namespace csi
{
    namespace xml
    {
        class Element;
    }
}




class OuputPage_Environment;
class PageDisplay;
class Docu_Display;

class DataDisplay : public ary::Display,
                    public csv::ConstProcessor<ary::cpp::Variable>
{
  public:
                        DataDisplay(
                            OuputPage_Environment &
                                                io_rInfo );
    virtual             ~DataDisplay();

    void                PrepareForConstants();
    void                PrepareForVariables();
    void                PrepareForStdMembers();
    void                PrepareForStaticMembers();

    void                Create_Files();

 private:
    // Interface csv::ConstProcessor<>:
    virtual void        do_Process(
                            const ary::cpp::Variable &
                                                i_rData );
    // Interface ary::cpp::Display:
    virtual const ary::cpp::Gate *
                        inq_Get_ReFinder() const;

    // Locals
    typedef ary::cpp::Lid                               SourceFileId;
    typedef std::map< SourceFileId, DYN PageDisplay* >  Map_FileId2PagePtr;

    PageDisplay &       FindPage_for(
                            const ary::cpp::Variable &
                                                i_rData );
    void                Display_SglDatum(
                            csi::xml::Element & rOut,
                            const ary::cpp::Variable &
                                                i_rData );
    // DATA
    Map_FileId2PagePtr  aMap_GlobalDataDisplay;
    Dyn<PageDisplay>    pClassMembersDisplay;

    OuputPage_Environment *
                        pEnv;
    Dyn<Docu_Display>   pDocuShow;
};




#endif

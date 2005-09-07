/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsply_da.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:26:35 $
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



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cpp_disp.hxx>
    // COMPONENTS
    // PARAMETERS


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

class DataDisplay : public ary::cpp::Display
{
  public:
                        DataDisplay(
                            OuputPage_Environment &
                                                io_rInfo );
    virtual             ~DataDisplay();

    virtual void        Display_Variable(
                            const ary::cpp::Variable &
                                                i_rData );
    void                PrepareForConstants();
    void                PrepareForVariables();
    void                PrepareForStdMembers();
    void                PrepareForStaticMembers();

    void                Create_Files();

 private:
    // Interface ary::cpp::Display:
    virtual const ary::DisplayGate *
                        inq_Get_ReFinder() const;

    // Locals
    typedef ary::Lid                                    SourceFileId;
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


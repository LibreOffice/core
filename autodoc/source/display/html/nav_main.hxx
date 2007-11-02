/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nav_main.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:28:39 $
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

#ifndef ADC_DISPLAY_HTML_NAV_MAIN_HXX
#define ADC_DISPLAY_HTML_NAV_MAIN_HXX

// USED SERVICES

namespace ary
{
namespace cpp
{
    class CodeEntity;
}
namespace loc
{
    class File;
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
class MainItem;




class MainRow
{
  public:
                        MainRow(
                            const OuputPage_Environment &
                                                i_rEnv );
                        ~MainRow();

    void                SetupItems_Overview();
    void                SetupItems_AllDefs();
    void                SetupItems_Index();
    void                SetupItems_Help();

    void                SetupItems_Ce(
                            const ary::cpp::CodeEntity &
                                                i_rCe );
    void                SetupItems_FunctionGroup(); /// For class member methods.
    void                SetupItems_DataGroup();     /// For class member data.

    void                Write2(
                            csi::xml::Element & o_rOut ) const;
  private:
    // Local
    enum E_Style
    {
        eSelf,
        eNo,
        eStd
    };

    /** @precond
        Only combinations of 1 eSelf and 2 eStd are allowed
        as arguments, here.
    */
    void                Create_ItemList_Global(
                            E_Style             i_eOverview,
                            E_Style             i_eIndex,
                            E_Style             i_eHelp );
    void                Create_ItemList_InDirTree_Cpp(
                            E_Style             i_eNsp,
                            E_Style             i_eClass,
                            E_Style             i_eTree,
                            const char *        i_sTreeLink );
    void                Add_Item(
                            E_Style             i_eStyle,
                            const String  &     i_sText,
                            const char *        i_sLink,
                            const char *        i_sTip );
    // DATA
    typedef std::vector< DYN MainItem* > ItemList;


    ItemList            aItems;
    const OuputPage_Environment *
                        pEnv;
};




#endif

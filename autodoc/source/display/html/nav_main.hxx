/*************************************************************************
 *
 *  $RCSfile: nav_main.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ADC_DISPLAY_HTML_NAV_MAIN_HXX
#define ADC_DISPLAY_HTML_NAV_MAIN_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cpp_disp.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
    namespace cpp
    {
        class DisplayGate;
    }

    class CodeEntity;
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


class MainRow : public ary::cpp::Display
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
                            const ary::CodeEntity &
                                                i_rCe );
    void                SetupItems_FunctionGroup(); /// For class member methods.
    void                SetupItems_FunctionGroup(   /// For global functions.
                            const ary::cpp::FileGroup &
                                                i_rFile );
    void                SetupItems_DataGroup();     /// For class member data.
    void                SetupItems_DataGroup(       /// For global data.
                            const ary::cpp::FileGroup &
                                                i_rFile );

    void                SetupItems_Project();
    void                SetupItems_File(
                            const ary::cpp::ProjectGroup &
                                                i_rProj );
    void                SetupItems_DefinitionsGroup(
                            const ary::cpp::ProjectGroup &
                                                i_rProj,
                            const ary::cpp::FileGroup &
                                                i_rFile );
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
                            const char *        i_sTreeLink,
                            E_Style             i_eProj,
                            const ary::cpp::ProjectGroup *
                                                i_pProj,
                            E_Style             i_eFile,
                            const ary::cpp::FileGroup *
                                                i_pFile );
    void                Create_ItemList_InDirTree_Prj(
                            E_Style             i_eProj,
                            const ary::cpp::ProjectGroup *
                                                i_pProj,
                            E_Style             i_eFile,
                            const ary::cpp::FileGroup *
                                                i_pFile );
    void                Add_Item(
                            E_Style             i_eStyle,
                            const udmstri &     i_sText,
                            const char *        i_sLink,
                            const char *        i_sTip );
    // DATA
    typedef std::vector< DYN MainItem* > ItemList;


    ItemList            aItems;
    const OuputPage_Environment *
                        pEnv;
};


#endif


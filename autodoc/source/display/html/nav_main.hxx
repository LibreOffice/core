/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

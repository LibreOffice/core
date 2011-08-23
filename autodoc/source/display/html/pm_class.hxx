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

#ifndef ADC_DISPLAY_HTML_PM_CLASS_HXX
#define ADC_DISPLAY_HTML_PM_CLASS_HXX



// USED SERVICES
    // BASE CLASSES
#include "pm_base.hxx"
    // COMPONENTS
    // PARAMETERS


namespace ary
{
     namespace cpp
    {
         class Class;
    }
}

class ChildList_Display;
class NavigationBar;


class PageMaker_Class : public SpecializedPageMaker
{
  public:
    enum E_ChidList
    {
        cl_NestedClasses = 0,
        cl_NestedStructs,
        cl_NestedUnions,
        cl_Enums,
        cl_Typedefs,
        cl_Operations,
        cl_StaticOperations,
        cl_Data,
        cl_StaticData,
        cl_MAX
    };
                        PageMaker_Class(
                            PageDisplay &       io_rPage,
                            const ary::cpp::Class &
                                                i_rClass );

    virtual				~PageMaker_Class();

    virtual void        MakePage();

  private:
    enum E_MemberProtection { mp_public = 0, mp_protected, mp_private, mp_MAX };

    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
    virtual void        Write_ChildList(
                            ary::SlotAccessId   i_nSlot,
                            E_ChidList          i_eChildListIndex,
                            const char *        i_sLabel,
                            const char *        i_sListTitle,
                            csi::xml::Element & o_rPublic,
                            csi::xml::Element & o_rProtected,
                            csi::xml::Element & o_rPrivate );
    void                Write_ChildList_forClasses(
                            csi::xml::Element & o_rPublic,
                            csi::xml::Element & o_rProtected,
                            csi::xml::Element & o_rPrivate,
                            const char *        i_sLabel,
                            const char *        i_sListTitle,
                            ary::cpp::E_ClassKey
                                                i_eFilter  );
    void                Write_ChildLists();
    static const char * ChildListLabel(
                            const char *        i_sLabel,
                            E_MemberProtection  i_eMpr );
    csi::html::DefListDefinition &
                        Setup_MemberSegment_Out(
                            E_MemberProtection  i_eMpr );
    void                Create_NaviSubRow(
                            E_MemberProtection  i_eMpr );
    void                Write_BaseHierarchy();
    void                Write_DerivedList();

    const ary::cpp::Class &
                        Me() const              { return *pMe; }
    // DATA
    const ary::cpp::Class *
                        pMe;
    Dyn<ChildList_Display>
                        pChildDisplay;
    Dyn<NavigationBar>  pNavi;

    Dyn<csi::xml::Element>
                        pProtectionArea[mp_MAX];

    bool                bChildLists_Exist[3*cl_MAX];
};





#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

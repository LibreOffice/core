/*************************************************************************
 *
 *  $RCSfile: pm_class.hxx,v $
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

    virtual             ~PageMaker_Class();

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


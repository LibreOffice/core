/*************************************************************************
 *
 *  $RCSfile: protarea.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:25 $
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

#ifndef ADC_DISPLAY_HTML_PROTAREA_HXX
#define ADC_DISPLAY_HTML_PROTAREA_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/c_etypes.hxx>

namespace csi
{
    namespace html
    {
        class Table;
    }
}


class ProtectionArea
{
  public:
                            ProtectionArea(
                                const char *        i_sLabel,
                                const char *        i_sTitle );
                            ProtectionArea(
                                const char *        i_sLabel,
                                const char *        i_sTitle_class,
                                const char *        i_sTitle_struct,
                                const char *        i_sTitle_union );
                            ~ProtectionArea();

    csi::html::Table &      GetTable();
    csi::html::Table &      GetTable(
                                ary::cpp::E_ClassKey
                                                    i_eClassKey );
    DYN csi::html::Table *  ReleaseTable();
    DYN csi::html::Table *  ReleaseTable(
                                ary::cpp::E_ClassKey
                                                    i_eClassKey );
    const char *            Label() const;

    int                     Size() const            { return pSglTable ? 1 : 3; }

    bool                    WasUsed_Area() const;
    bool                    WasUsed_Table() const;
    bool                    WasUsed_Table(
                                ary::cpp::E_ClassKey
                                                    i_eClassKey ) const;
  private:
    struct S_Slot_Table
    {
        const char *        sTableTitle;
        Dyn< csi::html::Table >
                            pTable;

                            S_Slot_Table(
                                const char *        i_sTitle );
                            ~S_Slot_Table();
        csi::html::Table &  GetTable();
        DYN csi::html::Table *
                            ReleaseTable()          { return pTable.Release(); }
        bool                WasUsed() const         { return pTable; }
    };

    UINT8                   Index(
                                ary::cpp::E_ClassKey
                                                    i_eClassKey ) const;
    // DATA
    Dyn<S_Slot_Table>       pSglTable;
    Dyn<S_Slot_Table>       aClassesTables[3];
    const char *            sLabel;
};



#endif


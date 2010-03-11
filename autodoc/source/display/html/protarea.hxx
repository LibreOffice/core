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

#ifndef ADC_DISPLAY_HTML_PROTAREA_HXX
#define ADC_DISPLAY_HTML_PROTAREA_HXX

// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>

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


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: protarea.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:33:39 $
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


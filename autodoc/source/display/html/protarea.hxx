/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


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



#ifndef ADC_DISPLAY_HTML_NAVIBAR_HXX
#define ADC_DISPLAY_HTML_NAVIBAR_HXX

// BASE CLASSES
#include "hdimpl.hxx"

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




/** Creates a HTML navigation bar wth the following parts:

    A main bar with fixed items.
    Zero to several subbars with user defined items, depending of
    the contents of the page.

    The main bar contains those items:

    Overview | Namespace | Class | Tree | Project | File | Index | Help
*/
class NavigationBar
{
  public:
    enum E_GlobalLocation
    {
        LOC_Overview,
        LOC_AllDefs,
        LOC_Index,
        LOC_Help
    };
    enum E_CeGatheringType
    {
        CEGT_operations,
        CEGT_data
    };

    /// Used for Overview, Index and Help.
                        NavigationBar(
                            const OuputPage_Environment &
                                                i_rEnv,
                            E_GlobalLocation    i_eLocation );
    /// Used for all Ces except operations and data.
                        NavigationBar(
                            const OuputPage_Environment &
                                                i_rEnv,
                            const ary::cpp::CodeEntity &
                                                i_rCe );
    /** Used for operations and data.
    */
                        NavigationBar(
                            const OuputPage_Environment &
                                                i_rEnv,
                            E_CeGatheringType   i_eCeGatheringType );
                        ~NavigationBar();

    void                MakeSubRow(
                            const char *        i_sTitle );
    void                AddItem(                /// Items are added to last made sub-row.
                            const char *        i_sName,
                            const char *        i_sLink,
                            bool                i_bValid );
    /** This writes the main bar and the pSubRowTable to o_rOut.
        The pSubRowsTable stays in memory and can be filled later,
        when all SubRow items are known.
    */
    void                Write(
                            csi::xml::Element & o_rOut,
                            bool                i_bWithSubRows = false ) const;
    void                Write_SubRows() const;

  private:
    struct CheshireCat;
    Dyn<CheshireCat>    pi;
    csi::html::Table *  pSubRowsTable;
};




#endif

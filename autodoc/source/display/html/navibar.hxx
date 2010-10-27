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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

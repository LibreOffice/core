/*************************************************************************
 *
 *  $RCSfile: hf_title.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:32:55 $
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

#ifndef ADC_DISPLAY_HF_TITLE_HXX
#define ADC_DISPLAY_HF_TITLE_HXX



// USED SERVICES
    // BASE CLASSES
#include <toolkit/htmlfactory.hxx>
    // COMPONENTS
    // PARAMETERS


class HF_TitleTable : public HtmlMaker
{
  public:
                        HF_TitleTable(
                            Xml::Element &      o_rOut );
    virtual             ~HF_TitleTable();

    void                Produce_Title(
                            const char *        i_title );
    /// @return a Html::TableCell reference.
    Xml::Element &      Add_Row();
};


class HF_SubTitleTable : public HtmlMaker
{
  public:
    enum E_SubLevel
    {
        sublevel_1,     /// Big title.
        sublevel_2,     /// Small title.
        sublevel_3      /// No title.
    };

    /** @param i_nColumns [1 .. n]
        @param i_nSubTitleLevel [1 .. 2]
        1 is a bit bigger than 2.
    */

                        HF_SubTitleTable(
                            Xml::Element &      o_rOut,
                            const String &      i_label,
                            const String &      i_title,
                            int                 i_nColumns,
                            E_SubLevel          i_eSubTitleLevel = sublevel_1 );
    virtual             ~HF_SubTitleTable();

    /// @return an Html::TableRow reference.
    Xml::Element &      Add_Row();
};


// IMPLEMENTATION



#endif

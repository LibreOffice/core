/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hf_title.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-03 16:57:45 $
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
    void                Produce_Title(
                            const char *        i_annotations,
//                            const char *        i_label,
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

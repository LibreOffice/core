/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hf_navi_main.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:55:59 $
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

#ifndef ADC_DISPLAY_HF_NAVI_MAIN_HXX
#define ADC_DISPLAY_HF_NAVI_MAIN_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "htmlfactory.hxx"
    // PARAMETERS


class HF_MainItem;


/** @task
    Create a HTML navigation bar with lightly coloured background.

    @descr
    There are three kinds of items:
    Item with link:             Add_StdItem(),
    Item without link:          Add_NoneItem(),
    Item that is current page:  Add_SelfItem().
*/
class HF_NaviMainRow : public HtmlMaker
{
  public:
    enum E_Style
    {
        eStd,
        eSelf,
        eNo
    };
                        HF_NaviMainRow(
                            Xml::Element &      o_out );
                        ~HF_NaviMainRow();

    void                Add_StdItem(
                            const char *        i_sText,
                            const char *        i_sLink );
    void                Add_SelfItem(
                            const char *        i_sText );
    void                Add_NoneItem(
                            const char *        i_sText );

    void                Produce_Row();

  private:
    // DATA
    typedef std::vector< DYN HF_MainItem* > ItemList;

    ItemList            aItems;
    Xml::Element *      pRow;
};



// IMPLEMENTATION




#endif



/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hf_navi_sub.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:56:20 $
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

#ifndef ADC_DISPLAY_HF_NAVI_SUB_HXX
#define ADC_DISPLAY_HFI_NAVI_SUB_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "htmlfactory.hxx"
    // PARAMETERS


class HF_NaviSubRow : public HtmlMaker
{
  public:
                        HF_NaviSubRow(
                            Xml::Element &      o_rOut );
    virtual             ~HF_NaviSubRow();

    void                AddItem(
                            const String &      i_sText,
                            const String &      i_sLink,
                            bool                i_bSwitchOn );
    void                SwitchOn(
                            int                 i_nIndex );
    void                Produce_Row();

  private:
    typedef std::pair<String,String>    SubRow_Data;
    typedef std::pair<SubRow_Data,bool> SubRow_Item;
    typedef std::vector<SubRow_Item>    SubRow;

    /** Puts the row's table into the parent XML-element, but
        doesn't write the items, because the actvity-status of
        the subitems isn't known yet.
    */
    void                Setup_Row();

    // DATA
    SubRow              aRow;
    Xml::Element *      pMyRow;
};




// IMPLEMENTATION




#endif



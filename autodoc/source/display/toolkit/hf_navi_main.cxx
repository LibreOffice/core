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

#include <precomp.h>
#include <toolkit/hf_navi_main.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>



//********************    MainItem and derived ones      ***************//
class HF_MainItem : public HtmlMaker
{
  public:
    virtual             ~HF_MainItem() {}
    void                Produce_Item() const    { do_ProduceItem(); }
  protected:
                        HF_MainItem(
                            Xml::Element &      o_out )
                                                :   HtmlMaker(o_out) {}
  private:
    virtual void        do_ProduceItem() const = 0;
};


namespace
{

class StdItem : public HF_MainItem
{
  public:
                        StdItem(
                            Xml::Element &      o_out,
                            const char *        i_sText,
                            const char *        i_sLink );

                        ~StdItem();
  private:
    virtual void        do_ProduceItem() const;

    // DATA
    String              sText;
    String              sLink;
};

class SelfItem : public HF_MainItem
{
  public:
                        SelfItem(
                            Xml::Element &      o_out,
                            const char *        i_sText );
                        ~SelfItem();
  private:
    virtual void        do_ProduceItem() const;

    // DATA
    String              sText;
};

class NoneItem : public HF_MainItem
{
  public:
                        NoneItem(
                            Xml::Element &      o_out,
                            const char *        i_sText );
                        ~NoneItem();
  private:
    virtual void        do_ProduceItem() const;

    // DATA
    String              sText;
};

}   // anonymous namespace



//********************    HF_NaviMainRow      ***************//



HF_NaviMainRow::HF_NaviMainRow( Xml::Element & o_out )
    :   HtmlMaker(o_out),
        aItems(),
        pRow(0)
{
    aItems.reserve(5);

    pRow = 
    &(  CurOut()
        >> *new Html::Table
           << new Html::ClassAttr("navimain") 
           << new Xml::AnAttribute( "border", "0" )
           << new Xml::AnAttribute( "cellpadding", "3" )
           >> *new Html::TableRow
     );
}

HF_NaviMainRow::~HF_NaviMainRow()
{
    csv::erase_container_of_heap_ptrs(aItems);
}

void
HF_NaviMainRow::Add_StdItem( const char * i_sText,
                             const char * i_sLink )
{
    aItems.push_back(new StdItem( *pRow,i_sText,i_sLink ));
}

void
HF_NaviMainRow::Add_SelfItem( const char * i_sText )
{
    aItems.push_back(new SelfItem( *pRow,i_sText ));
}

void
HF_NaviMainRow::Add_NoneItem( const char * i_sText )
{
    aItems.push_back(new NoneItem( *pRow,i_sText ));
}

void
HF_NaviMainRow::Produce_Row()
{
    ItemList::iterator itEnd = aItems.end();
    for ( ItemList::iterator iter = aItems.begin();
          iter != itEnd;
          ++iter )
    {
         (*iter)->Produce_Item();
    }
}




//********************    MainItem and derived ones      ***************//

namespace
{

StdItem::StdItem( Xml::Element &      o_out,
                  const char *        i_sText,
                  const char *        i_sLink )
    :   HF_MainItem(o_out),
        sText(i_sText),
        sLink(i_sLink)
{
}

StdItem::~StdItem()
{
}

void
StdItem::do_ProduceItem() const
{
    Xml::Element &
                rCell = CurOut() >>* new Html::TableCell;
    rCell
        << new Html::ClassAttr( "navimain" )
        >> *new Html::Link(sLink.c_str())
            << new Html::ClassAttr( "navimain" )
            << sText.c_str();
}

SelfItem::SelfItem( Xml::Element &      o_out,
                    const char *        i_sText )
    :   HF_MainItem(o_out),
        sText(i_sText)
{
}

SelfItem::~SelfItem()
{
}

void
SelfItem::do_ProduceItem() const
{
    Xml::Element &
                rCell = CurOut() >>* new Html::TableCell;
    rCell
        << new Html::ClassAttr( "navimainself" )
        << sText.c_str();
}

NoneItem::NoneItem( Xml::Element &      o_out,
                    const char *        i_sText )
    :   HF_MainItem(o_out),
        sText(i_sText)
{
}

NoneItem::~NoneItem()
{
}

void
NoneItem::do_ProduceItem() const
{
    Xml::Element &
                rCell = CurOut() >>* new Html::TableCell;
    rCell
        << new Html::ClassAttr( "navimainnone" )
        << sText.c_str();
}

}   // anonymous namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

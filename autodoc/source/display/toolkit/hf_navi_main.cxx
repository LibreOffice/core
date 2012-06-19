/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

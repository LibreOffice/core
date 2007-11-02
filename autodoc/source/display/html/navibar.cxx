/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navibar.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:28:54 $
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

#include <precomp.h>
#include "navibar.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include "nav_main.hxx"
#include "opageenv.hxx"


using namespace csi::xml;
using namespace csi::html;


namespace
{

//************************      SubRowItem      ***************************//

class SubRowItem
{
  public:
                        SubRowItem(
                            const char *        i_sText,
                            const char *        i_sLink,
                            bool                i_bActive,
                            bool                i_bFirstOfRow = false );
                        ~SubRowItem();

    void                Write2(
                            Element &           o_rOut ) const;
  private:
    String              sText;
    String              sLink;
    bool                bIsActive;
    bool                bFirstOfRow;
};

SubRowItem::SubRowItem( const char *        i_sText,
                        const char *        i_sLink,
                        bool                i_bActive,
                        bool                i_bFirstOfRow )
    :   sText(i_sText),
        sLink(i_sLink),
        bIsActive(i_bActive),
        bFirstOfRow(i_bFirstOfRow)
{
    csv_assert( NOT csv::no_str(i_sLink) );
}

SubRowItem::~SubRowItem()
{
}

void
SubRowItem::Write2( Element & o_rOut ) const
{
    o_rOut << new Sbr;
    if ( NOT bFirstOfRow )
        o_rOut << new XmlCode( "|&nbsp;" );
    else
        o_rOut << new XmlCode( "&nbsp;" );

    if ( bIsActive )
    {
        o_rOut
            >> *new Link( sLink.c_str() )
                >> *new AnElement( "font" )
                    << new AnAttribute("size","-2")
                    >> *new Bold
                        << sText.c_str();
    }
    else
    {
        o_rOut
            >> *new AnElement( "font" )
                << new AnAttribute("size","-2")
                << sText.c_str();
    }
}



//************************      SubRow      ***************************//

class SubRow
{
  public:
                        SubRow(
                            const char *        i_sTitle );
                        ~SubRow();

    void                AddItem(
                            const char *        i_sText,
                            const char *        i_sLink,
                            bool                i_bActive );
    void                Write2(
                            Table &             o_rOut ) const;
  private:
    typedef std::vector< DYN SubRowItem * >   List_Items;

    List_Items          aItemList;
    String              sTitle;
};

SubRow::SubRow( const char * i_sTitle )
//  :   // aItemList,
        // sTitle
{
    StreamStr sUp(i_sTitle,0);
    sUp.to_upper();
    sTitle = sUp.c_str();
}

SubRow::~SubRow()
{
}

inline void
SubRow::AddItem( const char *        i_sText,
                 const char *        i_sLink,
                 bool                i_bActive )
{
    aItemList.push_back( new SubRowItem(i_sText, i_sLink, i_bActive, aItemList.empty()) );
}

void
SubRow::Write2( Table & o_rOut ) const
{
    TableRow * pRow = new TableRow;
    o_rOut << pRow;

    if (sTitle.length() > 0)
    {
        Element & rCell1 = pRow->AddCell();
        rCell1
            << new WidthAttr("20%")
            >> *new AnElement( "font" )
                    << new AnAttribute("size","-2")
                       << sTitle
                       << ":";
    }

    Element & rCell2 = pRow->AddCell();
    for ( List_Items::const_iterator it = aItemList.begin();
          it != aItemList.end();
          ++it )
    {
         (*it)->Write2( rCell2 );
    }
}


}   // anonymous namespace



//*************************      CheshireCat     ***********************//


typedef std::vector< DYN SubRow * >   List_SubRows;

struct NavigationBar::CheshireCat
{
    MainRow             aMainRow;
    List_SubRows        aSubRows;
    const OuputPage_Environment *
                        pEnv;


                        CheshireCat(
                            const OuputPage_Environment &
                                                i_rEnv );
                        ~CheshireCat();
};

NavigationBar::
CheshireCat::CheshireCat( const OuputPage_Environment & i_rEnv )
    :   aMainRow( i_rEnv ),
        pEnv( & i_rEnv )
{
}

NavigationBar::
CheshireCat::~CheshireCat()
{
    csv::erase_container_of_heap_ptrs( aSubRows );
}


//************************       NavigationBar       *******************//

NavigationBar::NavigationBar( const OuputPage_Environment & i_rEnv,
                              E_GlobalLocation              i_eLocation )
    :   pi( new CheshireCat(i_rEnv) )
{
    switch (i_eLocation)
    {
         case LOC_Overview:  pi->aMainRow.SetupItems_Overview();  break;
         case LOC_AllDefs:   pi->aMainRow.SetupItems_AllDefs();   break;
        case LOC_Index:     pi->aMainRow.SetupItems_Index();     break;
        case LOC_Help:      pi->aMainRow.SetupItems_Help();      break;
        default:
                            csv_assert(false);
    }
}

NavigationBar::NavigationBar( const OuputPage_Environment & i_rEnv,
                              const ary::cpp::CodeEntity &  i_rCe  )
    :   pi( new CheshireCat(i_rEnv) )
{
    pi->aMainRow.SetupItems_Ce( i_rCe );
}

NavigationBar::NavigationBar( const OuputPage_Environment & i_rEnv,
                              E_CeGatheringType             i_eCeGatheringType )
    :   pi( new CheshireCat(i_rEnv) )
{
    switch (i_eCeGatheringType)
    {
         case CEGT_operations:   pi->aMainRow.SetupItems_FunctionGroup();  break;
        case CEGT_data:         pi->aMainRow.SetupItems_DataGroup();      break;
        default:
                                csv_assert(false);
    }
}

NavigationBar::~NavigationBar()
{
    csv::erase_container_of_heap_ptrs( pi->aSubRows );
}

void
NavigationBar::MakeSubRow( const char * i_sTitle )
{
    pi->aSubRows.push_back( new SubRow(i_sTitle) );
}

void
NavigationBar::AddItem( const char *        i_sName,
                        const char *        i_sLink,
                        bool                i_bValid )
{
    csv_assert( pi->aSubRows.size() > 0 );
    StreamStr sName(i_sName, 0);
    sName.to_upper();

    StreamLock aSum(100);
    pi->aSubRows.back()->AddItem( sName.c_str(),
                                  aSum() << "#" << i_sLink << c_str,
                                  i_bValid );
}

void
NavigationBar::Write( Element &  o_rOut,
                      bool       i_bWithSubRows ) const
{
    pi->aMainRow.Write2( o_rOut );

    const_cast< NavigationBar* >(this)->pSubRowsTable = new Table;
    o_rOut << pSubRowsTable;
    *pSubRowsTable
        << new AnAttribute( "class", "navisub" )
        << new AnAttribute( "cellpadding", "0" )
        << new AnAttribute( "cellspacing", "3" );

    if (i_bWithSubRows)
    {
        Write_SubRows();
    }
}

void
NavigationBar::Write_SubRows() const
{
    for ( List_SubRows::const_iterator it = pi->aSubRows.begin();
          it != pi->aSubRows.end();
          ++it )
    {
         (*it)->Write2( *pSubRowsTable );
    }
}

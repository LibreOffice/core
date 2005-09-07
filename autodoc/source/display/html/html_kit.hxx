/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: html_kit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:30:28 $
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

#ifndef ADC_DISPLAY_HTML_KIT_HXX
#define ADC_DISPLAY_HTML_KIT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <udm/xml/xmlitem.hxx>
#include <udm/html/htmlitem.hxx>
    // PARAMETERS
#include <ary/ids.hxx>

namespace ary
{
    namespace cpp
    {
        struct S_TplParam;
    }
}


namespace adcdisp
{

typedef csi::xml::Element XmlElement;

class PageTitle_Central
{
  public:
                        PageTitle_Central(
                            XmlElement &        o_rOwner );
    XmlElement &        Out()                   { return *pOut; }

  private:
    XmlElement *        pOut;
};

class PageTitle_Left
{
  public:
    void                operator()(
                            XmlElement &        o_rOwner,
                            const char *        i_sTypeTitle,
                            const udmstri &     i_sLocalName );
};

class PageTitle_Std
{
  public:
    void                operator()(
                            XmlElement &        o_rOwner,
                            const char *        i_sTypeTitle,
                            const udmstri &     i_sLocalName );
    XmlElement &        operator()(
                            XmlElement &        o_rOwner );
};

class OperationTitle
{
  public:
    void                operator()(
                            XmlElement &        o_rOwner,
                            const char *        i_sItemName,
                            ary::OSid           i_nSignature );
};


class TemplateClause
{
  public:
    typedef std::vector< ary::cpp::S_TplParam> List_TplParams;

    void                operator()(
                            XmlElement &        o_rOwner,
                            const List_TplParams &
                                                i_rTplParams );
};


class ExplanationList
{
  public:
                        ExplanationList(
                            XmlElement &        o_rOwner,
                            bool                i_bMemberStyle = false );

    void                AddEntry(
                            const char *        i_sTerm = 0,
                            const char *        i_sDifferentClass = 0 );
    void                AddEntry_NoTerm();

    XmlElement &        Term()                  { return *pTerm; }
    XmlElement &        Def()                   { return *pDefinition; }

  private:
    csi::html::DefList* pList;
    XmlElement *        pTerm;
    XmlElement *        pDefinition;
    bool                bMemberStyle;
};

class ExplanationTable
{
  public:
                        ExplanationTable(
                            XmlElement &        o_rOwner );

    void                AddEntry(
                            const char *        i_sTerm = 0,
                            const char *        i_sDifferentStyle = 0 );

    XmlElement &        Term()                  { return *pTerm; }
    XmlElement &        Def()                   { return *pDefinition; }

  private:
    csi::html::Table*   pTable;
    XmlElement *        pTerm;
    XmlElement *        pDefinition;
};

class ParameterTable
{
  public:
                        ParameterTable(
                            XmlElement &        o_rOwner );

    void                AddEntry();

    XmlElement &        Type()                  { return *pTerm; }
    XmlElement &        Name()                  { return *pDefinition; }

  private:
    csi::html::Table*   pTable;
    XmlElement *        pTerm;
    XmlElement *        pDefinition;
};

class FlagTable
{
  public:
                        FlagTable(
                            XmlElement &        o_rOwner,
                            uintt               i_nNrOfColumns );

    void                SetColumn(
                            uintt               i_nColumnPosition,  /// Starting with 0.
                            const char *        i_sColumnName,
                            bool                i_bValue );         /// "YES" or "NO"
    void                SetColumn(
                            uintt               i_nColumnPosition,  /// Starting with 0.
                            const char *        i_sColumnName,
                            const char *        i_sValue );
  private:
    typedef std::pair< csi::html::TableCell*, csi::html::TableCell* > CellPair;

    // DATA
    csi::html::Table*   pTable;
    std::vector<CellPair>
                        aCells;
};

class IndexList
{
  public:
                        IndexList(
                            XmlElement &        o_rOwner );

    void                AddEntry();

    XmlElement &        Term()                  { return *pTerm; }
    XmlElement &        Def()                   { return *pDefinition; }

  private:
    csi::html::DefList* pList;
    XmlElement *        pTerm;
    XmlElement *        pDefinition;
};




// IMPLEMENTATION



}   // namespace adcdisp


#endif



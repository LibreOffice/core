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



#ifndef ADC_DISPLAY_HTML_KIT_HXX
#define ADC_DISPLAY_HTML_KIT_HXX

// BASE CLASSES
#include <udm/xml/xmlitem.hxx>
#include <udm/html/htmlitem.hxx>
// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
    namespace cpp
    {
        struct S_TplParam;
        class OperationSignature;
        class Gate;
    }
}




namespace adcdisp
{

typedef csi::xml::Element XmlElement;

class PageTitle_Left
{
  public:
    void                operator()(
                            XmlElement &        o_rOwner,
                            const char *        i_sTypeTitle,
                            const String  &     i_sLocalName );
};

class PageTitle_Std
{
  public:
    void                operator()(
                            XmlElement &        o_rOwner,
                            const char *        i_sTypeTitle,
                            const String  &     i_sLocalName );
    XmlElement &        operator()(
                            XmlElement &        o_rOwner );
};

class OperationTitle
{
  public:
    void                operator()(
                            XmlElement &        o_rOwner,
                            const char *        i_sItemName,
                            ary::cpp::Ce_id     i_nId,
                            const ::ary::cpp::Gate &
                                                i_gate );
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




}   // namespace adcdisp
#endif

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_FRAMEWORK_INC_QUERIES_H
#define INCLUDED_FRAMEWORK_INC_QUERIES_H

#include "general.h"

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          These values describe our supported queries for type, filter ... properties.
                    They are used by our FilterFactory or our TypeDetection to return
                    subsets of our cached configuration.
*//*-*************************************************************************************************************/
#define BASE_QUERY_ALL                                  "_query_all"
#define BASE_QUERY_WRITER                               "_query_Writer"
#define BASE_QUERY_WEB                                  "_query_web"
#define BASE_QUERY_GLOBAL                               "_query_global"
#define BASE_QUERY_CHART                                "_query_chart"
#define BASE_QUERY_CALC                                 "_query_calc"
#define BASE_QUERY_IMPRESS                              "_query_impress"
#define BASE_QUERY_DRAW                                 "_query_draw"
#define BASE_QUERY_MATH                                 "_query_math"
#define BASE_QUERY_GRAPHICS                             "_query_graphics"

/*-************************************************************************************************************
    @short          These parameters can be used in combination with BASE_QUERY_... defines to
                    specialize it.
                    use follow syntax to do so:     "<query>[:<param>[=<value>]]"
                    e.g.:                           "_query_writer:default_first:use_order:sort_prop=uiname"

                    argument                        description                                     default
                    -----------------------------------------------------------------------------------------------
                    iflags=<mask>                   include filters by given mask                   0
                    eflags=<mask>                   exclude filters by given mask                   0
                    sort_prop=<[name,uiname]>       sort by internal name or uiname                 name
                    descending                      sort descending                                 false
                    use_order                       use order flag of filters for sorting           false
                    default_first                   set default filter on top of return list        false
                    case_sensitive                  compare "sort_prop" case sensitive              false
*//*-*************************************************************************************************************/
#define SEPARATOR_QUERYPARAM                            ((sal_Unicode)':')
#define SEPARATOR_QUERYPARAMVALUE                       ((sal_Unicode)'=')

#define QUERYPARAM_IFLAGS                               "iflags"
#define QUERYPARAM_EFLAGS                               "eflags"
#define QUERYPARAM_SORT_PROP                            "sort_prop"

#define QUERYPARAM_DESCENDING                           "descending"
#define QUERYPARAM_USE_ORDER                            "use_order"
#define QUERYPARAM_DEFAULT_FIRST                        "default_first"
#define QUERYPARAM_CASE_SENSITIVE                       "case_sensitive"

#define QUERYPARAMVALUE_SORT_PROP_NAME                  "name"
#define QUERYPARAMVALUE_SORT_PROP_UINAME                "uiname"

/*-************************************************************************************************************
    @short          Helper class to support easy building of a query statements.
*//*-*************************************************************************************************************/
class QueryBuilder
{
    public:

        // start with empty query

        QueryBuilder()
        {
            resetAll();
        }

        // forget all setted params and start with empty ones
        // Attention: base of query isn't changed!

        void resetParams()
        {
            m_sParams.makeStringAndClear();
            m_sParams.ensureCapacity( 256 );
        }

        // start with new empty query

        void resetAll()
        {
            m_sBase.clear();
            resetParams();
        }

    private:
        OUString         m_sBase;
        OUStringBuffer   m_sParams;

};      // class QueryBuilder

/*-************************************************************************************************************
    @short          Helper class to analyze queries and split into his different parts (base, params and values).
*//*-*************************************************************************************************************/
class QueryAnalyzer
{
    public:

        // it's will not perform to compare strings as query type ...
        // so we convert it into these enum values.
        // default = E_ALL!

        enum EQuery
        {
            E_ALL       ,
            E_WRITER    ,
            E_WEB       ,
            E_GLOBAL    ,
            E_CHART     ,
            E_CALC      ,
            E_IMPRESS   ,
            E_DRAW      ,
            E_MATH      ,
            E_GRAPHICS
        };

        // these are valid values for param "sort_prop".
        // other ones are not supported!
        // default = E_NAME

        enum ESortProp
        {
            E_NAME      ,
            E_UINAME
        };

        // analyze given query and split it into his different parts; <base>:<param1>:<param2=value>...

        QueryAnalyzer(const OUString& sQuery)
            // Don't forget to set default values for non given params!
            :   m_eQuery        ( E_ALL      )   // return ALL filter ...
            ,   m_nIFlags       ( 0          )   // which has set ANY flag ... (we remove all entries which match with these mask .. => 0!)
            ,   m_nEFlags       ( 0          )   // (only used, if nIFlags==0 and himself!=0!)
            ,   m_eSortProp     ( E_NAME     )   // sort it by internal name ...
            ,   m_bDescending   ( sal_False  )   // in ascending order ...
            ,   m_bCaseSensitive( sal_False  )   // ignore case ...
            ,   m_bUseOrder     ( sal_False  )   // don't use order flag ...
            ,   m_bDefaultFirst ( sal_False  )   // and don't handle default entries in special case!
        {
            // Translate old query format to new one first!
            OUString sNewQuery( sQuery );
            if (sQuery == "_filterquery_textdocument_withdefault")
                sNewQuery = "_query_writer:default_first:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_webdocument_withdefault")
                sNewQuery = "_query_web:default_first:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_globaldocument_withdefault")
                sNewQuery = "_query_global:default_first:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_chartdocument_withdefault")
                sNewQuery = "_query_chart:default_first:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_spreadsheetdocument_withdefault")
                sNewQuery = "_query_calc:default_first:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_presentationdocument_withdefault")
                sNewQuery = "_query_impress:default_first:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_drawingdocument_withdefault")
                sNewQuery = "_query_draw:default_first:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_formulaproperties_withdefault")
                sNewQuery = "_query_math:default_first:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_textdocument")
                sNewQuery = "_query_writer:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_webdocument")
                sNewQuery = "_query_web:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_globaldocument")
                sNewQuery = "_query_global:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_chartdocument")
                sNewQuery = "_query_chart:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_spreadsheetdocument")
                sNewQuery = "_query_calc:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_presentationdocument")
                sNewQuery = "_query_impress:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_drawingdocument")
                sNewQuery = "_query_draw:use_order:sort_prop=uiname";
            else if (sQuery == "_filterquery_formulaproperties")
                sNewQuery = "_query_math:use_order:sort_prop=uiname";

            // Analyze query ...
            // Try to find base of it and safe it for faster access as enum value!
            sal_Int32 nToken = 0;
            OUString sParam;
            OUString sBase = sNewQuery.getToken( 0, SEPARATOR_QUERYPARAM, nToken );

            if (sBase.equalsIgnoreAsciiCase(BASE_QUERY_ALL))
                m_eQuery = E_ALL;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_WRITER))
                m_eQuery = E_WRITER;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_WEB))
                m_eQuery = E_WEB;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_GLOBAL))
                m_eQuery = E_GLOBAL;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_CHART))
                m_eQuery = E_CHART;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_CALC))
                m_eQuery = E_CALC;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_IMPRESS))
                m_eQuery = E_IMPRESS;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_DRAW))
                m_eQuery = E_DRAW;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_MATH))
                m_eQuery = E_MATH;
            else if( sBase.equalsIgnoreAsciiCase(BASE_QUERY_GRAPHICS))
                m_eQuery = E_GRAPHICS;

            // Try to get additional parameters ...
            while( nToken >= 0 )
            {
                sParam = sNewQuery.getToken( 0, SEPARATOR_QUERYPARAM, nToken );
                // "default_first"
                if( sParam.startsWith( QUERYPARAM_DEFAULT_FIRST ) )
                {
                    m_bDefaultFirst = sal_True;
                }
                // "use_order"
                else if( sParam.startsWith( QUERYPARAM_USE_ORDER ) )
                {
                    m_bUseOrder = sal_True;
                }
                // "descending"
                else if( sParam.startsWith( QUERYPARAM_DESCENDING ) )
                {
                    m_bDescending = sal_True;
                }
                // "case_sensitive"
                else if( sParam.startsWith( QUERYPARAM_CASE_SENSITIVE ) )
                {
                    m_bCaseSensitive = sal_True;
                }
                // "iflags=<mask>"
                else if( sParam.startsWith( QUERYPARAM_IFLAGS ) )
                {
                    sal_Int32       nSubToken  = 0;
                    sParam.getToken( 0, SEPARATOR_QUERYPARAMVALUE, nSubToken );
                    if( nSubToken > 0 )
                    {
                        m_nIFlags = sParam.getToken( 0, SEPARATOR_QUERYPARAMVALUE, nSubToken ).toInt32();
                    }
                }
                // "eflags=<mask>"
                else if( sParam.startsWith( QUERYPARAM_EFLAGS ) )
                {
                    sal_Int32       nSubToken  = 0;
                    sParam.getToken( 0, SEPARATOR_QUERYPARAMVALUE, nSubToken );
                    if( nSubToken > 0 )
                    {
                        m_nEFlags = sParam.getToken( 0, SEPARATOR_QUERYPARAMVALUE, nSubToken ).toInt32();
                    }
                }
                // "sort_prop=<[name,uiname]>"
                else if( sParam.startsWith( QUERYPARAM_SORT_PROP ) )
                {
                    sal_Int32       nSubToken  = 0;
                    sParam.getToken( 0, SEPARATOR_QUERYPARAMVALUE, nSubToken );
                    if( nSubToken > 0 )
                    {
                        OUString sParamValue = sParam.getToken( 0, SEPARATOR_QUERYPARAMVALUE, nSubToken );
                        if( sParamValue.startsWith( QUERYPARAMVALUE_SORT_PROP_NAME ) )
                            m_eSortProp = E_NAME;
                        else if( sParamValue.startsWith( QUERYPARAMVALUE_SORT_PROP_UINAME ) )
                            m_eSortProp = E_UINAME;
                    }
                }
            }
        }

        // this method checks if given string match any supported query.
        // (ignore additional parameters!)

        static bool isQuery( const OUString& sQuery )
        {
            return(
                    sQuery.startsWith("_query_") ||    // new style
                    sQuery.startsWith("_filterquery_")       // old style!
                  );
        }

    private:
        EQuery      m_eQuery;
        sal_uInt32  m_nIFlags;
        sal_uInt32  m_nEFlags;
        ESortProp   m_eSortProp;
        bool        m_bDescending;
        bool        m_bCaseSensitive;
        bool        m_bUseOrder;
        bool        m_bDefaultFirst;

};      // class QueryAnalyzer

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_QUERIES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

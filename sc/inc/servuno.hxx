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



#ifndef SC_SERVUNO_HXX
#define SC_SERVUNO_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Sequence.hxx>

class String;
class ScDocShell;


//! AutoFormat wirklich hier oder besser global??????

#define SC_SERVICE_SHEET        0
#define SC_SERVICE_URLFIELD     1
#define SC_SERVICE_PAGEFIELD    2
#define SC_SERVICE_PAGESFIELD   3
#define SC_SERVICE_DATEFIELD    4
#define SC_SERVICE_TIMEFIELD    5
#define SC_SERVICE_TITLEFIELD   6
#define SC_SERVICE_FILEFIELD    7
#define SC_SERVICE_SHEETFIELD   8
#define SC_SERVICE_CELLSTYLE    9
#define SC_SERVICE_PAGESTYLE    10
#define SC_SERVICE_AUTOFORMAT   11
#define SC_SERVICE_CELLRANGES   12

//  drawing layer tables
#define SC_SERVICE_GRADTAB      13
#define SC_SERVICE_HATCHTAB     14
#define SC_SERVICE_BITMAPTAB    15
#define SC_SERVICE_TRGRADTAB    16
#define SC_SERVICE_MARKERTAB    17
#define SC_SERVICE_DASHTAB      18
#define SC_SERVICE_NUMRULES     19

#define SC_SERVICE_DOCDEFLTS    20
#define SC_SERVICE_DRAWDEFLTS   21

#define SC_SERVICE_DOCSPRSETT   22
#define SC_SERVICE_DOCCONF      23

#define SC_SERVICE_IMAP_RECT    24
#define SC_SERVICE_IMAP_CIRC    25
#define SC_SERVICE_IMAP_POLY    26

// #100263# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
#define SC_SERVICE_EXPORT_GOR   27
#define SC_SERVICE_IMPORT_GOR   28
#define SC_SERVICE_EXPORT_EOR   29
#define SC_SERVICE_IMPORT_EOR   30

#define SC_SERVICE_VALBIND      31
#define SC_SERVICE_LISTCELLBIND 32
#define SC_SERVICE_LISTSOURCE   33

#define SC_SERVICE_CELLADDRESS  34
#define SC_SERVICE_RANGEADDRESS 35

#define SC_SERVICE_SHEETDOCSET  36

// BM
#define SC_SERVICE_CHDATAPROV   37

// formula parser
#define SC_SERVICE_FORMULAPARS  38
#define SC_SERVICE_OPCODEMAPPER 39

// VBA specific
#define SC_SERVICE_VBAOBJECTPROVIDER   40
#define SC_SERVICE_VBACODENAMEPROVIDER   41
#define SC_SERVICE_VBAGLOBALS   42

#define SC_SERVICE_COUNT        43
#define SC_SERVICE_INVALID      USHRT_MAX


class ScServiceProvider
{
public:
                            // pDocShell wird nicht fuer alle Services benoetigt
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                            MakeInstance( sal_uInt16 nType, ScDocShell* pDocShell );
    static ::com::sun::star::uno::Sequence<rtl::OUString> GetAllServiceNames();
//UNUSED2008-05  static String           GetProviderName(sal_uInt16 nObjectType);
    static sal_uInt16       GetProviderType(const String& rServiceName);
};



#endif


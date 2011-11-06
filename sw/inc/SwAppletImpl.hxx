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



#ifndef _SW_APPLET_IMPL_HXX
#define _SW_APPLET_IMPL_HXX

#define SWHTML_OPTTYPE_IGNORE 0
#define SWHTML_OPTTYPE_TAG 1
#define SWHTML_OPTTYPE_PARAM 2
#define SWHTML_OPTTYPE_SIZE 3

#include <com/sun/star/embed/XEmbeddedObject.hpp>


#include <tools/string.hxx>
#include <sfx2/frmhtml.hxx>
#include <sfx2/frmhtmlw.hxx>
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#include <sot/storage.hxx>
#include <svl/itemset.hxx>

#include <svl/ownlist.hxx>

class SfxItemSet;

#define OOO_STRING_SW_HTML_O_Hidden "HIDDEN"

class SwApplet_Impl
{
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > xApplet;
    SvCommandList     aCommandList; // und die szugehorige Command-List
    SfxItemSet        aItemSet;
    String            sAlt;

public:
    static sal_uInt16 GetOptionType( const String& rName, sal_Bool bApplet );
    SwApplet_Impl( SfxItemPool& rPool, sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
    SwApplet_Impl( SfxItemSet& rSet ): aItemSet ( rSet) {}
    ~SwApplet_Impl();
    void CreateApplet( const String& rCode, const String& rName,
                       sal_Bool bMayScript, const String& rCodeBase,
                       const String& rBaseURL );
#ifdef SOLAR_JAVA
    sal_Bool CreateApplet( const String& rBaseURL );
    void AppendParam( const String& rName, const String& rValue );
#endif
    void FinishApplet();
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetApplet() { return xApplet; }
    SfxItemSet& GetItemSet() { return aItemSet; }
    const String& GetAltText() { return sAlt; }
    void          SetAltText( const String& rAlt ) {sAlt = rAlt;}
};
#endif

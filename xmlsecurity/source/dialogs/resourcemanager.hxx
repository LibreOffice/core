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



#ifndef _RESOURCEMANAGER_HXX
#define _RESOURCEMANAGER_HXX

#include <tools/resmgr.hxx>
#include <tools/datetime.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>

class FixedImage;
class FixedInfo;
class Control;
class LocaleDataWrapper;

namespace XmlSec
{
    ResMgr*         GetResMgr( void );

    const LocaleDataWrapper&    GetLocaleData( void );
    DateTime        GetDateTime( const ::com::sun::star::util::DateTime& _rDT );
    String          GetDateTimeString( const ::com::sun::star::util::DateTime& _rDT );
    String          GetDateTimeString( const rtl::OUString& _rDate, const rtl::OUString& _rTime );
    String          GetDateString( const ::com::sun::star::util::DateTime& _rDT );

    std::vector< std::pair< ::rtl::OUString, ::rtl::OUString> >
        parseDN(const ::rtl::OUString& rRawString);
    std::pair< ::rtl::OUString, ::rtl::OUString> GetDNForCertDetailsView(
        const ::rtl::OUString & rRawString);
    String          GetContentPart( const String& _rRawString );

    String          GetHexString( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rSeq, const char* _pSep = ":", sal_uInt16 _nLineBreak = 0xFFFF );

    long            ShrinkToFitWidth( Control& _rCtrl, long _nOffs = 0 );       // return = new width
    void            AlignAfterImage( const FixedImage& _rImage, Control& _rCtrl, long _nXOffset = 0 );
    void            AlignAfterImage( const FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset = 0 );
    void            AlignAndFitImageAndControl( FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset = 0 );
}

#define XMLSEC_RES(id)      ResId(id,*XmlSec::GetResMgr())

#endif

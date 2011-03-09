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

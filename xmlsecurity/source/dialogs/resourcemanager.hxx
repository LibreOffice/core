/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourcemanager.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:13:29 $
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

#ifndef _RESOURCEMANAGER_HXX
#define _RESOURCEMANAGER_HXX

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

class FixedImage;
class FixedInfo;
class Control;

namespace XmlSec
{
    ResMgr*         GetResMgr( void );

    International*  GetInternational( void );
    DateTime        GetDateTime( const ::com::sun::star::util::DateTime& _rDT );
    String          GetDateTimeString( const ::com::sun::star::util::DateTime& _rDT );
    String          GetDateTimeString( const rtl::OUString& _rDate, const rtl::OUString& _rTime );
    String          GetDateString( const ::com::sun::star::util::DateTime& _rDT );

    String          GetPureContent( const String& _rRawString,
                                    const char* _pCommaReplacement = ", ",
                                    bool _bPreserveId = false );        // strips "CN=" and so from string
    String          GetContentPart( const String& _rRawString, const String& _rPartId );
    String          GetContentPart( const String& _rRawString );

    String          GetHexString( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rSeq, const char* _pSep = ":", UINT16 _nLineBreak = 0xFFFF );

    long            ShrinkToFitWidth( Control& _rCtrl, long _nOffs = 0 );       // return = new width
    void            AlignAfterImage( const FixedImage& _rImage, Control& _rCtrl, long _nXOffset = 0 );
    void            AlignAfterImage( const FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset = 0 );
    void            AlignAndFitImageAndControl( FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset = 0 );
}

#define XMLSEC_RES(id)      ResId(id,XmlSec::GetResMgr())

#endif

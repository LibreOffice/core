/*************************************************************************
 *
 *  $RCSfile: resourcemanager.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: gt $ $Date: 2004-07-27 07:57:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

    String          GetHexString( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rSeq, const char* _pSep = ":", UINT16 _nLineBreak = 0xFFFF );

    long            ShrinkToFitWidth( Control& _rCtrl, long _nOffs = 0 );       // return = new width
    void            AlignAfterImage( const FixedImage& _rImage, Control& _rCtrl, long _nXOffset = 0 );
    void            AlignAfterImage( const FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset = 0 );
    void            AlignAndFitImageAndControl( FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset = 0 );
}

#define XMLSEC_RES(id)      ResId(id,XmlSec::GetResMgr())

#endif

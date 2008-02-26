/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmhtmlw.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:57:59 $
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
#ifndef _FRMHTMLW_HXX
#define _FRMHTMLW_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#include <sfx2/frmdescr.hxx>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _PARHTML_HXX
#include <svtools/parhtml.hxx>
#endif
#ifndef _HTMLOUT_HXX
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif

class SfxFrameSetObjectShell;
class SfxFrame;

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

class SFX2_DLLPUBLIC SfxFrameHTMLWriter
{
    SAL_DLLPRIVATE static const sal_Char sNewLine[];
    SAL_DLLPRIVATE static void OutMeta( SvStream& rStrm,
                                const sal_Char *pIndent, const String& rName,
                                const String& rContent, BOOL bHTTPEquiv,
                                rtl_TextEncoding eDestEnc,
                            String *pNonConvertableChars = 0 );
    SAL_DLLPRIVATE inline static void OutMeta( SvStream& rStrm,
                                const sal_Char *pIndent, const sal_Char *pName,
                                const String& rContent, BOOL bHTTPEquiv,
                                rtl_TextEncoding eDestEnc,
                            String *pNonConvertableChars = 0 );

public:
    static void Out_DocInfo( SvStream& rStrm, const String& rBaseURL,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>&,
            const sal_Char *pIndent,
            rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
            String *pNonConvertableChars = 0 );

    static void Out_FrameDescriptor(
        SvStream&, const String& rBaseURL, const com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet >& xSet,
        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
        String *pNonConvertableChars = 0 );

    String CreateURL( SfxFrame* pFrame );

};

inline void SfxFrameHTMLWriter::OutMeta( SvStream& rStrm,
                            const sal_Char *pIndent, const sal_Char *pName,
                            const String& rContent, BOOL bHTTPEquiv,
                            rtl_TextEncoding eDestEnc,
                            String *pNonConvertableChars )
{
    String sTmp = String::CreateFromAscii( pName );
    OutMeta( rStrm, pIndent, sTmp, rContent, bHTTPEquiv, eDestEnc, pNonConvertableChars );
}

#endif


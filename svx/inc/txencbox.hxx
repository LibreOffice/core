/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txencbox.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:20:56 $
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
#ifndef _SVX_TXENCBOX_HXX
#define _SVX_TXENCBOX_HXX

// include ---------------------------------------------------------------

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvxTextEncodingTable;

class SVX_DLLPUBLIC SvxTextEncodingBox : public ListBox
{
private:
    const SvxTextEncodingTable*     m_pEncTable;

    SVX_DLLPRIVATE USHORT               EncodingToPos_Impl( rtl_TextEncoding nEnc ) const;

public:
                        SvxTextEncodingBox( Window* pParent, const ResId& rResId );
                        ~SvxTextEncodingBox();

    /** Fill with all known encodings but exclude those matching one or more
        given flags as defined in rtl/tencinfo.h

         <p> If nButIncludeInfoFlags is given, encodings are included even if they
         match nExcludeInfoFlags. Thus it is possible to exclude 16/32-bit
         Unicode with RTL_TEXTENCODING_INFO_UNICODE but to include UTF7 and UTF8
         with RTL_TEXTENCODING_INFO_MIME </p>

        @param bExcludeImportSubsets
            If <TRUE/>, some specific encodings are not listed, as they are a
            subset of another encoding. This is the case for
            RTL_TEXTENCODING_GB_2312, RTL_TEXTENCODING_GBK,
            RTL_TEXTENCODING_MS_936, which are covered by
            RTL_TEXTENCODING_GB_18030. Normally, this flag should be set to
            <TRUE/> whenever the box is used in import dialogs. */
    void                FillFromTextEncodingTable(
                            sal_Bool bExcludeImportSubsets = FALSE,
                            sal_uInt32 nExcludeInfoFlags = 0,
                            sal_uInt32 nButIncludeInfoFlags = 0
                            );

    /** Fill with all encodings known to the dbtools::OCharsetMap but exclude
        those matching one or more given flags as defined in rtl/tencinfo.h

         <p> If nButIncludeInfoFlags is given, encodings are included even if they
         match nExcludeInfoFlags. Thus it is possible to exclude 16/32-bit
         Unicode with RTL_TEXTENCODING_INFO_UNICODE but to include UTF7 and UTF8
         with RTL_TEXTENCODING_INFO_MIME </p>

        @param bExcludeImportSubsets
            If <TRUE/>, some specific encodings are not listed, as they are a
            subset of another encoding. This is the case for
            RTL_TEXTENCODING_GB_2312, RTL_TEXTENCODING_GBK,
            RTL_TEXTENCODING_MS_936, which are covered by
            RTL_TEXTENCODING_GB_18030. Normally, this flag should be set to
            <TRUE/> whenever the box is used in import dialogs. */
    void                FillFromDbTextEncodingMap(
                            sal_Bool bExcludeImportSubsets = FALSE,
                            sal_uInt32 nExcludeInfoFlags = 0,
                            sal_uInt32 nButIncludeInfoFlags = 0
                            );

    /** Fill with all known MIME encodings and select the best according to
        <method>GetBestMimeEncoding</method>
     */
    void                FillWithMimeAndSelectBest();

    /** Get the best MIME encoding matching the system locale, or if that isn't
        determinable one that matches the UI locale, or UTF8 if everything else
        fails.
     */
    static  rtl_TextEncoding    GetBestMimeEncoding();

    const SvxTextEncodingTable*     GetTextEncodingTable() const
                            { return m_pEncTable; }

    void                InsertTextEncoding( const rtl_TextEncoding nEnc,
                            USHORT nPos = LISTBOX_APPEND );

    void                InsertTextEncoding( const rtl_TextEncoding nEnc,
                            const String& rEntry,
                            USHORT nPos = LISTBOX_APPEND );

    void                RemoveTextEncoding( const rtl_TextEncoding nEnc );

    void                SelectTextEncoding( const rtl_TextEncoding nEnc,
                            BOOL bSelect = TRUE );

    rtl_TextEncoding    GetSelectTextEncoding() const;

    const String&       GetSelectTextString() const;

    BOOL                IsTextEncodingSelected( const rtl_TextEncoding nEnc ) const;
};

#endif


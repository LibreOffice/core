/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentInfoPreview.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:45:50 $
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
#ifndef SVTOOLS_DOCUMENTINFOPREVIEW_HXX
#define SVTOOLS_DOCUMENTINFOPREVIEW_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTPROPERTIES_HPP_
#include <com/sun/star/document/XDocumentProperties.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

class SvtExtendedMultiLineEdit_Impl;
class SvtDocInfoTable_Impl;

namespace svtools
{
    class SVT_DLLPUBLIC ODocumentInfoPreview : public Window
    {
        SvtExtendedMultiLineEdit_Impl*  m_pEditWin;
        SvtDocInfoTable_Impl*           m_pInfoTable;
        com::sun::star::lang::Locale    m_aLocale;

    public:
        ODocumentInfoPreview( Window* pParent ,WinBits _nBits);
        virtual ~ODocumentInfoPreview();

        virtual void Resize();
        void    Clear();
        void    fill(const ::com::sun::star::uno::Reference<
                  ::com::sun::star::document::XDocumentProperties>& i_xDocProps
                    ,const String& i_rURL);
        void    InsertEntry( const String& rTitle, const String& rValue );
        void SetAutoScroll(BOOL _bAutoScroll);
    };
}

#endif // SVTOOLS_DOCUMENTINFOPREVIEW_HXX


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


#ifndef SVTOOLS_DOCUMENTINFOPREVIEW_HXX
#define SVTOOLS_DOCUMENTINFOPREVIEW_HXX

#include "svtools/svtdllapi.h"
#include <vcl/window.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/lang/Locale.hpp>

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
        void SetAutoScroll(sal_Bool _bAutoScroll);
    };
}

#endif // SVTOOLS_DOCUMENTINFOPREVIEW_HXX


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


#ifndef RPT_XMLIMAGE_HXX
#define RPT_XMLIMAGE_HXX

#include "xmlReportElementBase.hxx"
#ifndef _COM_SUN_STAR_REPORT_XImageControl_HPP_
#include <com/sun/star/report/XImageControl.hpp>
#endif

namespace rptxml
{
    class ORptFilter;
    class OXMLImage : public OXMLReportElementBase
    {
        OXMLImage(const OXMLImage&);
        void operator =(const OXMLImage&);
    public:

        OXMLImage( ORptFilter& rImport, sal_uInt16 nPrfx,
                    const ::rtl::OUString& rLName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XImageControl >& _xComponent
                    ,OXMLTable* _pContainer);
        virtual ~OXMLImage();
    };
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

#endif // RPT_XMLIMAGE_HXX

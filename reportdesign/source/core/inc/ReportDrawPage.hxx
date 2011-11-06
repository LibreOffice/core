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



#ifndef REPORTDRAWPAGE_HXX_INCLUDED
#define REPORTDRAWPAGE_HXX_INCLUDED

#include <svx/unopage.hxx>
#include <com/sun/star/report/XSection.hpp>

namespace reportdesign
{
    class OReportDrawPage : public SvxDrawPage
    {
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XSection > m_xSection;
        OReportDrawPage(const OReportDrawPage&);
        void operator =(const OReportDrawPage&);
    protected:
        virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape )throw ();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const throw ();
    public:
        OReportDrawPage(SdrPage* pPage,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
        virtual ~OReportDrawPage() throw(){}
    };
}
#endif //REPORTDRAWPAGE_HXX_INCLUDED


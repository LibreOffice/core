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



#ifndef INCLUDED_FIXEDTEXTCOLOR_HXX
#define INCLUDED_FIXEDTEXTCOLOR_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/report/XFixedText.hpp>
/** === end UNO includes === **/

#include <IReportControllerObserver.hxx>

//#include <boost/noncopyable.hpp>
#include <vector>

namespace rptui
{
    class OReportController;

    class FixedTextColor : public IReportControllerObserver
    {
        const OReportController& m_rReportController;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer > getVclWindowPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFixedText >& _xComponent) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > getXControl(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFixedText >& _xFixedText) throw(::com::sun::star::uno::RuntimeException);

        void setPropertyTextColor(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >& _xVclWindowPeer, sal_Int32 _nFormatKey);

        // sal_Int32 getTextColor();

    public:
        FixedTextColor(const OReportController & _aObserver);
        virtual ~FixedTextColor();

        void    notifyPropertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent );
        void    notifyElementInserted( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );
        void    handle( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );

    };

} // namespace rptui


#endif

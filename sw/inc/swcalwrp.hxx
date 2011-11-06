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



#ifndef _SWCALWRP_HXX
#define _SWCALWRP_HXX

#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <unotools/calendarwrapper.hxx>
#include <salhelper/singletonref.hxx>

class SwCalendarWrapper : public CalendarWrapper
{
    String sUniqueId;
    sal_uInt16 nLang;

public:
    SwCalendarWrapper( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory > & xMSF = ::comphelper::getProcessServiceFactory() )
        : CalendarWrapper( xMSF ), nLang( LANGUAGE_SYSTEM )
    {}

    void LoadDefaultCalendar( sal_uInt16 nLang );
};


salhelper::SingletonRef<SwCalendarWrapper>* s_getCalendarWrapper();


#endif


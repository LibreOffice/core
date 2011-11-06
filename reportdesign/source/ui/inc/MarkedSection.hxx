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



#ifndef RPTUI_MARKEDSECTION_HXX
#define RPTUI_MARKEDSECTION_HXX

#include <boost/shared_ptr.hpp>

namespace rptui
{
    class OSectionWindow; // forward declaration

    enum NearSectionAccess
    {
        CURRENT = 0,
        PREVIOUS = -1,
        POST = 1
    };

    class IMarkedSection
    {
    public:
        /** returns the section which is currently marked.
        */
        virtual ::boost::shared_ptr<OSectionWindow> getMarkedSection(NearSectionAccess nsa) const =0;

        /** mark the section on the given position .
        *
        * \param _nPos the position is zero based.
        */
        virtual void markSection(const sal_uInt16 _nPos) = 0;
    };

} // rptui

#endif /* RPTUI_MARKEDSECTION_HXX */

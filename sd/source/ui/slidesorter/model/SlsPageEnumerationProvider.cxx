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



#include "precompiled_sd.hxx"

#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlsPageDescriptor.hxx"
#include <boost/function.hpp>

namespace sd { namespace slidesorter { namespace model {


namespace {

class AllPagesPredicate
{
public:
    bool operator() (const SharedPageDescriptor& rpDescriptor)
    {
        (void)rpDescriptor;
        return true;
    }
};





class SelectedPagesPredicate
{
public:
    bool operator() (const SharedPageDescriptor& rpDescriptor)
    {
        return rpDescriptor->HasState(PageDescriptor::ST_Selected);
    }
};




class VisiblePagesPredicate
{
public:
    bool operator() (const SharedPageDescriptor& rpDescriptor)
    {
        return rpDescriptor->HasState(PageDescriptor::ST_Visible);
    }
};

}




PageEnumeration PageEnumerationProvider::CreateAllPagesEnumeration (
    const SlideSorterModel& rModel)
{
//    AllPagesPredicate aPredicate; // spurious warning on unxsoli4 debug=t
    return PageEnumeration::Create(rModel, AllPagesPredicate());
}




PageEnumeration PageEnumerationProvider::CreateSelectedPagesEnumeration (
    const SlideSorterModel& rModel)
{
    return PageEnumeration::Create(
        rModel,
        SelectedPagesPredicate());
}




PageEnumeration PageEnumerationProvider::CreateVisiblePagesEnumeration (
    const SlideSorterModel& rModel)
{
    return PageEnumeration::Create(
        rModel,
        VisiblePagesPredicate());
}


} } } // end of namespace ::sd::slidesorter::model

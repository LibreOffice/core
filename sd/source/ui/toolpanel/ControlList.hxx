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



#ifndef SD_TOOLPANEL_CONTROL_LIST_HXX
#define SD_TOOLPANEL_CONTROL_LIST_HXX

#include "ConstrainedIterator.hxx"
#include "ConstrainedIterator.cxx"
#include "TitledControl.hxx"

#include <vector>

namespace sd { namespace toolpanel {


typedef ::std::vector<TitledControl*> ControlList;
typedef ConstrainedIterator<ControlList> ControlIterator;


class VisibilityConstraint
    : public Constraint<ControlList>
{
public:
    virtual bool operator() (
        const ControlList& rContainer,
        const ControlList::iterator& rIterator) const
    {
        return (**rIterator).GetWindow()->IsVisible();
    }
};


} } // end of namespace ::sd::toolpanel

#endif

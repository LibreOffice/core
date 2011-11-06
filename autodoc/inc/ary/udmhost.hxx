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



#ifndef ARY_UDMHOST_HXX
#define ARY_UDMHOST_HXX
//  KORR_DEPRECATED_3.0



// USED SERVICES
    // BASE CLASSES
#include <ary/host.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{



class UdmHost : public Host
{
  public:
    enum E_ClassId { class_id = 1000 };

                        UdmHost();
    virtual             ~UdmHost();

  private:
    virtual Host_ClassId
                       inq_ClassId() const;
};




}   // namespace ary
#endif

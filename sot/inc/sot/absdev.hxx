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



#ifndef _SOT_ABSDEV_HXX
#define _SOT_ABSDEV_HXX

#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif

class JobSetup;
class AbstractDeviceData
{
protected:
    JobSetup * pJobSetup;
public:
    virtual ~AbstractDeviceData() {}
    virtual AbstractDeviceData *    Copy() const = 0;
    virtual sal_Bool                    Equals( const AbstractDeviceData & ) const = 0;

    JobSetup * GetJobSetup() const { return pJobSetup; }
};

#endif // _SOT_ABSDEV_HXX

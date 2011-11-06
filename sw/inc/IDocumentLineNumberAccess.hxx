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



 #ifndef IDOCUMENTLINENUMBERACCESS_HXX_INCLUDED
 #define IDOCUMENTLINENUMBERACCESS_HXX_INCLUDED

 #ifndef _SAL_TYPES_H_
 #include <sal/types.h>
 #endif

class SwLineNumberInfo;

 /** Access to the line number information
 */
 class IDocumentLineNumberAccess
 {
 public:

    virtual const SwLineNumberInfo& GetLineNumberInfo() const = 0;
    virtual void SetLineNumberInfo(const SwLineNumberInfo& rInfo) = 0;

 protected:
    virtual ~IDocumentLineNumberAccess() {};
};

#endif // IDOCUMENTLINENUMBERACCESS_HXX_INCLUDED


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



#ifndef _BOOLEXPRESSION_HXX
#define _BOOLEXPRESSION_HXX

// include for parent class
#include "computedexpression.hxx"

namespace xforms
{

/** BoolExpression represents a computed XPath expression that returns
 * a bool value and caches the results.
 *
 * As this class has no virtual methods, it should never be used
 * polymorphically. */
class BoolExpression : public ComputedExpression
{
public:
    BoolExpression();
    ~BoolExpression();

    /// set the expression string
    /// (overridden for new definition of a simple expression)
    void setExpression( const rtl::OUString& rExpression );
};

} // namespace xforms

#endif

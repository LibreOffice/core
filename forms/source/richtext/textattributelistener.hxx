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


#ifndef FORMS_SOURCE_RICHTEXT_TEXTATTRIBUTELISTENER_HXX
#define FORMS_SOURCE_RICHTEXT_TEXTATTRIBUTELISTENER_HXX

#include "rtattributes.hxx"

struct ESelection;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= ITextAttributeListener
    //====================================================================
    class ITextAttributeListener
    {
    public:
        virtual void    onAttributeStateChanged( AttributeId _nAttributeId, const AttributeState& _rState ) = 0;
    };

    //====================================================================
    //= ITextAttributeListener
    //====================================================================
    class ITextSelectionListener
    {
    public:
        virtual void    onSelectionChanged( const ESelection& _rSelection ) = 0;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_TEXTATTRIBUTELISTENER_HXX


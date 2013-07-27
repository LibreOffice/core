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



#ifndef _XMLOFF_TRANSFORMERACTIONINIT_HXX
#define _XMLOFF_TRANSFORMERACTIONINIT_HXX

#include <xmloff/xmltoken.hxx>


struct XMLTransformerActionInit
{
    sal_uInt16 m_nPrefix;
    ::xmloff::token::XMLTokenEnum m_eLocalName;
    sal_uInt32 m_nActionType;
    sal_uInt32 m_nParam1;
    sal_uInt32 m_nParam2;
    sal_uInt32 m_nParam3;

    static sal_uInt32 QNameParam( sal_uInt16 nPrefix,
                                    ::xmloff::token::XMLTokenEnum eLocalName )
    {
        return (static_cast< sal_uInt32 >( nPrefix ) << 16) +
               static_cast< sal_uInt32 >( eLocalName );
    }
};

#endif  //  _XMLOFF_TRANSFORMERACTIONINIT_HXX

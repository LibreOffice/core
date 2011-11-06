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



#ifndef ARY_DOC_D_TYPES4DOC_HXX
#define ARY_DOC_D_TYPES4DOC_HXX

// USED SERVICES



namespace ary
{
namespace doc
{


/** Type of a documentation: multiple lines or single line.
*/
enum E_BlockType
{
    dbt_none = 0,
    dbt_multiline,
    dbt_singleline
};

/** Type of documentation text: with html or without.
*/
enum E_TextType
{
    dtt_none = 0,
    dtt_plain,
    dtt_html
};

namespace nodetype
{

typedef int id;

}   //  namespace nodetype



}   //  namespace doc
}   //  namespace ary
#endif

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



#ifndef _XMLENUMS_HXX_
#define _XMLENUMS_HXX_

enum XMLForbiddenCharactersEnum
{
    XML_FORBIDDEN_CHARACTER_LANGUAGE,
    XML_FORBIDDEN_CHARACTER_COUNTRY,
    XML_FORBIDDEN_CHARACTER_VARIANT,
    XML_FORBIDDEN_CHARACTER_BEGIN_LINE,
    XML_FORBIDDEN_CHARACTER_END_LINE,
    XML_FORBIDDEN_CHARACTER_MAX
};

enum XMLSymbolDescriptorsEnum
{
    XML_SYMBOL_DESCRIPTOR_NAME,
    XML_SYMBOL_DESCRIPTOR_EXPORT_NAME,
    XML_SYMBOL_DESCRIPTOR_SYMBOL_SET,
    XML_SYMBOL_DESCRIPTOR_CHARACTER,
    XML_SYMBOL_DESCRIPTOR_FONT_NAME,
    XML_SYMBOL_DESCRIPTOR_CHAR_SET,
    XML_SYMBOL_DESCRIPTOR_FAMILY,
    XML_SYMBOL_DESCRIPTOR_PITCH,
    XML_SYMBOL_DESCRIPTOR_WEIGHT,
    XML_SYMBOL_DESCRIPTOR_ITALIC,
    XML_SYMBOL_DESCRIPTOR_MAX
};
#endif

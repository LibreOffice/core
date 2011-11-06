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



#ifndef _HTMLFORM_HXX
#define _HTMLFORM_HXX


enum HTMLEventType
{
    HTML_ET_ONSUBMITFORM,   HTML_ET_ONRESETFORM,
    HTML_ET_ONGETFOCUS,     HTML_ET_ONLOSEFOCUS,
    HTML_ET_ONCLICK,        HTML_ET_ONCLICK_ITEM,
    HTML_ET_ONCHANGE,       HTML_ET_ONSELECT,
    HTML_ET_END
};

extern HTMLEventType __FAR_DATA aEventTypeTable[];
extern const sal_Char * __FAR_DATA aEventListenerTable[];
extern const sal_Char * __FAR_DATA aEventMethodTable[];
extern const sal_Char * __FAR_DATA aEventSDOptionTable[];
extern const sal_Char * __FAR_DATA aEventOptionTable[];




#endif



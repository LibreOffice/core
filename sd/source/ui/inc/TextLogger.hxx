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



#ifndef SD_TEXT_LOGGER_HXX
#define SD_TEXT_LOGGER_HXX

#include <tools/link.hxx>

class String;
class VclWindowEvent;

namespace sd { namespace notes {

class EditWindow;

class TextLogger
{
public:
    static TextLogger& Instance (void);

    void AppendText (const char* sText);
    void AppendText (const String& sText);
    void AppendNumber (long int nValue);

    void ConnectToEditWindow (EditWindow* pWindow);

private:
    static TextLogger* spInstance;
    EditWindow* mpEditWindow;

    TextLogger (void);

    DECL_LINK(WindowEventHandler, VclWindowEvent*);
};

} } // end of namespace ::sd::notes

#endif

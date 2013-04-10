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



#ifndef SD_TEXT_OBJECT_BAR_HXX
#define SD_TEXT_OBJECT_BAR_HXX


#include <sfx2/module.hxx>
#include <sfx2/shell.hxx>
#include "glob.hxx"

class CommandEvent;

namespace sd {

class View;
class ViewShell;
class Window;

class TextObjectBar
    : public SfxShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDDRAWTEXTOBJECTBAR)

    TextObjectBar (
        ViewShell* pSdViewShell,
        SfxItemPool& rItemPool,
        ::sd::View* pSdView);
    virtual ~TextObjectBar (void);

    void GetAttrState( SfxItemSet& rSet );
    void GetCharState( SfxItemSet& rSet );
    void Execute( SfxRequest &rReq );

    virtual void Command( const CommandEvent& rCEvt );

private:
    ViewShell* mpViewShell;
    ::sd::View* mpView;
};

} // end of namespace sd

#endif

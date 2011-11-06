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


#ifndef _POPBOX_HXX
#define _POPBOX_HXX


#ifndef _TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#include <svtools/transfer.hxx>

class SwNavigationPI;

class SwHelpToolBox: public ToolBox, public DropTargetHelper
{
    Link aDoubleClickLink;
    Link aRightClickLink;       // Link bekommt MouseEvent als Parameter !!!

    using ToolBox::DoubleClick;

protected:
    virtual void MouseButtonDown(const MouseEvent &rEvt);
    virtual long DoubleClick(ToolBox *);
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

public:
    SwHelpToolBox(SwNavigationPI* pParent, const ResId &);
    ~SwHelpToolBox();
    void SetDoubleClickLink(const Link &);      // inline
    void SetRightClickLink(const Link &);       // inline
};

inline void SwHelpToolBox::SetDoubleClickLink(const Link &rLink) {
    aDoubleClickLink = rLink;
}

inline void SwHelpToolBox::SetRightClickLink(const Link &rLink) {
    aRightClickLink = rLink;
}


#endif

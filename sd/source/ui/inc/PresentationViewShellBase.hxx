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



#ifndef SD_PRESENTATION_VIEW_SHELL_BASE_HXX
#define SD_PRESENTATION_VIEW_SHELL_BASE_HXX

#include "ViewShellBase.hxx"


namespace sd {

/** This class exists to be able to register another factory that
    creates the view shell for the presentation.
*/
class PresentationViewShellBase
    : public ViewShellBase
{
public:
    SFX_DECL_VIEWFACTORY(PresentationViewShellBase);

    /** This constructor is used by the view factory of the SFX
        macros.
    */
    PresentationViewShellBase (SfxViewFrame *pFrame, SfxViewShell* pOldShell);
    virtual ~PresentationViewShellBase (void);

protected:
    virtual void InitializeFramework (void);
};

} // end of namespace sd

#endif

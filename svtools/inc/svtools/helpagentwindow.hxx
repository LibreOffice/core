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



#ifndef _SVTOOLS_HELPAGENTWIDNOW_HXX_
#define _SVTOOLS_HELPAGENTWIDNOW_HXX_

#include "svtools/svtdllapi.h"
#include <vcl/floatwin.hxx>
#include <vcl/image.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= IHelpAgentCallback
    //====================================================================
    class IHelpAgentCallback
    {
    public:
        virtual void helpRequested() = 0;
        virtual void closeAgent() = 0;
    };

    //====================================================================
    //= HelpAgentWindow
    //====================================================================
    class SVT_DLLPUBLIC HelpAgentWindow : public FloatingWindow
    {
    protected:
        Window*                 m_pCloser;
        IHelpAgentCallback*     m_pCallback;
        Size                    m_aPreferredSize;
        Image                   m_aPicture;

    public:
        HelpAgentWindow( Window* _pParent );
        ~HelpAgentWindow();

        /// returns the preferred size of the window
        const Size& getPreferredSizePixel() const { return m_aPreferredSize; }

        // callback handler maintenance
        void                setCallback(IHelpAgentCallback* _pCB) { m_pCallback = _pCB; }
        IHelpAgentCallback* getCallback() const { return m_pCallback; }

    protected:
        virtual void Resize();
        virtual void Paint( const Rectangle& rRect );
        virtual void MouseButtonUp( const MouseEvent& rMEvt );

        DECL_LINK( OnButtonClicked, Window* );

    private:
        SVT_DLLPRIVATE Size implOptimalButtonSize( const Image& _rButtonImage );
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif  // _SVTOOLS_HELPAGENTWIDNOW_HXX_


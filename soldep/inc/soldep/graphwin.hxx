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



#ifndef _FMRWRK_GRAPHWIN_HXX
#define _FMRWRK_GRAPHWIN_HXX

#include <svtools/scrwin.hxx>

class GraphWin : public ScrollableWindow
{
private:
    void* mpClass;

protected:
    Window aBufferWindow;

public:
    GraphWin( Window * pParent, void * pClass );
            void CalcSize();
    virtual void EndScroll( long nDeltaX, long nDeltaY );
    virtual void Resize();
    virtual void Command( const CommandEvent& rEvent);
    Window* GetBufferWindow(){ return &aBufferWindow; };
};

#define PIXELS( nLeft, nTop, nWidth, nHeight )\
    LogicToPixel( Point( nLeft, nTop ) ), LogicToPixel( Size( nWidth, nHeight ) )

#endif //  _FMRWRK_GRAPHWIN_HXX


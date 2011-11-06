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



#ifndef _PAD_TITLECTRL_HXX_
#define _PAD_TITLECTRL_HXX_

#include <vcl/ctrl.hxx>
#include <vcl/image.hxx>

namespace padmin
{

class TitleImage : public Control
{
    Image               m_aImage;
    String              m_aText;
    Color               m_aBGColor;
    Point               m_aImagePos;
    Point               m_aTextPos;

    bool                m_bArranged;

    void arrange();
public:
    TitleImage( Window* pParent, const ResId& rResId );
    ~TitleImage();

    virtual void Paint( const Rectangle& rRect );

    void SetImage( const Image& rImage );
    const Image& GetImage() const { return m_aImage; }

    virtual void SetText( const String& rText );
    virtual String GetText() const { return m_aText; }

    void SetBackgroundColor( const Color& rColor );
    const Color& GetBackgroundColor() const { return m_aBGColor; }
};

}

#endif // _PAD_TITLECTRL_HXX_

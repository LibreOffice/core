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



#ifndef _SD_HTMLATTR_HXX
#define _SD_HTMLATTR_HXX

#include <vcl/ctrl.hxx>
#include <tools/color.hxx>

class SdHtmlAttrPreview : public Control
{
protected:

    Color   m_aBackColor, m_aTextColor, m_aLinkColor;
    Color   m_aVLinkColor, m_aALinkColor;

public:
    SdHtmlAttrPreview( Window* pParent, const ResId& rResId );
    ~SdHtmlAttrPreview();

    virtual void    Paint( const Rectangle& rRect );

    void    SetColors( Color& aBack, Color& aText, Color& aLink,
                       Color& aVLink, Color& aALink );
};

#endif // _SD_HTMLATTR_HXX

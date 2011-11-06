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


#ifndef _SFXPREVIEW_HXX
#define _SFXPREVIEW_HXX


#include <sfx2/doctempl.hxx>
#include <sfx2/basedlgs.hxx>

class SfxObjectShell;
//REMOVE    class SvStorageRef;
class SfxFrameWindow;

class SfxFrame;
class GDIMetaFile;

class SfxPreviewBase_Impl : public Window
{
protected:
    ::boost::shared_ptr<GDIMetaFile> pMetaFile;
public:
                    SfxPreviewBase_Impl( Window* pParent,
                                         const ResId& rResId );
                    SfxPreviewBase_Impl( Window* pParent );
                    ~SfxPreviewBase_Impl(  );
    void            SetObjectShell( SfxObjectShell* pObj );
    void            SetGDIFile( ::boost::shared_ptr<GDIMetaFile> pFile );
    virtual void    Resize();
};



class SfxPreviewWin_Impl: public SfxPreviewBase_Impl
{
protected:
    virtual void    Paint( const Rectangle& rRect );
public:
                    SfxPreviewWin_Impl( Window* pParent,
                                    const ResId& rResId )
                    : SfxPreviewBase_Impl( pParent, rResId ){};
                    SfxPreviewWin_Impl( Window* pParent )
                    : SfxPreviewBase_Impl( pParent ){};
    static void     ImpPaint(
        const Rectangle& rRect, GDIMetaFile* pFile, Window* pWindow );
    static SfxFrameWindow*
                    PreviewFactory( SfxFrame* pFrame, const String& rName );
};

#endif

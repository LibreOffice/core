/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: preview.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:12:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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

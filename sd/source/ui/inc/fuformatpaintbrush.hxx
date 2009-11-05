/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fuformatpaintbrush.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_FU_FORMATPAINTBRUSH_HXX
#define SD_FU_FORMATPAINTBRUSH_HXX

#include "futext.hxx"

// header for class SfxItemSet
#include <svtools/itemset.hxx>
#include <boost/scoped_ptr.hpp>

class SfxItemSet;

namespace sd {

class DrawViewShell;

class FuFormatPaintBrush : public FuText
{
public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);
    virtual BOOL KeyInput(const KeyEvent& rKEvt);

    virtual void Activate();
    virtual void Deactivate();

    static void GetMenuState( DrawViewShell& rDrawViewShell, SfxItemSet &rSet );
    static bool CanCopyThisType( UINT32 nObjectInventor, UINT16 nObjectIdentifier );

private:
    FuFormatPaintBrush ( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq);

    void DoExecute( SfxRequest& rReq );

    bool HasContentForThisType( UINT32 nObjectInventor, UINT16 nObjectIdentifier ) const;
    void Paste( bool, bool );

    void implcancel();

    ::boost::shared_ptr<SfxItemSet> mpItemSet;
    bool   mbPermanent;
    bool   mbOldIsQuickTextEditMode;
};

} // end of namespace sd

#endif

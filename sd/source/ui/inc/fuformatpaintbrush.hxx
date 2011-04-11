/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <svl/itemset.hxx>
#include <boost/scoped_ptr.hpp>

namespace sd {

class DrawViewShell;

class FuFormatPaintBrush : public FuText
{
public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);

    virtual void Activate();
    virtual void Deactivate();

    static void GetMenuState( DrawViewShell& rDrawViewShell, SfxItemSet &rSet );
    static bool CanCopyThisType( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier );

private:
    FuFormatPaintBrush ( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq);

    void DoExecute( SfxRequest& rReq );

    bool HasContentForThisType( sal_uInt32 nObjectInventor, sal_uInt16 nObjectIdentifier ) const;
    void Paste( bool, bool );

    void implcancel();

    ::boost::shared_ptr<SfxItemSet> mpItemSet;
    bool   mbPermanent;
    bool   mbOldIsQuickTextEditMode;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SIDEBAR_PANELLAYOUT_HXX
#define SIDEBAR_PANELLAYOUT_HXX

#include <svx/svxdllapi.h>

#include <vcl/builder.hxx>
#include <vcl/ctrl.hxx>

#include <com/sun/star/frame/XFrame.hpp>

/// This class is the base for the Widget Layout-based sidebar panels.
class SVX_DLLPUBLIC PanelLayout : public Control, public VclBuilderContainer
{
public:
    PanelLayout(Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
            const com::sun::star::uno::Reference<com::sun::star::frame::XFrame> &rFrame);
    virtual ~PanelLayout() {}

    virtual Size GetOptimalSize() const;
    virtual void setPosSizePixel(long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags = WINDOW_POSSIZE_ALL);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

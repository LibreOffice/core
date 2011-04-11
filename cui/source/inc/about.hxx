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
#ifndef _ABOUT_HXX
#define _ABOUT_HXX

// include ---------------------------------------------------------------

#include <tools/resary.hxx>
#include <vcl/button.hxx>
#include <vcl/accel.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/stdctrl.hxx>
#include "svtools/fixedhyper.hxx"
#include <sfx2/basedlgs.hxx>        // SfxModalDialog
#include <vector>

typedef ::std::vector< Accelerator* > AccelList;

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
    OKButton        aOKButton;
    Image           aAppLogo;

    MultiLineEdit       aVersionText;
    MultiLineEdit       aCopyrightText;
    svt::FixedHyperlink aInfoLink;

//    ResStringArray    aDeveloperAry; // RIP ...
    String          aAccelStr;
    String          aVersionData;
    String          aVersionTextStr;
    String          aCopyrightTextStr;
    String          aLinkStr;

    AccelList       aAccelList;

    AutoTimer       aTimer;
    long            nOff;
    long            m_nDeltaWidth;
    int             m_nPendingScrolls;

protected:
    virtual sal_Bool Close();
    virtual void Paint( const Rectangle& rRect );

public:
    AboutDialog( Window* pParent, const ResId& rId);
    ~AboutDialog();

    DECL_LINK( TimerHdl, Timer * );
    DECL_LINK( AccelSelectHdl, Accelerator * );
    DECL_LINK( HandleHyperlink, svt::FixedHyperlink * );
};

#endif // #ifndef _ABOUT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

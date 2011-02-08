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
#include <tools/list.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/basedlgs.hxx>        // SfxModalDialog

DECLARE_LIST( AccelList, Accelerator* )

// class AboutDialog -----------------------------------------------------

class AboutDialog : public SfxModalDialog
{
private:
    OKButton        aOKButton;
    Image           aAppLogo;

    FixedInfo       aVersionText;
    MultiLineEdit   aCopyrightText;
    FixedInfo       aBuildData;

    ResStringArray* pDeveloperAry;
    String          aDevVersionStr;
    String          aAccelStr;
    String          aVersionData;
    String          aCopyrightTextStr;

    AccelList       aAccelList;

    AutoTimer       aTimer;
    long            nOff;
    long            m_nDeltaWidth;
    int             m_nPendingScrolls;

    sal_Bool            bNormal;

protected:
    virtual sal_Bool    Close();
    virtual void    Paint( const Rectangle& );

public:
    AboutDialog( Window* pParent, const ResId& nId );
    ~AboutDialog();

    DECL_LINK( TimerHdl, Timer * );
    DECL_LINK( AccelSelectHdl, Accelerator * );
};

#endif // #ifndef _ABOUT_HXX



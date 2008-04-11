/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: about.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _RESARY_HXX //autogen
#include <tools/resary.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _ACCEL_HXX //autogen
#include <vcl/accel.hxx>
#endif
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
    FixedInfo       aCopyrightText;
    FixedInfo       aBuildData;

    ResStringArray  aDeveloperAry;
    String          aDevVersionStr;
    String          aAccelStr;
    String          aVersionData;

    AccelList       aAccelList;

    AutoTimer       aTimer;
    long            nOff;
    long            nEnd;
    long            m_nDeltaWidth;

    BOOL            bNormal;

protected:
    virtual BOOL    Close();
    virtual void    Paint( const Rectangle& );

public:
    AboutDialog( Window* pParent, const ResId& rId, const String& rVerStr );
    ~AboutDialog();

    DECL_LINK( TimerHdl, Timer * );
    DECL_LINK( AccelSelectHdl, Accelerator * );
};

#endif // #ifndef _ABOUT_HXX


